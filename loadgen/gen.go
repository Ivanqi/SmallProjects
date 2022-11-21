package loadgen

import (
	"bytes"
	"context"
	"errors"
	"fmt"
	"math"
	"sync/atomic"
	"time"

	"loadgen/lib"

	log "gLog"
)

// 日志记录器
var logger = log.DLogger()

// myGenerator 代表载荷发生器的实现类型
type myGenerator struct {
	caller      lib.Caller           // 调用器
	timeoutNS   time.Duration        // 处理超时时间，单位：纳秒
	lps         uint32               // 每秒载荷量
	durationNS  time.Duration        // 负载持续时间，单位：纳秒
	concurrency uint32               // 载荷并发量
	tickets     lib.GoTickets        // Goroutine票池
	ctx         context.Context      // 上下文
	cancelFunc  context.CancelFunc   // 取消函数
	callCount   int64                // 调用计数
	status      uint32               // 状态
	resultCh    chan *lib.CallResult // 调用结果通道
}

// NewGenerator 会新建一个载荷发生器
func NewGenerator(pset ParamSet) (lib.Generator, error) {
	logger.Infoln("New a load generator...")

	if err := pset.Check(); err != nil {
		return nil, err
	}

	gen := &myGenerator{
		caller:     pset.Caller,
		timeoutNS:  pset.TimeoutNS,
		lps:        pset.LPS,
		durationNS: pset.DurationNS,
		status:     lib.STATUS_ORIGINAL,
		resultCh:   pset.ResultCh,
	}

	if err := gen.init(); err != nil {
		return nil, err
	}

	return gen, nil
}

// 初始化载荷发生器
func (gen *myGenerator) init() error {
	var buf bytes.Buffer
	buf.WriteString("Initializing the load genrator...")

	// 载荷的并发量 = 载荷的响应超时时间 / 载荷的发送间隔时间
	// 1e9/gen.lps 表示就是根据使用方对每秒载荷发送量的设定计算出的载荷发生器的间隔时间，单位ns
	var total64 = int64(gen.timeoutNS)/int64(1e9/gen.lps) + 1
	if total64 > math.MaxInt32 {
		total64 = math.MaxInt32
	}

	gen.concurrency = uint32(total64)
	//计算并发量的最大意义是：为了约束并发运行的goroutine的数据提供依据
	tickets, err := lib.NewGoTickets(gen.concurrency)
	if err != nil {
		return err
	}

	gen.tickets = tickets

	buf.WriteString(fmt.Sprintf("Done. (concurrency=%d)", gen.concurrency))
	logger.Infoln(buf.String())

	return nil
}

// callOne 会想载荷承受方发起一次调用
func (gen *myGenerator) callOne(rawReq *lib.RawReq) *lib.RawResp {
	// AddInt64原子性的将val的值添加到 gen.callCount 并返回新值
	atomic.AddInt64(&gen.callCount, 1)
	if rawReq == nil {
		return &lib.RawResp{ID: -1, Err: errors.New("Invalid raw request")}
	}

	start := time.Now().UnixNano()
	resp, err := gen.caller.Call(rawReq.Req, gen.timeoutNS)
	end := time.Now().UnixNano()

	elapsedTime := time.Duration(end - start)
	var rawResp lib.RawResp

	if err != nil {
		errMsg := fmt.Sprintf("Sync Call Error: %s.", err)
		rawResp = lib.RawResp{
			ID:     rawReq.ID,
			Err:    errors.New(errMsg),
			Elapse: elapsedTime}
	} else {
		rawResp = lib.RawResp{
			ID:     rawReq.ID,
			Resp:   resp,
			Elapse: elapsedTime}
	}

	return &rawResp
}

// asyncSend 会异步地调用承受方接口
func (gen *myGenerator) asyncCall() {
	// 从goroutine票池获得一张goroutine票
	// 一旦goroutine池中无票可拿，就阻塞于此
	gen.tickets.Take()
	go func() {
		defer func() {
			if p := recover(); p != nil {
				err, ok := interface{}(p).(error)
				var errMsg string

				if ok {
					errMsg = fmt.Sprintf("Async Call Panic! (error: %s)", err)
				} else {
					errMsg = fmt.Sprintf("Async Call Panic! (clue: %#v)", p)
				}

				logger.Errorln(errMsg)
				result := &lib.CallResult{
					ID:   -1,
					Code: lib.RET_CODE_FATAL_CALL,
					Msg:  errMsg}

				gen.sendResult(result)
			}
			// 归还goroutine票
			gen.tickets.Return()
		}()

		rawReq := gen.caller.BuildReq()

		// 调用状态: 0,未调用或调用中、1,调用完成、2,调用超时
		var callStatus uint32
		timer := time.AfterFunc(gen.timeoutNS, func() {
			if !atomic.CompareAndSwapUint32(&callStatus, 0, 2) {
				return
			}

			result := &lib.CallResult{
				ID:     rawReq.ID,
				Req:    rawReq,
				Code:   lib.RET_CODE_WARNING_CALL_TIMEOUT,
				Msg:    fmt.Sprintf("Timeout! (expected: < %v)", gen.timeoutNS),
				Elapse: gen.timeoutNS,
			}

			gen.sendResult(result)
		})

		rawResp := gen.callOne(&rawReq)
		if !atomic.CompareAndSwapUint32(&callStatus, 0, 1) {
			return
		}

		timer.Stop()

		var result *lib.CallResult
		if rawResp.Err != nil {
			result = &lib.CallResult{
				ID:     rawResp.ID,
				Req:    rawReq,
				Code:   lib.RET_CODE_ERROR_CALL,
				Msg:    rawResp.Err.Error(),
				Elapse: rawResp.Elapse,
			}
		} else {
			result = gen.caller.CheckResp(rawReq, *rawResp)
			result.Elapse = rawResp.Elapse
		}

		gen.sendResult(result)
	}()
}

// sendResult 用于发送调用结果
func (gen *myGenerator) sendResult(result *lib.CallResult) bool {
	// LoadUint32原子性的获取 gen.status 的值
	if atomic.LoadUint32(&gen.status) != lib.STATUS_STARTED {
		gen.printIgnoredResult(result, "stopped load generator")
		return false
	}

	select {
	case gen.resultCh <- result:
		return true
	default:
		gen.printIgnoredResult(result, "full result channle")
		return false
	}
}

// printIgnoredResult 打印被忽略的结果
func (gen *myGenerator) printIgnoredResult(result *lib.CallResult, cause string) {
	resultMsg := fmt.Sprintf(
		"ID=%d, Code=%d, Msg=%s, Elapse=%v",
		result.ID, result.Code, result.Msg, result.Elapse)
	logger.Warnf("Ignored result: %s. (cause: %s)\n", resultMsg, cause)
}

// prepareStop 用于为停止载荷发生器做准备
func (gen *myGenerator) prepareToStop(ctxError error) {
	logger.Infoln("Prepare to stop load generator (cause: %s)...", ctxError)
	// CompareAndSwapUint32原子性的比较 gen.status 和 lib.STATUS_STARTED ，如果相同则将 lib.STATUS_STOPPING 赋值给 gen.status 并返回真
	atomic.CompareAndSwapUint32(&gen.status, lib.STATUS_STARTED, lib.STATUS_STOPPING)

	logger.Infof("Closing result channel...")
	close(gen.resultCh)

	atomic.StoreUint32(&gen.status, lib.STATUS_STOPPED)
}

/**
genLoad 会产生载荷并向承受方发送
	总体上控制调用流程的执行，该方法接收节流阀throttle作为参数
	在方法中，使用了一个for循环周期性地向被测软件发送载荷，这个周期的长短由节流阀控制

	在循环体的结尾处，如果lps字段的值大于0，就表示节流阀是有效并需要使用的。这时，利用select语句等待节流阀的到期通知
	一旦接收到了这样一个通知，就立即开始下一次迭代(即开始生成并发送下一个载荷)

	当然，如果在等待节流阀到期通知的过程中接收到了上下文的“信号”，就需要立即为停止载荷发生器做准备
*/
func (gen *myGenerator) genLoad(throttle <-chan time.Time) {
	for {
		select {
		case <-gen.ctx.Done():
			gen.prepareToStop(gen.ctx.Err())
			return
		default:
		}

		gen.asyncCall()

		if gen.lps > 0 {
			select {
			case <-throttle:
			case <-gen.ctx.Done():
				gen.prepareToStop(gen.ctx.Err())
				return
			}
		}
	}
}

// Start 会启动载荷发生器
func (gen *myGenerator) Start() bool {
	logger.Infoln("Starting load generator...")
	// 检查是否具备可启动的状态，顺便设置状态为正启动
	// CompareAndSwapUint32原子性的比较 gen.status 和 lib.STATUS_ORIGINAL ，如果相同则将 lib.STATUS_STARTING 赋值给 gen.status 并返回真
	if !atomic.CompareAndSwapUint32(&gen.status, lib.STATUS_ORIGINAL, lib.STATUS_STARTING) {
		if !atomic.CompareAndSwapUint32(&gen.status, lib.STATUS_STOPPED, lib.STATUS_STARTING) {
			return false
		}
	}

	// channel 即指通道类型，也指代可以传递某种类型的值的通道
	// 通道是在多个goroutine之间传递数据和同步的重要手段，而对通道的操作本身也是同步的
	// 设定节流阀
	var throttle <-chan time.Time
	if gen.lps > 0 {
		interval := time.Duration(1e9 / gen.lps)
		logger.Infof("Setting throttle (%v)...", interval)
		throttle = time.Tick(interval)
	}

	// 初始化上下文和取消函数
	gen.ctx, gen.cancelFunc = context.WithTimeout(context.Background(), gen.durationNS)

	// 初始化调用计数
	gen.callCount = 0

	// 设置状态为已启动
	// StoreUint32原子性的将 lib.STATUS_STARTED 的值保存到 gen.status
	atomic.StoreUint32(&gen.status, lib.STATUS_STARTED)

	go func() {
		// 生成并发送载荷
		logger.Infoln("Generating loads...")
		gen.genLoad(throttle)
		logger.Infof("Stopped. (call count: %d)", gen.callCount)
	}()

	return true
}

func (gen *myGenerator) Stop() bool {
	if !atomic.CompareAndSwapUint32(&gen.status, lib.STATUS_STARTED, lib.STATUS_STOPPING) {
		return false
	}

	gen.cancelFunc()

	for {
		if atomic.LoadUint32(&gen.status) == lib.STATUS_STOPPED {
			break
		}
		time.Sleep(time.Microsecond)
	}
	return true
}

func (gen *myGenerator) Status() uint32 {
	return atomic.LoadUint32(&gen.status)
}

func (gen *myGenerator) CallCount() int64 {
	return atomic.LoadInt64(&gen.callCount)
}

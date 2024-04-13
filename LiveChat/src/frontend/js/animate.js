// 用于记录上次pso的临时变量
let target_point_x = null, target_point_y = null;

let move = (function() {
    /**
     * 动画逻辑
     * @param (动画目标) target  dom 节点
     * @param (方向) direction left/top/botoom/right
     * @param (目标巨鹿) aim_distance 数字类型
     * @param (速度) speed int
     * @param (开始执行函数) 函数类型
     */
    return function(target, direction, aim_distance, speed) {
        speed = typeof speed === "number" ? speed : 5;

        // 方向设置
        direction === "left" && (temp_target_point = target_point_x = target_point_x !== null
                    ? target_point_x - aim_distance
                    : target.offsetLeft - aim_distance);

        direction === "right" && (temp_target_point = target_point_x = target_point_x !== null
                    ? target_point_x - aim_distance
                    : target.offsetLeft + aim_distance);
        
        direction === "top" && (temp_target_point = target_point_y = target_point_x !== null
                    ? target_point_y - aim_distance
                    : target.offsetLeft - aim_distance);
        
        direction === "bottom" && (temp_target_point = target_point_y = target_point_y !== null
                    ? target_point_y - aim_distance
                    : target.offsetTop + aim_distance);
        
        
        return function() {
            if (direction === "left") {
                (target.offsetLeft > temp_target_point) 
                    ? (target.style.left = target.offsetLeft - speed + "px") 
                    : (target.style.left = temp_target_point + "px");
            } else if (direction === "right") {
                (target.offsetLeft < temp_target_point) 
                    ? (target.style.left = target.offsetLeft + speed + "px") 
                    : (target.style.left = temp_target_point + "px");
            } else if (direction === "top") {
                (target.offsetTop > temp_target_point) 
                    ? (target.style.top = target.offsetTop - speed + "px") 
                    : (target.style.top = temp_target_point + "px");
            } else if (direction === "bottom") {
                (target.offsetTop < temp_target_point) 
                    ? (target.style.top = target.offsetTop + speed + "px") 
                    : (target.style.top = temp_target_point + "px");
            }

            return temp_target_point;
        }
    }
})();


/**
 * 上下左右动画小插件
 * @param {要执行动画的目标} target dom节点
 * @param {方向} direction left,right,top,bottom
 * @param {要移动的距离} aim_distance 数字类型
 * @param {速度} speed 数字类型
 * @param {回调函数} cb 函数
 * @return {fn} 真正的动画调用者
 */
function animation(target, direction, aim_distance, speed, cb = function() {
    console.log("callback called");
}) {
    cb = typeof speed === "function" ? speed : cb;
    let queueObj = [];
    // 调用新的animation的时候，重置一下，不然每次都是叠加的
    target_point_x = null, target_point_y = null;
    queueObj.next = function (n_dir, queue_x, speed) {
        this.push({
            fn: move(target, n_dir, queue_x, speed),
            dir: n_dir,
            last_x: queue_x
        });
        return queueObj;
    }

    return (function() {
        /**
         * 真正执行动画的
         * @param {判断时候结束的条件} ani fn
         * @param {方向} read_dir left top bottom  right
         * @return {动画队列数组} 用于依次开启动画的队列数组
         * 
         * requestAnimationFrame 是一个 JavaScript API，主要用于在浏览器中请求动画下一帧的绘制
         * 这个方法允许开发者告诉浏览器他们希望执行动画，并且请求浏览器在下一次重绘之前调用特定的回调函数来更新动画
         * 这确保了动画的绘制与浏览器屏幕的刷新率同步，通常与垂直同步（VSync）信号保持一致，从而提供了更平滑、更高效的动画性能
         */
        function real(ani, read_dir) {
            let temp = ani, tempArr;

            function fn() {
                if (read_dir === "left" || read_dir === "right") {
                    if (target.offsetLeft !== temp()) {
                        requestAnimationFrame(fn)
                    } else {
                        queueObj[0] && (tempArr = queueObj.shift());
                        tempArr || cb();
                        tempArr && real(tempArr.fn, tempArr.dir)
                    }
                } else if (read_dir === "top" || read_dir === "bottom") {
                    if (target.offsetTop !== temp()) {
                        requestAnimationFrame(fn);
                    } else {
                        queueObj[0] && (tempArr = queueObj.shift());
                        tempArr || cb();
                        tempArr && real(tempArr.fn, tempArr.dir);
                    }
                }
            }
            requestAnimationFrame(fn);
        }

        real(move(target, direction, aim_distance, speed), direction);
        return queueObj;
    })();
}

export default animation
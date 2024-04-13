// eventsource-polyfill 是一个针对 Node.js 环境的库，它提供了一种方式来模拟浏览器中的 EventSource 接口
// EventSource 是一个能够接收服务器推送事件的接口，通常用于构建服务器与客户端之间的实时通信
require('eventsource-polyfill')

// webpack-hot-middleware/client?noInfo=true&reload=true 是一个用于在 Node.js 应用程序中实现热加载的客户端设置
// 这是在使用 webpack-hot-middleware 这个中间件时，客户端（通常是浏览器）与服务端建立 WebSocket 连接的参数配置
// noInfo: 当设置为 true 时，它告诉中间件不要在控制台输出任何信息，保持静默
// reload: 当设置为 true 时，如果热加载失败，它会自动重新加载整个页面
// webpack-hot-middleware 是一个 Express 中间件，用于将 Webpack 的热模块替换（HMR）功能添加到你的开发环境中
// 这样，在开发过程中，当你的源代码发生变化时，Webpack 会编译变化的部分，并通过这个中间件推送给客户端
//  客户端接收到这些更新后可以立即替换掉旧的模块，从而实现实时预览变化
var hotClient = require('webpack-hot-middleware/client?noInfo=true&reload=true')

hotClient.subscribe(function(event) {
    if (event.action === 'reload') {
        window.location.reload()
    }
})
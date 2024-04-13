require('./check-versions')()

var config = require('../config')
if (!process.env.NODE_ENV) {
    process.env.NODE_ENV = JSON.parse(config.dev.env.NODE_ENV)
}

var opn = require('opn')
var path = require('path')
var express = require('express')
var webpack = require('webpack')

// http-proxy-middleware 是一个在 Node.js 环境中使用的中间件
// 它允许开发者为 Express、Connect 或其他类似框架的服务器创建 HTTP 代理
var proxyMiddleware = require('http-proxy-middleware')
var webpackConfig = require('./webpack.dev.conf')

// default port where dev server listens for incoming traffic
var port = process.env.PORT || config.dev.port

// automatically open browser, if not set will be false
var autoOpenBrowser = !!config.dev.autoOpenBrowser

// Define HTTP proxies to your custom API backend
// https://github.com/chimurai/http-proxy-middleware
var proxyTable = config.dev.proxyTable

var app = express()
var compiler = webpack(webpackConfig)

// webpack-dev-middleware 是一个用于在开发环境中将Webpack的编译结果提供给服务器中间件的Node.js模块
// 当使用 webpack-dev-middleware 时，它会处理来自服务器的请求，并提供Webpack编译后的文件
// 这意味着你可以在本地服务器上实时预览你的应用，而无需每次修改代码后都手动构建和刷新页面
var devMiddleware = require('webpack-dev-middleware')(compiler, {
    publicPath: webpackConfig.output.publicPath,
    quite: true
})

// webpack-hot-middleware 是一个 Node.js 中间件，它允许你在开发过程中实现模块热替换（Hot Module Replacement，HMR）
// 这个中间件与 webpack-dev-middleware 配合使用，可以提供热加载的功能，即在代码变更时不需要刷新整个页面，只需替换修改的部分即可
// 当使用 webpack-hot-middleware 时，它会向 Webpack 编译过程中注入特定的代码，以便在浏览器中建立与编译服务器的 WebSocket 连接
// 这样，当代码更新时，中间件能够通过这个 WebSocket 连接通知浏览器，浏览器端的 HMR 客户端会接收这些更新并替换相应的模块
var hotMiddleware = require('webpack-hot-middleware')(compiler, {
    log:() => {}
})

// force page reload when html-webpack-plugin template changes
compiler.plugin('compilation', function(compilation) {
    compilation.plugin('html-webpack-plugin-after-emit', function(data, cb) {
        hotMiddleware.publish({action: 'reload'})
        cb()
    })
})

// proxy api requests
Object.keys(proxyTable).forEach(function (context) {
    var options = proxyTable[context]
    if (typeof options === 'string') {
      options = { target: options }
    }
    app.use(proxyMiddleware(options.filter || context, options))
  })

// handle fallback for HTML5 history API
// connect-history-api-fallback 是一个 Node.js 中间件，用于在开发基于 HTML5 History API 的单页面应用程序（SPA）时提供回退支持
// 当使用 HTML5 History API 时，如果直接访问某个历史路径（不是通过点击链接或重定向）
// 浏览器不会发送请求到服务器，因为浏览器期望该路径由前端路由处理。但是，如果直接访问这些路径，服务器上并没有对应的静态资源，就会返回 404 错误
// connect-history-api-fallback 中间件解决了这个问题。它拦截所有不是文件请求的 GET 请求，并将它们重定向到单个入口点（通常是 index.html）
app.use(require('connect-history-api-fallback')())

// serve webpack bundle output
app.use(devMiddleware)

// enable hot-reload and state-preserving
// compilation error display
app.use(hotMiddleware)

// serve pure static assets
var staticPath = path.posix.join(config.dev.assetsPublicPath, config.dev.assetsSubDirectory)
app.use(staticPath, express.static('./static'))

var uri = 'http://localhost:' + port

var _resolve
var readyPromise = new Promise(resolve => {
    _resolve = resolve
})

console.log('> Starting dev server...')
devMiddleware.waitUntilValid(() => {
    console.log('> Listening at ' + uri + '\n')
    // when env is testing, don't need open it
    if (autoOpenBrowser && process.env.NODE_ENV !== 'testing') {
        opn(uri)
    }
    _resolve()
})

var server = app.listen(port)

module.exports = {
    ready: readyPromise,
    close: () => {
        server.close()
    }
}
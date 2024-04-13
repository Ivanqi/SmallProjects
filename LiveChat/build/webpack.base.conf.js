var path = require('path')
var utils = require('./utils')
var config = require('../config')
var vueLoaderConfig = require('./vue-loader.conf')

function resolve(dir) {
    return path.join(__dirname, '..', dir)
}

module.exports = {
    entry: {
        app: './src/frontend/main.js'
    },
    output: {
        path: config.build.assetsRoot,
        filename: '[name].js',
        publicPath: process.env.NODE_ENV === 'production' 
            ? config.build.assetsPublicPath 
            : config.dev.assetsPublicPath
    },
    resolve: {
        extensions: ['.js', '.vue', '.json'],
        alias: {
            'vue$': 'vue/dist/vue.esm.js',
            '@': resolve('src')
        }
    },
    module: {
        // module.rules 是一个数组，用于定义不同类型模块的加载规则。每一条规则都是一个对象，包含以下属性
        // test: 一个正则表达式，用于匹配需要处理的文件类型
        // loader: 用来处理匹配到的文件的加载器（loader）名称
        // options: 传递给加载器的选项
        rules: [
            {
                test: /\.vue$/, // test: /\.vue$/ 匹配以 .vue 结尾的文件，即单文件 Vue 组件
                loader: 'vue-loader', // loader: 'vue-loader' 使用 vue-loader 来处理这些 Vue 组件文件
                options: vueLoaderConfig // options: vueLoaderConfig 传递一个名为 vueLoaderConfig 的配置对象给 vue-loader
            },
            {
                test: /\.js$/, // /\.js$/ 匹配以 .js 结尾的 JavaScript 文件
                // loader: 'babel-loader' 使用 babel-loader 来处理这些 JavaScript 文件
                // 通常用于将 ES6+ 代码转换为向后兼容的 ES5 代码
                loader: 'babel-loader', 
                // include: [resolve('src'), resolve('test')] 指定 babel-loader 应该只处理 src 和 test 目录下的文件
                include: [resolve('src'), resolve('test')] 
            },
            {
                // 用于处理图片文件（包括 .png, .jpg, .jpeg, .gif, .svg 格式
                test: /\.(png|jpe?g|gif|svg)(\?.*)?$/, // 正则表达式，用于匹配特定扩展名的文件
                loader: 'url-loader',
                options: {
                    // limit: 限制文件大小（以字节为单位），如果文件大小小于或等于这个值，则文件将被转换为 base64 编码的字符串
                    // 并直接嵌入到打包后的 JavaScript 文件中；如果文件大小大于这个值，则使用 file-loader 将文件输出到指定目录
                    limit: 10000,
                    // name: 指定输出文件的名称，这里使用了 utils.assetsPath 函数来生成文件路径
                    name: utils.assetsPath('img/[name].[hash:7].[ext]')
                }
            },
            {
                // 用于处理字体文件（包括 .woff, .woff2, .eot, .ttf, .otf 格式）
                test: /\.(woff2?|eot|ttf|otf)(\?.*)?$/,
                loader: 'url-loader',
                options: {
                    limit: 10000,
                    name: utils.assetsPath('fonts/[name].[hash:7].[ext]')
                }
              }
        ]
    }
}
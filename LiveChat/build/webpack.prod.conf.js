var path = require('path')
var utils = require('./utils')
var webpack = require('webpack')
var config = require('../config')

// webpack-merge 是一个用于合并 Webpack 配置的实用工具
// 在构建复杂的 Webpack 配置时，通常会将配置分成几个部分，比如基础配置、开发环境配置和生产环境配置
// webpack-merge 允许你将不同的配置片段合并成一个完整的配置对象，这样可以避免重复编写配置，并使得配置更加模块化和可维护
var merge = require('webpack-merge')

var baseWebpackConfig = require('./webpack.base.conf')

// copy-webpack-plugin 是一个 Webpack 插件，用于将单个文件或整个目录从源位置复制到构建目录
// 这个插件在资源管理方面非常有用，尤其是在需要将静态资源（如图片、字体文件、robots.txt 等）复制到生产环境的构建目录时
var CopyWebpackPlugin = require('copy-webpack-plugin')

// html-webpack-plugin 是一个Webpack插件，用于生成HTML文件，并将Webpack打包生成的脚本和样式标签注入到这个HTML文件中
var HtmlWebpackPlugin = require('html-webpack-plugin')

// extract-text-webpack-plugin 是一个 Webpack 插件，用于将打包过程中生成的 CSS 文件提取到一个单独的文件中
var ExtractTextPlugin = require('extract-text-webpack-plugin')

// optimize-css-assets-webpack-plugin 是一个Webpack插件，用于优化和压缩CSS
// 在Webpack的构建过程中，这个插件可以帮助减少CSS文件的大小，提高加载速度和缓存效率
var OptimizeCSSPlugin = require('optimize-css-assets-webpack-plugin')

var env = config.build.env

var webpackConfig = merge(baseWebpackConfig, {
    module: {
        rules: utils.styleLoaders({
            sourceMap: config.build.productionSourceMap,
            extract: true
        })
    },
    devtool: config.build.productionSourceMap ? '#source-map' : false,
    output: {
        path: config.build.assetsRoot,
        filename: utils.assetsPath('js/[name].[chunkhash].js'),
        chunkFilename: utils.assetsPath('js/[id].[chunkhash].js')
    },
    plugins: [
        // http://vuejs.github.io/vue-loader/en/workflow/production.html
        new webpack.DefinePlugin({
            'process.env': env
        }),
        new webpack.optimize.UglifyJsPlugin({
            compress: {
                warnings: false
            },
            sourceMap: true
        }),
        // extract css into its own file
        new ExtractTextPlugin({
            // 指定了一个插件实例，告诉 Webpack 将所有 CSS 代码提取到名为 xxx.css 的文件中
            filename: utils.assetsPath('css/[name].[contenthash].css')
        }),
        // Compress extracted CSS
        // We are using this plugin so that possible duplicated CSS from different components can be deduped.
        new OptimizeCSSPlugin({
            // cssProcessorOptions，这是一个配置选项，用于传递给CSS处理器
            // 这个选项允许你指定如何处理CSS代码的细节，例如可以设置特定的压缩选项
            cssProcessorOptions: {
                safe: true, // 避免使用可能导致问题的优化
            }
        }),
        // generate dist index.html with correct asset hash for caching.
        // you can customize output by editing /index.html
        // see https://github.com/ampedandwired/html-webpack-plugin
        new HtmlWebpackPlugin({
            filename: config.build.index,   // 输出的HTML文件名
            template: 'index.html',         // 模板文件路径，插件会根据这个文件生成新的HTML文件
            inject: true, // // 将打包生成的脚本和样式标签注入到HTML文件中
            minify: {
                removeComments: true,
                collapseWhitespace: true,
                removeAttributeQuotes: true
                // more options:
                // https://github.com/kangax/html-minifier#options-quick-reference
            },
            // necessary to consistently work with multiple chunks via CommonsChunkPlugin
            chunksSortMode: 'dependency'
        }),
        // split vendor js into its own file
        new webpack.optimize.CommonsChunkPlugin({
            name: 'vendor',
            minChunk: function (module, count) {
                // any required modules inside node_modules are extracted to vendor
                return (
                    module.resource &&
                    /\.js$/.test(module.resource) &&
                    module.resource.indexOf(
                        path.join(__dirname, '../node_modules')
                    ) === 0
                )
            }
        }),
        // extract webpack runtime and module manifest to its own file in order to
        // prevent vendor hash from being updated whenever app bundle is updated
        // webpack.optimize.CommonsChunkPlugin 是一个在Webpack中用于代码分割和优化的插件
        // 它可以将多个入口（entry）共用的模块（通常是第三方库或框架）提取到一个单独的chunk中，这样做的好处是可以提高缓存利用率
        // 因为只要这个公共chunk不发生变化，用户在浏览不同页面时就不需要重新下载这个chunk
        new webpack.optimize.CommonsChunkPlugin({
            name: 'mainfest',   // 公共chunk的名称
            chunks: ['vendor']
        }),
        // copy custom static assets
        new CopyWebpackPlugin([
            {
                from: path.resolve(__dirname, '../static'),
                to: config.build.assetsSubDirectory,
                ignore: ['.*']
            }
        ])
    ]
})

if (config.build.productionGzip) {
    // compression-webpack-plugin 是一个Webpack插件，用于在Webpack构建过程中生成压缩版的资源文件，比如gzip、brotli等格式的压缩文件
    // 这个插件可以帮助减小服务器发送到客户端的资源文件大小，从而提高加载速度和整体性能
    var CompressionWebpackPlugin = require('compression-webpack-plugin')

    webpackConfig.plugins.push(
        new CommonsChunkPlugin({
            asset: '[path].gz[query]',
            algorithm: 'gzip',
            test: new RegExp(
                '\\.(' +
                config.build.productionGzipExtensions.join('|') +
                ')$'

            ),
            threshold: 10240,
            minRatio: 0.8
        })
    )
}

if (config.build.bundleAnalyzerReport) {
    // webpack-bundle-analyzer 是一个用于分析 Webpack 打包输出结果的插件
    // 这个工具通过可视化方式帮助开发者了解打包后的文件大小和组成，从而优化资源使用和加快加载时间
    var BundleAnalyzerPlugin = require('webpack-bundle-analyzer').BundleAnalyzerPlugin
    webpackConfig.plugins.push(new BundleAnalyzerPlugin)
}

module.exports = webpackConfig
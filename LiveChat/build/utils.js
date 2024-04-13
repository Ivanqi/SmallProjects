var path = require('path')
var config = require('../config')
/**
 * extract-text-webpack-plugin 是一个用于 webpack 的插件，它的主要用途是从打包的文件中提取文本内容
 * 通常是 CSS 文件，到一个单独的文件中。这样做的好处包括
 * 1. 可以将 CSS 文件分离出来，便于浏览器缓存，提高页面加载速度
 * 2. 避免将 CSS 代码内嵌到 JavaScript 文件中，减少 JavaScript 文件的体积
 * 3. 方便对 CSS 文件进行压缩和优化
 */
var ExtractTextPlugin = require('extract-text-webpack-plugin')

exports.assetsPath = function(_path) {
    var assetsSubDirectory = process.env.NODE_ENV === 'production' 
        ? config.build.assetsSubDirectory
        : config.dev.assetsSubDirectory
    
    return path.posix.join(assetsSubDirectory, _path)

}

exports.cssLoaders = function(options) {
    options = options || {}

    var cssLoaders = {
        loader: 'css-loader',
        options: {
            minimize: process.env.NODE_ENV === 'production',
            sourceMap: options.sourceMap
        }
    }

    // generate loader string to be used with extract text plugin
    function generateLoaders(loader, loaderOptions) {
        var loaders = [cssLoaders]
        if (loader) {
            loaders.push({
                loader: loader + '-loader',
                options: Object.assign({}, loaderOptions, {
                    sourceMap: options.sourceMap
                })
              })
        }

        // Extract CSS when that option is specified (which is the case during production build)
        if (options.extract) {
            return ExtractTextPlugin.extract({
                use: loaders,
                fallback: 'vue-style-loader'
            })
        } else {
            return ['vue-style-loader'].concat(loaders)
        }
    }

    // https://vue-loader.vuejs.org/en/configurations/extract-css.html
    return {
        css: generateLoaders(),
        postcss: generateLoaders(),
        less: generateLoaders('less'),
        sass: generateLoaders('sass', { indentedSyntax: true }),
        style: generateLoaders('sass'),
        stylus: generateLoaders('stylus'),
        styl: generateLoaders('stylus')
    }
}

//  Generate loaders for standalone style files (outside of .vue)
exports.styleLoaders = function(options) {
    var output = []
    var loaders = exports.cssLoaders(options)
    for (var extension in loaders) {
        var loader = loaders[extension]
        output.push({
            test: new RegExp('\\.' + extension + '$'),
            use: loader
        })
    }

    return output
}
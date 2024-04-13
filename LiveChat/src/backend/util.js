function copyProperties(target, source) {
    if (!target || !source) {
        return error('target and source must not be null')
    }

    for (var key in source) {
        target[key] = source[key]
    }
}
  
var response = {
    ok: function (data) {
        this.code = 1
        this.rData = data
        return this
    },
    fail: function (msg) {
        this.code = 0
        this.rMsg = msg
        return this
    }
}
  
exports.copyProperties = copyProperties
exports.response = response
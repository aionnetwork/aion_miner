const dateFormat = require('dateformat')
const colors = require('colors')

const severityValues = {
    'debug': 1,
    'warning': 2,
    'error': 3,
    'special': 4
}
function severityToColor(severity, text) {
    switch(severity) {
        case 'debug': return text.green
        case 'warning': return text.yellow
        case 'error': return text.red
        case 'special': return text.cyan.underline
        default:
            console.log("unknown severity " + severity)
            return text.italic
    }
}

module.exports = function (configuration) {
    let logLevelInt = severityValues[configuration.logLevel]
    let logColors = configuration.logColors
    let log = function(severity, system, component, text, subcat) {
        if (severityValues[severity] < logLevelInt) 
            return
        if (subcat){
            let realText = subcat
            let realSubCat = text
            text = realText
            subcat = realSubCat
        }
        let logString, 
            entryDesc = dateFormat(new Date(), 'yyyy-mm-dd HH:MM:ss') + ' [' + system + ']\t'
        if (logColors) {
            entryDesc = severityToColor(severity, entryDesc)
            logString = entryDesc + ('[' + component + '] ').italic
            if (subcat)
                logString += ('(' + subcat + ') ').bold.grey
            logString += text.grey
        }
        else {
            logString = entryDesc + '[' + component + '] '
            if (subcat)
                logString += '(' + subcat + ') '
            logString += text
        }
        console.log(logString)
    }
    let _this = this
    Object.keys(severityValues).forEach((logType)=>{
        _this[logType] = function(){
            let args = Array.prototype.slice.call(arguments, 0)
            args.unshift(logType)
            log.apply(this, args)
        }
    })
}
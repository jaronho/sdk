/***********************************************************************
 ** Author:	jaron.ho
 ** Date:	2015-09-08
 ** Brief:	log functions
 ***********************************************************************/
function createLog(showRegion, showTime, printFunc, level) {
    level = 'number' == typeof(level) && !isNaN(level) ? level : 1;
    var logString = "";
    function padDate(num, n) {
      var len = num.toString().length;
      while (len < n) {
        num = "0" + num;
        ++len;
      }
      return num;
    }
    function nowDate() {
        var dt = new Date();
        return padDate(dt.getFullYear(), 4) + "-" + padDate(dt.getMonth() + 1, 2) + "-" + padDate(dt.getDate(), 2)
               + " " +
               padDate(dt.getHours(), 2) + ":" + padDate(dt.getMinutes(), 2) + ":" + padDate(dt.getSeconds(), 2)
               + "." + padDate(dt.getMilliseconds(), 3);
    }
    function printLocal(str) {
        if ('number' == typeof(str)) {
            str = str.toString();
        }
        var dateStr = "";
        if (showTime) {
            dateStr = nowDate() + " ";
        }
        if ('function' == typeof(printFunc)) {
            printFunc(dateStr + str);
        } else {
            if (1 == level) {
                console.log(dateStr + str);
            } else if (2 == level) {
                console.warn(dateStr + str);
            } else if (3 == level) {
                console.error(dateStr + str);
            }
        }
        if (0 == logString.length) {
            logString += str;
        } else {
            logString += "\n" + str;
        }
    }
    function keyToText(key) {
        if ('string' == typeof(key)) {
            return "[\"" + key + "\"]";
        } else if ('number' == typeof(key)) {
            return "[" + key + "]";
        } else {
            return "unknown -- not support key type '" + typeof(key) + "'";
        }
    }
    function valueToText(value) {
        if (undefined == value) {
            return "undefined";
        } else if (null == value) {
            return "null";
        } else if ('string' == typeof(value)) {
            return "\"" + value + "\"";
        } else if ('number' == typeof(value)) {
            return value;
        } else if ('boolean' == typeof(value)) {
            return value ? "true" : "false";
        } else {
            return "unknown -- not support value type '" + typeof(value) + "'";
        }
    }
    function logTableKV(k, obj, tabCount, parentArr) {
        var strOut = "";
        for (var i = 0; i < tabCount * 4; ++i) {
            strOut += " ";
        }
        /* check if is table */
        if (!obj || (!(obj instanceof Array) && 'object' != typeof(obj))) {
            strOut += keyToText(k) + " = " + valueToText(obj) + ",";
            printLocal(strOut);
            return;
        }
        /* check if parent node exist endless loop */
        for (var m = 0, n = parentArr.length; m < n; ++m) {
            if (obj == parentArr[m]) {
                strOut += keyToText(k) + " = " + "unknown" + "," + " -- can not print parent table";
                printLocal(strOut);
                return;
            }
        }
        parentArr.push(obj);        /* record parent node */
        ++tabCount;					/* tab count + 1 */
        printLocal(strOut + keyToText(k) + " = ");
        if (obj instanceof Array) {
            printLocal(strOut + "[");
            for (var j = 0, len = obj.length; j < len; ++j) {
                logTableKV(j, obj[j], tabCount, parentArr);
            }
            printLocal(strOut + "],");
        } else {
            printLocal(strOut + "{");
            for (var key in obj) {
                if (obj.hasOwnProperty(key)) {
                    logTableKV(key, obj[key], tabCount, parentArr);
                }
            }
            printLocal(strOut + "},");
        }
        /* remove record */
        for (var x = 0, y = parentArr.length; x < y; ++x) {
            if (obj == parentArr[x]) {
                parentArr.splice(x, 1);
                return;
            }
        }
    }
    function logOne(obj) {
        if (obj instanceof Array || 'object' == typeof(obj)) {
            var parentArr = [];		/* record parent node */
            parentArr.push(obj);
            if (obj instanceof Array) {
                printLocal("[");
                for (var i = 0, len = obj.length; i < len; ++i) {
                    logTableKV(i, obj[i], 1, parentArr);
                }
                printLocal("]")
            } else {
                printLocal("{");
                for (var key in obj) {
                    if (obj.hasOwnProperty(key)) {
                        logTableKV(key, obj[key], 1, parentArr);
                    }
                }
                printLocal("}")
            }
        } else if ('string' == typeof(obj)) {
            printLocal("\"" + obj + "\"");
        } else if ('number' == typeof(obj)) {
            printLocal(obj);
        } else if ('boolean' == typeof(obj)) {
            printLocal(obj);
        } else {
            printLocal(typeof(obj));
        }
    }
    return function() {
        logString = "";
        if (showRegion) {
            printLocal("[[*********************************************");
        }
        for (var i = 0, len = arguments.length; i < len; ++i) {
            logOne(arguments[i]);
        }
        if (showRegion) {
            printLocal("***********************************************]]");
        }
        return logString;
    };
}
//----------------------------------------------------------------------

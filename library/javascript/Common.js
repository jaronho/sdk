/***********************************************************************
 ** Author:	jaron.ho
 ** Date:	2015-08-11
 ** Brief:	common functions
 ***********************************************************************/
var Common = {};
//----------------------------------------------------------------------
// 断言
function assert(cond, msg) {
    if (!cond) {
        throw new Error('string' == typeof(msg) ? msg : "");
    }
}
//----------------------------------------------------------------------
// 变量是否为空
function isNil(variable) {
	return undefined == variable || null == variable;
}
//----------------------------------------------------------------------
// 变量是否为对象(非undefine,非null,非Array)
function isObject(variable) {
	return variable && !(variable instanceof Array) && 'object' == typeof(variable);
}
//----------------------------------------------------------------------
// 调用函数
function callFunction(func, target) {
	if ('function' == typeof(func)) {
        func.apply(target, Array.prototype.slice.call(arguments, 2));
	}
}
//----------------------------------------------------------------------
// 生成枚举类型,arr - ["aaaa", "bbbb", "cccc", ...],返回{"aaaa"=1, "bbbb"=2, "cccc"=3, ...}
Common.enum = function(arr) {
	if (!(arr instanceof Array)) {
		throw new Error("not support for arr type '" + typeof(arr) + "' in enum");
	}
	var enumObj = {};
	for (var i = 0, len = arr.length; i < len; ++i) {
		if (enumObj.hasOwnProperty(arr[i])) {
			throw new Error("duplicate tag '" + arr[i] + "' in enum");
		}
		enumObj[arr[i]] = i;
	}
	return enumObj;
};
//----------------------------------------------------------------------
// 获取表大小,isExceptNull:是否排除空值
Common.size = function(table, isExceptNull) {
	var capacity = 0;
	if (table && 'object' == typeof(table)) {
		for (var key in table) {
			if (table.hasOwnProperty(key)) {
				capacity += isExceptNull ? ((undefined == table[key] || null == table[key]) ? 0 : 1) : 1;
			}
		}
	}
	return capacity;
};
//----------------------------------------------------------------------
// 查找数据.conditionFunc:条件函数(返回boolean值),isQueryMultiple:是否查询多条
Common.query = function(table, conditionFunc, target, isQueryMultiple) {
	var resultArray = [];
	if (table && 'object' == typeof(table) && 'function' == typeof(conditionFunc)) {
		for (var key in table) {
			if (table.hasOwnProperty(key) && conditionFunc.apply(target, [key, table[key]])) {
				if (isQueryMultiple) {
					resultArray.push(table[key]);
				} else {
					return table[key];
				}
			}
		}
	}
	return isQueryMultiple ? resultArray : null;
};
//----------------------------------------------------------------------
// 深度拷贝
Common.clone = function(obj) {
	function copyObj(obj) {
        if (obj && 'object' == typeof(obj)) {
            var newObj = obj instanceof Array ? [] : {};
            for (var key in obj) {
                newObj[key] = copyObj(obj[key]);
            }
            return newObj;
        }
        return obj;
	}
	return copyObj(obj);
};
//----------------------------------------------------------------------
// 数据拼接
Common.join = function() {
	var arr = [];
	for (var i = 0, len = arguments.length; i < len; ++i) {
		var arg = arguments[i];
		if (arg instanceof Array) {
			for (var key in arg) {
				if (arg.hasOwnProperty(key)) {
					arr.push(arg[key]);
				}
			}
		} else if (undefined != arg && null != arg) {
			arr.push(arg);
		}
	}
	return arr;
};
//----------------------------------------------------------------------
// 检查单个字符占位数
Common.characterPlaceholder = function(ch, charset) {
    if ('number' != typeof(ch)) {
        throw new Error("not support for ch type '" + typeof(ch) + "'");
    }
    /* 计算字符串所占的内存字节数,默认使用UTF-8的编码方式计算,也可制定为UTF-16
     * UTF-8:是一种可变长度的Unicode编码格式,使用一至四个字节为每个字符编码
     * 000000 - 00007F(128个代码)      0zzzzzzz(00-7F)                              一个字节
     * 000080 - 0007FF(1920个代码)     110yyyyy(C0-DF) 10zzzzzz(80-BF)              两个字节
     * 000800 - 00D7FF                                                             Unicode在范围 D800-DFFF 中不存在任何字符
     * 00E000 - 00FFFF(61440个代码)    1110xxxx(E0-EF) 10yyyyyy 10zzzzzz            三个字节
     * 010000 - 10FFFF(1048576个代码)  11110www(F0-F7) 10xxxxxx 10yyyyyy 10zzzzzz   四个字节
     * UTF-16:大部分使用两个字节编码,编码超出65535的使用四个字节
     * 000000 - 00FFFF                                                             两个字节
     * 010000 - 10FFFF                                                             四个字节
     */
    if ('string' == typeof(charset)) {
        charset = charset.toLowerCase();
    }
    if ('utf-16' == charset || 'utf16' == charset) {
        if (ch <= 0xffff) {
            return 2;
        }
    } else {
        if (ch <= 0x007f) {
            return 1;
        } else if (ch <= 0x07ff) {
            return 2;
        } else if (ch <= 0xffff) {
            return 3;
        }
    }
    return 4;
};
//----------------------------------------------------------------------
// 判断字符个数
Common.characterCount = function(str, charset) {
    if ('string' != typeof(str)) {
        throw new Error("not support type '" + typeof(str) + "'");
    }
    var count = 0;
    for (var i = 0, len = str.length; i < len; ++i) {
        count += this.characterPlaceholder(str.charCodeAt(i), charset);
    }
    return count;
};
//----------------------------------------------------------------------
// 截取文件信息
Common.stripFileInfo = function(fullFileName) {
    if ('string' != typeof(fullFileName)) {
        throw new Error("not support type '" + typeof(fullFileName) + "'");
    }
    var dirpos = fullFileName.lastIndexOf('/');
    var dirposTmp = fullFileName.lastIndexOf('\\');
    if (dirpos < dirposTmp) {
        dirpos = dirposTmp;
    }
    dirpos = dirpos >= 0 ? dirpos + 1 : 0;
    var dirname = fullFileName.substring(0, dirpos);
    var filename = fullFileName.substring(dirpos);
    var extpos = filename.lastIndexOf('.');
    extpos = extpos >= 0 ? extpos : filename.length;
    var basename = filename.substring(0, extpos);
    var extname = filename.substring(extpos);
    return {dirname: dirname, filename: filename, basename: basename, extname: extname};
};
//----------------------------------------------------------------------
// 概率值,percentage:百分比,如:0.02表示2%,返回:true,false
Common.probability = function(percentage) {
	if ('number' != typeof(percentage)) {
		throw new Error("not support for percentage type '" + typeof(percentage) + "' in probability");
	}
	if (percentage < 0 || percentage > 1) {
		throw new Error("range of percentage " + percentage + " is wrong in probability");
	}
	var randomValue = Math.random();
	return randomValue > 0 && randomValue <= percentage;
};
//----------------------------------------------------------------------
// 四舍五入
Common.mathRound = function(num) {
	return 'number' == typeof(num) ? Math.floor(num + (num >= 0 ? 0.5 : -0.5)) : 0;
};
//----------------------------------------------------------------------
// 获取指定范围内的随机值.isInteger:是否整型
Common.random = function(minNum, maxNum, isInteger) {
	if ('number' == typeof(minNum) && 'number' == typeof(maxNum)) {
		if (minNum > maxNum) {
			var tmpMinNum = minNum;
			minNum = maxNum;
			maxNum = tmpMinNum;
		}
        minNum = minNum < -9007199254740991 ? -9007199254740991 : minNum;
        maxNum = maxNum > 9007199254740991 ? 9007199254740991 : maxNum;
		var randomValue = Math.random()*(maxNum - minNum) + minNum;
		return isInteger ? Math.floor(randomValue + (randomValue >= 0 ? 0.5 : -0.5)) : randomValue;
	}
	return isInteger ? Math.floor(Math.random() + 0.5) : Math.random();
};
//----------------------------------------------------------------------
// 获取随机值:Array.从数组中随机获得一个数,Integer.获取指定位数内的随机值
Common.getRandom = function(arrayOrInteger) {
	var tempValueArray = [];
	if (arrayOrInteger instanceof Array) {
		tempValueArray = arrayOrInteger;
	} else if (arrayOrInteger && 'object' == typeof(arrayOrInteger)) {
        for (var key in arrayOrInteger) {
            if (arrayOrInteger.hasOwnProperty(key)) {
                tempValueArray.push(arrayOrInteger[key]);
            }
        }
    } else if ('number' == typeof(arrayOrInteger)) {
        arrayOrInteger = Math.abs(arrayOrInteger);
        var maxNum = 0;
        for (var i = 0; i < arrayOrInteger; ++i) {
            maxNum += Math.pow(10, i) * 9;
        }
        maxNum = maxNum > 9007199254740991 ? 9007199254740991 : maxNum;
        return Math.floor(Math.random()*maxNum + 0.5);
	} else {
		throw new Error("not support for arrayOrInteger type '" + typeof(arrayOrInteger) + "' in getRandom");
	}
    return tempValueArray[Math.floor(Math.random()*(tempValueArray.length - 1) + 0.5)];
};
//----------------------------------------------------------------------
// 对数据表洗牌
Common.wash = function(arr) {
	if (arr instanceof Array) {
		var keyList = [];
		for (var key in arr) {
			if (arr.hasOwnProperty(key)) {
				keyList.push(keyList, key);
			}
		}
		for (var i = 0, len = keyList.length; i < len; ++i) {
			key = keyList[i];
			var rand = Math.floor(Math.random()*i + 0.5);
			var randKey = keyList[rand];
			var temp = arr[randKey];
			arr[randKey] = arr[key];
			arr[key] = temp;
		}
	}
	return arr
};
//----------------------------------------------------------------------
// 生成uid,bit:位数(19-24),如:"1447817348611214450"
Common.createUniqueId = function(bit) {
	if ('number' == typeof(bit)) {
        if (bit < 19) {
            bit = 19
        } else if (bit > 24) {
			bit = 24;
		}
	} else {
		bit = 24;
	}
    var nt = Date.now().toString();
    var offset = bit - nt.length;
    if (0 == offset) {
        return nt;
    } else if (offset < 0) {
        return nt.substr(0, bit);
    }
    var max = 0;
    for (var i = 0; i < offset; ++i) {
        max += Math.pow(10, i) * 9;
    }
    function np (num, bit) {
        var len = num.toString().length;
        while (len++ < bit) {
            num = '0' + num;
        }
        return num.toString();
    }
    return nt + np(Math.floor(Math.random()*max + 0.5), offset);
};
//----------------------------------------------------------------------
// 字符串格式化.如:format("{0},{1}","a","b") => "a,b"或format("{name},{age}",{name:"Jim",age:20}) => "Jim,20"
Common.format = function() {
	var regular = null;
	if ('string' == typeof(arguments[0]) && 'object' == typeof(arguments[1])) {
		for (var key in arguments[1]) {
			if (arguments[1].hasOwnProperty(key)) {
				regular = new RegExp("({)" + key + "(})", "g");
				if (regular.test(arguments[0])) {
					arguments[0] = arguments[0].replace(regular, arguments[1][key]);
				}
			}
		}
	} else if ('string' == typeof(arguments[0])) {
		for (var i = 1, len = arguments.length; i < len; ++i) {
			regular = new RegExp("({)" + (i - 1) + "(})", "g");
			if (regular.test(arguments[0])) {
				arguments[0] = arguments[0].replace(regular, arguments[i]);
			}
		}
	}
	return arguments[0];
};
//----------------------------------------------------------------------
// 格式化钱币.如:(100112000, 0) => "100,112,000"或(100112000, 2) => "100,112,000.00"
Common.formatMoney = function(money, bit) {
	bit = (isNaN(bit) || bit < 0 || bit > 20) ? 0 : bit;
	money = parseFloat((money + "").replace(/[^\d\.-]/g, "")).toFixed(bit) + "";
	var integer = money.split(".")[0].split("").reverse();
	var decimals = money.split(".")[1];
	var str = "";
	for (var i = 0; i < integer.length; ++i) {
		str += integer[i] + (0 == (i + 1)%3 && (i + 1) != integer.length ? "," : "");
	}
	return str.split("").reverse().join("") + (decimals ? ("." + decimals) : "");
};
//----------------------------------------------------------------------
// 数字补位.如:padNumber(3, 2) => 03,padNumber(3, 3) => 003, eg..
Common.numberPad = function(num, bit) {
	if ('number' == typeof(num) && 'number' == typeof(bit)) {
		var len = num.toString().length;
		while (len++ < bit) {
			num = '0' + num;
		}
        return num.toString();
	} else {
        throw new Error("not support for num type '" + typeof(num) + "' or bit type '" + typeof(bit) + "'");
    }
};
//----------------------------------------------------------------------
// 简单异或加密
Common.simpleXOR = function(str, key) {
	if ('string' == typeof(str) && str.length > 0 && 'string' == typeof(key) && key.length > 0) {
		var result = "", strLength = str.length, keyLength = key.length;
		for (var i = 0; i < strLength; ++i) {
			result += String.fromCharCode(str.charCodeAt(i) ^ key.charCodeAt(i % keyLength));
		}
		return result;
	}
	return str;
};
//----------------------------------------------------------------------
// 简单ASCII加密
Common.asciiCrypto = function(str, isCrypto, offset) {
	if ('string' == typeof(str) && str.length > 0) {
		offset = 'number' == typeof(offset) && !isNaN(offset) ? (0 == offset ? 1 : offset) : 1;
		var result = "";
		for (var i = 0, len = str.length; i < len; ++i) {
			result += String.fromCharCode(str.charCodeAt(i) + (isCrypto ? (i + offset) : -(i + offset)));
		}
		return result;
	}
	return str;
};
//----------------------------------------------------------------------

/***********************************************************************
 ** Author.	jaron.ho
 ** Date.	2015-08-28
 ** Brief.	file load
 ***********************************************************************/
/*----------------------------------------------------------------------
 * url				// 资源地址,如:http://10.0.0.190/a.json
 * loadCB			// 成功回调函数.参数:(url, result, method),当1==method时,result=成功数据,当2==method时,reuslt=xhr
 * errorCB			// 失败回调函数.参数:(url, result, method),当1==method时,result=错误信息,当2==method时,reuslt=xhr
 * target			// 目标对象,回调函数的宿主对象,可为null
----------------------------------------------------------------------*/
function LoadFileByFS(url, loadCB, errorCB, target) {
	if ('undefined' === typeof require || !require("fs")) {
		return false;
	}
	// is node js
	require("fs").readFile(url, function (err, data) {
		if (err) {
			if ('function' === typeof errorCB) {
				errorCB.apply(target, [url, err, 1]);
			}
		} else {
			if ('function' === typeof loadCB) {
				loadCB.apply(target, [url, data, 1]);
			}
		}
	});
	return true;
}
function LoadFileByXHR(url, loadCB, errorCB, target) {
	var xhr = window.XMLHttpRequest ? new window.XMLHttpRequest() : new ActiveXObject("MSXML2.XMLHTTP");
	xhr.open("GET", url, true);
	if (/msie/i.test(navigator.userAgent) && !/opera/i.test(navigator.userAgent)) {
		xhr.setRequestHeader("Accept-Charset", "utf-8");
		xhr.onreadystatechange = function () {
			if (4 === xhr.readyState) {
				if (200 === xhr.status) {
					if ('function' === typeof loadCB) {
						loadCB.apply(target, [url, xhr, 2]);
					}
				} else {
					if ('function' === typeof errorCB) {
						errorCB.apply(target, [url, xhr, 2]);
					}
				}
			}
		};
	} else {
		if (xhr.overrideMimeType) {
			xhr.overrideMimeType("text\/plain; charset=utf-8");
		}
		xhr.onload = function () {
			if (4 === xhr.readyState) {
				if ('function' === typeof loadCB) {
					loadCB.apply(target, [url, xhr, 2]);
				}
			}
		};
		xhr.onerror = function () {
			if ('function' === typeof errorCB) {
				errorCB.apply(target, [url, xhr, 2]);
			}
		};
	}
	xhr.send(null);
}
function LoadFile(url, loadCB, errorCB, target) {
	if (!LoadFileByFS(url, loadCB, errorCB, target)) {
		LoadFileByXHR(url, loadCB, errorCB, target);
	}
}
/***********************************************************************/
/*----------------------------------------------------------------------
 * url				// 资源地址,如:http://10.0.0.190/a.font
 * loadCB			// 成功回调函数.参数:(url, data)
 * errorCB			// 失败回调函数.参数:(url)
 * target			// 目标对象,回调函数的宿主对象,可为null
 ----------------------------------------------------------------------*/
function LoadBinary(url, loadCB, errorCB, target) {
	function string2Uint8Array(stringData) {
		if (stringData) {
			var arrayData = new Uint8Array(stringData.length);
			for (var i = 0, len = stringData.length; i < len; ++i) {
				arrayData[i] = stringData.charCodeAt(i) & 0xff;
			}
			return arrayData;
		}
	}
	function convertResponseBodyToText(binary) {
		var byteMapping = {};
		for (var i = 0; i < 256; ++i) {
			for (var j = 0; j < 256; ++j) {
				byteMapping[String.fromCharCode(i + j * 256)] = String.fromCharCode(i) + String.fromCharCode(j);
			}
		}
		var rawBytes = window.IEBinaryToArray_ByteStr(binary);
		var lastChr = window.IEBinaryToArray_ByteStr_Last(binary);
		return rawBytes.replace(/[\s\S]/g, function (match) {
					return byteMapping[match];
				}) + lastChr;
	}
	var xhr = window.XMLHttpRequest ? new window.XMLHttpRequest() : new ActiveXObject("MSXML2.XMLHTTP");
	xhr.open("GET", url, true);
	if ((/msie/i.test(navigator.userAgent) && !/opera/i.test(navigator.userAgent) && window.IEBinaryToArray_ByteStr && window.IEBinaryToArray_ByteStr_Last)) {
		// IE-specific logic here
		xhr.setRequestHeader("Accept-Charset", "x-user-defined");
		xhr.onreadystatechange = function () {
			if (4 === xhr.readyState) {
				if (200 === xhr.status) {
					var fileContents = convertResponseBodyToText(xhr["responseBody"]);
					if ('function' === typeof loadCB) {
						loadCB.apply(target, [url, string2Uint8Array(fileContents)]);
					}
				} else {
					if ('function' === typeof errorCB) {
						errorCB.apply(target, [url]);
					}
				}
			}
		};
	} else {
		if (xhr.overrideMimeType) {
			xhr.overrideMimeType("text\/plain; charset=x-user-defined");
		}
		xhr.onload = function () {
			if (4 === xhr.readyState) {
				if (200 === xhr.status) {
					if ('function' === typeof loadCB) {
						loadCB.apply(target, [url, string2Uint8Array(xhr.responseText)]);
					}
				} else {
					if ('function' === typeof errorCB) {
						errorCB.apply(target, [url]);
					}
				}
			}
		};
	}
	xhr.send(null);
}
/***********************************************************************/
/*----------------------------------------------------------------------
 * url				// 资源地址,如:http://10.0.0.190/a.png
 * isCrossOrigin	// 是否跨域
 * loadCB			// 成功回调函数.参数:(url, image)
 * errorCB			// 失败回调函数.参数:(url)
 * target			// 目标对象,回调函数的宿主对象,可为null
 ----------------------------------------------------------------------*/
function LoadImage(url, isCrossOrigin, loadCB, errorCB, target) {
	var img = new Image();
	img.src = url;
	if ((null === isCrossOrigin ? true : isCrossOrigin) && "file://" != location.origin) {
		img.crossOrigin = "Anonymous";
	}
	function loadCallback() {
		img.removeEventListener('load', loadCallback, false);
		img.removeEventListener('error', errorCallback, false);
		if ('function' === typeof loadCB) {
			loadCB.apply(target, [url, img]);
		}
	}
	function errorCallback() {
		img.removeEventListener('load', loadCallback, false);
		img.removeEventListener('error', errorCallback, false);
		if (img.crossOrigin && "anonymous" === img.crossOrigin.toLowerCase()) {
			LoadImage(url, false, loadCB, errorCB, target);
		} else {
			if ('function' === typeof errorCB) {
				errorCB.apply(target, [url]);
			}
		}
	}
	img.addEventListener("load", loadCallback, false);
	img.addEventListener("error", errorCallback, false);
	return img;
}
/************************************************************************/
/*----------------------------------------------------------------------
 * url				// 资源地址,如:http://10.0.0.190/a.js
 * isAsync			// 是否异步
 * isCache			// 是否缓存
 * loadCB			// 成功回调函数.参数:(url)
 * errorCB			// 失败回调函数.参数:(url)
 * target			// 目标对象,回调函数的宿主对象,可为null
 ----------------------------------------------------------------------*/
function LoadScript(url, isAsync, isCache, loadCB, errorCB, target) {
	var scriptElement = document.createElement("script");
	scriptElement.async = isAsync ? true : false;
	if (isCache) {
		scriptElement.src = url;
	} else {
		if (/\?/.test(url)) {
			scriptElement.src = url + "&_t=" + (new Date() - 0);
		} else {
			scriptElement.src = url + "?_t=" + (new Date() - 0);
		}
	}
	function loadCallback() {
		scriptElement.removeEventListener('load', loadCallback, false);
		scriptElement.removeEventListener('error', errorCallback, false);
		scriptElement.parentNode.removeChild(scriptElement);
		if ('function' === typeof loadCB) {
			loadCB.apply(target, [url]);
		}
	}
	function errorCallback() {
		scriptElement.removeEventListener('load', loadCallback, false);
		scriptElement.removeEventListener('error', errorCallback, false);
		scriptElement.parentNode.removeChild(scriptElement);
		if ('function' === typeof errorCB) {
			errorCB.apply(target, [url]);
		}
	}
	scriptElement.addEventListener('load', loadCallback, false);
	scriptElement.addEventListener('error', errorCallback, false);
	document.body.appendChild(scriptElement);
}
/************************************************************************/
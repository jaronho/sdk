/***********************************************************************
 ** Author:	jaron.ho
 ** Date:	2015-08-12
 ** Brief:	http operator
 ***********************************************************************/
/*----------------------------------------------------------------------
 ****************************** xhr properties:
 * onreadystatechange		// 只写,指定readyState变化时的处理函数,即回调函数,必在send方法前设定
 * readyState				// 只读,返回当前请求状态,有[0-4]五种状态
								0 - 请求未初始化,请求对象已经建立,但是还没有初始化,即尚未调用open()方法创建http请求
								1 - 请求已经建立,请求对象已经建立,但是还没有调用send()方法发送http请求
								2 - 请求已经发送,send()方法已经调用,但是当前状态以及http头未知
								3 - 请求处理中,响应中已有部分数据可用了,但是服务器和没有完成响应的生成
								4 - 响应已经完成,数据接受完毕,此时可以用response的系列方法获取完整的回应数据
 * responseBody				// 只读,响应信息正文以unsign byte数组形式返回
 * responseStream			// 只读,以Ado Stream 对象的形式返回响应信息
 * responseText				// 只读,响应信息以字符串形式返回,默认数据编码为UTF-8
 * responseXML				// 只读,响应信息以格式化XML Document对象返回
 * status					// 只读,返回当前请求的http状态码,有多种状态
								1XX,临时响应,表示临时响应并需要请求者继续执行操作的状态代码
									100,继续,请求者应当继续提出请求,服务器返回此代码表示已收到请求的第一部分,正在等待其余部分
									101,切换协议,请求者已要求服务器切换协议,服务器已确认并准备切换
								2XX,成功,表示成功处理了请求的状态代码
									200,成功,服务器已成功处理了请求,通常,这表示服务器提供了请求的网页
									201,已创建,请求成功并且服务器创建了新的资源
									202,已接受,服务器已接受请求,但尚未处理
									203,非授权信息,服务器已成功处理了请求,但返回的信息可能来自另一来源
									204,无内容,服务器成功处理了请求,但没有返回任何内容
									205,重置内容,服务器成功处理了请求,但没有返回任何内容
									206,部分内容,服务器成功处理了部分GET请求
								3XX,重定向,表示要完成请求,需要进一步操作,通常这些状态代码用来重定向
									300,多种选择,针对请求,服务器可执行多种操作,服务器可根据请求者(user agent)选择一项操作,或提供操作列表供请求者选择
									301,永久移动,请求的网页已永久移动到新位置,服务器返回此响应(对GET或HEAD请求的响应)时,会自动将请求者转到新位置
									302,临时移动,服务器目前从不同位置的网页响应请求,但请求者应继续使用原有位置来进行以后的请求
									303,查看其他位置,请求者应当对不同的位置使用单独的GET请求来检索响应时,服务器返回此代码
									304,未修改,自从上次请求后，请求的网页未修改过,服务器返回此响应时,不会返回网页内容
									305,使用代理,请求者只能使用代理访问请求的网页,如果服务器返回此响应,还表示请求者应使用代理
									307,临时重定向,服务器目前从不同位置的网页响应请求,但请求者应继续使用原有位置来进行以后的请求
								4XX,请求错误,这些状态代码表示请求可能出错,妨碍了服务器的处理
									400,错误请求,服务器不理解请求的语法
									401,未授权,请求要求身份验证,对于需要登录的网页,服务器可能返回此响应
									403,禁止,服务器拒绝请求
									404,未找到,服务器找不到请求的网页
									405,方法禁用,禁用请求中指定的方法
									406,不接受,无法使用请求的内容特性响应请求的网页
									407,需要代理授权,此状态代码与401(未授权)类似,但指定请求者应当授权使用代理
									408,请求超时,服务器等候请求时发生超时
									409,冲突,服务器在完成请求时发生冲突,服务器必须在响应中包含有关冲突的信息
									410,已删除,如果请求的资源已永久删除,服务器就会返回此响应
									411,需要有效长度,服务器不接受不含有效内容长度标头字段的请求
									412,未满足前提条件,服务器未满足请求者在请求中设置的其中一个前提条件
									413,请求实体过大,服务器无法处理请求,因为请求实体过大,超出服务器的处理能力
									414,请求的uri过长,请求的uri(通常为网址)过长,服务器无法处理
									415,不支持的媒体类型,请求的格式不受请求页面的支持
									416,请求范围不符合要求,如果页面无法提供请求的范围,则服务器会返回此状态代码
									417,未满足期望值,服务器未满足"期望"请求标头字段的要求
								5XX,服务器错误,这些状态代码表示服务器在尝试处理请求时发生内部错误,这些错误可能是服务器本身的错误,而不是请求出错
									500,服务器内部错误,服务器遇到错误,无法完成请求
									501,尚未实施,服务器不具备完成请求的功能,例如,服务器无法识别请求方法时可能会返回此代码
									502,错误网关,服务器作为网关或代理,从上游服务器收到无效响应
									503,服务不可用,服务器目前无法使用(由于超载或停机维护),通常这只是暂时状态
									504,网关超时,服务器作为网关或代理,但是没有及时从上游服务器收到请求
									505,http版本不受支持,服务器不支持请求中所用的http协议版本
 * statusText				// 只读,返回当前请求的响应状态行
 ****************************** xhr methods:
 * abort					// 停止当前请求,abort()
 * getAllResponseHeaders	// 获取所有http头,以键/值对返回;含Content-Length,Date,URL
 * getResponseHeader		// 从响应中获取指定http头;getResponseHeader(string header)
 * setRequestHeader			// 单独指定某个http头;setRequestHeader(string header, string value)
 * open						// 创建新的http请求;多以open(method,url,ture)或open(method,url,ture,username,pwd)形式建立
 * send						// 发送请求到http服务器并接受回应;send(content)
 ****************************** parameters:
 * method					// http行为(字符串,不区分大小写),POST,GET,PUT,DELETE
 * url						// http地址,如:http://10.0.0.190/login
 * callback					// 回调函数,两个参数:xhr,param,此参数可为null
 * target					// 目标对象,回调函数的宿主对象,可为null
 * headerMap				// 请求头,哈希表结构,如:{"Content-Type" : "text/plain;charset=UTF-8"}
 * content					// 发送内容,undefined,null或字符串
----------------------------------------------------------------------*/
function httpSend(method, url, headerMap, content, callback, target) {
	if ('string' != typeof(method)) {
		throw new Error("not support method type for '" + typeof(method) + "'");
	}
	if (0 == method.length) {
		throw new Error("not support empty method");
	}
	if ('string' != typeof(url)) {
		throw new Error("not support url type for '" + typeof(url) + "'");
	}
	if (0 == url.length) {
		throw new Error("not support empty url");
	}
	var XMLHttpFactories = [
		function() {return new XMLHttpRequest()},
		function() {return new ActiveXObject("Msxml2.XMLHTTP")},
		function() {return new ActiveXObject("Msxml3.XMLHTTP")},
		function() {return new ActiveXObject("Microsoft.XMLHTTP")}
	];
	var xhr = null;
	for (var i = 0, len = XMLHttpFactories.length; i < len; ++i) {
		xhr = XMLHttpFactories[i]();
		if (xhr) {
			break;
		}
	}
	if (!xhr) {
		throw Error("XMLHttpRequest is not supported");
	}
	xhr.open(method, url, true);
	xhr.onreadystatechange = function() {
		if ('function' == typeof(callback)) {
			var args = Array.prototype.slice.call(arguments, 6);
			args.unshift(xhr);
			callback.apply(target, args);
		}
	};
	if (headerMap && 'object' == typeof(headerMap)) {
		for (var header in headerMap) {
			if (headerMap.hasOwnProperty(header) && 'string' == typeof(headerMap[header]) && headerMap[header].length > 0) {
				xhr.setRequestHeader(header, headerMap[header]);
			}
		}
	}
	xhr.send(content);
}
//----------------------------------------------------------------------
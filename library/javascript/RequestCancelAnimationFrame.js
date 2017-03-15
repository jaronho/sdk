// http://paulirish.com/2011/requestanimationframe-for-smart-animating/
// http://my.opera.com/emoller/blog/2011/12/20/requestanimationframe-for-smart-er-animating
// requestAnimationFrame polyfill by Erik Möller. fixes from Paul Irish and Tino Zijdel
// MIT license
(function() {
	var lastTime = 0;
	var prefixes = ['ms', 'moz', 'webkit', 'o'];
	for (var i = 0, len = prefixes.length; i < len; ++i) {
		var prefix = prefixes[i];
		if (!window.requestAnimationFrame) {
			window.requestAnimationFrame = window[prefix + 'RequestAnimationFrame'];
		}
		if (!window.cancelAnimationFrame) {
			window.cancelAnimationFrame = window[prefix + 'CancelAnimationFrame'] || window[prefix + 'CancelRequestAnimationFrame'];
		}
	}
	if (!window.requestAnimationFrame) {
		window.requestAnimationFrame = function(callback) {
			var currTime = new Date().getTime();
			var timeToCall = Math.max(0, 16 - (currTime - lastTime));
			var id = window.setTimeout(function() {
				callback(currTime + timeToCall);
			}, timeToCall);
			lastTime = currTime + timeToCall;
			return id;
		};
	}
	if (!window.cancelAnimationFrame) {
		window.cancelAnimationFrame = function(id) {
			window.clearTimeout(id);
		};
	}
}());
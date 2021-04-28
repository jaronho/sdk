/***********************************************************************
 ** Author:	jaron.ho
 ** Date:	2019-03-13
 ** Brief:  Canvas引擎
 ***********************************************************************/
function createCanvasEngine(fps) {
    var engine = {};
    engine._fps = 'number' === typeof(fps) ? (fps > 24 ? (fps < 60 ? fps : 60) : 24) : 24;
    engine._objectMap = {};
    /* 获取帧率 */
    engine.getFPS = function() {
        return this._fps;
    };
    /* 设置帧率 */
    engine.setFPS = function(fps) {
        this._fps = ('number' === typeof(fps) && fps > 24) ? fps : this._fps;
    };
    /* 获取一帧时间(毫秒) */
    engine.getFrameTime = function() {
        return 1000 / this._fps;
    };
    /* 主循环 */
    engine.loop = function(ctx, width, height) {
        if (undefined === ctx || null === ctx) {
            return;
        }
        ctx.clearRect(0, 0, width, height);
        for (var key in this._objectMap) {
            if (this._objectMap.hasOwnProperty(key)) {
                var obj = this._objectMap[key];
                if ('object' === typeof(obj) && 'function' === typeof(obj.loop)) {
                    obj.loop(ctx);
                }
            }
        }
    };
    /* 清空对象 */
    engine.removeAll = function() {
        this._objectMap = {};
    };
    /* 获取对象 */
    engine.getObj = function() {
        return this._objectMap[id];
    };
    /* 创建对象 */
    engine.createObj = function(id, visible) {
        var obj = {};
        obj._id = id;
        obj._visible = 'boolean' === typeof(visible) ? visible : true;
        obj._opacity = 1;
        obj._opacityDelta = 0;
        obj._fadeInCallback = null;
        obj._fadeOutCallback = null;
        obj._calculate = function() {
            /* 计算透明度 */
            this._opacity += this._opacityDelta;
            if (this._opacity < 0) {
                this._opacity = 0;
                this._opacityDelta = 0;
                if ('function' === typeof(this._fadeOutCallback)) {
                    this._fadeOutCallback();
                    this._fadeOutCallback = null;
                }
            } else if (this._opacity > 1) {
                this._opacity = 1;
                this._opacityDelta = 0;
                if ('function' === typeof(this._fadeInCallback)) {
                    this._fadeInCallback();
                    this._fadeInCallback = null;
                }
            }
        };
        /* 获取Id */
        obj.getId = function() {
            return this._id;
        };
        /* 是否可见 */
        obj.isVisible = function() {
            return this._visible;
        };
        /* 设置是否可见 */
        obj.setVisible = function(visible) {
            this._visible = visible;
        };
        /* 获取透明度 */
        obj.getOpacity = function() {
            return this._opacity;
        };
        /* 设置透明度 */
        obj.setOpacity = function(opacity) {
            this._opacity = opacity;
            this._opacityDelta = 0;
            this._fadeInCallback = null;
            this._fadeOutCallback = null;
        };
        /* 是否淡入中 */
        obj.isFadeIn = function() {
            return null !== this._fadeInCallback;
        }
        /* 是否淡出中 */
        obj.isFadeOut = function() {
            return null !== this._fadeOutCallback;
        }
        /* 淡入 */
        obj.fadeIn = function(duration, callback) {
            this._fadeInCallback = null;
            this._fadeOutCallback = null;
            if (duration > 0) {
                this._opacityDelta = (1 - this._opacity) / (duration / (1000 / engine._fps));
                if ('function' === typeof(callback)) {
                    this._fadeInCallback = callback;
                }
            } else {
                if ('function' === typeof(obj._calculate)) {
                    obj._calculate();
                }
                if (obj._visible) {
                    if ('function' === typeof(obj.onDraw)) {
                        obj.onDraw(ctx, obj._opacity);
                    }
                }
                this._opacity = 1;
                this._opacityDelta = 0;
                if ('function' === typeof(callback)) {
                    callback();
                }
            }
        };
        /* 淡出 */
        obj.fadeOut = function(duration, callback) {
            this._fadeInCallback = null;
            this._fadeOutCallback = null;
            if (duration > 0) {
                this._opacityDelta = -this._opacity / (duration / (1000 / engine._fps));
                if ('function' === typeof(callback)) {
                    this._fadeOutCallback = callback;
                }
            } else {
                this._opacity = 0;
                this._opacityDelta = 0;
                if ('function' === typeof(callback)) {
                    callback();
                }
            }
        };
        /* 销毁 */
        obj.destroy = function() {
            delete engine._objectMap[id];
            engine._objectMap[id] = undefined;
        };
        /* 循环 */
        obj.loop = function(ctx) {
            if ('function' === typeof(this._calculate)) {
                this._calculate();
            }
            if (this._visible) {
                if ('function' === typeof(this.onDraw)) {
                    this.onDraw(ctx, this._opacity);
                }
            }
        };
        /* 渲染函数(需要外部实现) */
        obj.onDraw = function(ctx, opacity){};
        this._objectMap[id] = obj;
        return obj;
    };
    /* 删除对象 */
    engine.removeObj = function(id) {
        delete this._objectMap[id];
        this._objectMap[id] = undefined;
    };
    return engine;
}

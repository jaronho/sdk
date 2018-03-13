/***********************************************************************
 ** Author:	jaron.ho
 ** Date:	2017-08-31
 ** Brief:  util functions
 ***********************************************************************/
.pragma library
Qt.include("Common.js")
Qt.include("Log.js")
Qt.include("Timer.js")
//----------------------------------------------------------------------
var log = new CreateLog(true, false, 1);
var mTimerManager = new CreateTimerManager();
var mTimerMap = {};
var mBlinkManager = new CreateTimerManager();
var mBlinkMap = {};
//----------------------------------------------------------------------
// 监听
function listen() {
    var nowTime = Date.now();
    mTimerManager.updateTimer(nowTime);
    mBlinkManager.updateTimer(nowTime);
}
//----------------------------------------------------------------------
// 创建定时器
function createTimer(interval, count, runCF, overCF, id) {
    if ('string' != typeof(id) || 0 === id.length) {
        id = Common.createUniqueId();
    }
    if (mTimerMap[id]) {
        mTimerMap[id].stop(false);
        delete mTimerMap[id];
    }
    var timer = mTimerManager.createTimer(interval, count, runCF, overCF, null, null);
    mTimerMap[id] = timer;
    timer.start(Date.now(), false);
    return id;
}
//----------------------------------------------------------------------
// 销毁定时器
function destroyTimer(id) {
    if ('string' != typeof(id) || 0 === id.length) {
        return;
    }
    if (mTimerMap[id]) {
        mTimerMap[id].stop(false);
        delete mTimerMap[id];
    }
}
//----------------------------------------------------------------------
// 延迟调用
function delayWith(time, callCF, id) {
    if (time <= 0) {
        if ('function' == typeof(callCF)) {
            callCF();
        }
        return;
    }
    return createTimer(time, 1, null, callCF, id);
}
//----------------------------------------------------------------------
// 开始闪烁
function startBlink(viewList, showTime, hideTime, count, showAtLast, id) {
    if (0 === viewList.length || showTime <= 0 || hideTime <= 0) {
        return;
    }
    if ('string' != typeof(id) || 0 === id.length) {
        id = Common.createUniqueId();
    }
    if (mBlinkMap[id]) {
        mBlinkMap[id].stop(false);
        delete mBlinkMap[id];
    }
    function setViewListShow(show) {
        for (var i = 0, len = viewList.length; i < len; ++i) {
            viewList[i].visible = show ? true : false;
        }
    }
    setViewListShow(true);
    var showFlag = true;
    var blink = mBlinkManager.createTimer(showTime, count * 2, function(tm, runCount) {
        showFlag = !showFlag;
        tm.setInterval(showFlag ? showTime : hideTime);
        setViewListShow(showFlag);
        var params = tm.getParam();
        if (params instanceof Array && "stop" === params[0] && showFlag === params[1]) {
            if (mBlinkMap[id]) {
                mBlinkMap[id].stop(false);
                delete mBlinkMap[id];
            }
        }
    }, function(tm) {
        setViewListShow(showAtLast);
    }, null, null);
    mBlinkMap[id] = blink;
    blink.start(Date.now(), false);
    return id;
}
//----------------------------------------------------------------------
// 停止闪烁
function stopBlink(viewList, immediately, showAtLast, id) {
    if (0 === viewList.length || 'string' !== typeof(id) || 0 === id.length) {
        return;
    }
    function setViewListShow(show) {
        for (var i = 0, len = viewList.length; i < len; ++i) {
            viewList[i].visible = show ? true : false;
        }
    }
    if (immediately) {
        if (mBlinkMap[id]) {
            mBlinkMap[id].stop(false);
            delete mBlinkMap[id];
        }
        setViewListShow(showAtLast);
    } else {
        if (mBlinkMap[id]) {
            mBlinkMap[id].setParam(["stop",showAtLast]);
        } else {
            setViewListShow(showAtLast);
        }
    }
}
//----------------------------------------------------------------------
// 创建对象,qmlFile:例"Sprite.qml"
function createObject(qmlFile, parent, callback) {
    if ('function' !== typeof(callback)) {
        callback = function(obj){};
    }
    if ('string' != typeof(qmlFile) || 0 === qmlFile.length) {
        console.warn("createObject => qmlFile is not string or empty");
        callback(null);
        return;
    }
    var com = Qt.createComponent(qmlFile);
    /*
        status:
            Component.Null - 0, no data is available for the component
            Component.Ready - 1, the component has been loaded, and can be used to create instances
            Component.Loading - 2, the component is currently being loaded
            Component.Error - 3, an error occurred while loading the component.
                                Calling errorString() will provide a human-readable description of any errors
     */
    function finishCreation() {
        if (0 === com.status) {
            console.warn("createObject => no data is available for the component \"" + qmlFile + "\"");
            callback(null);
        } else if (1 === com.status) {
            var obj = com.createObject(parent);
            if (null === obj) {
                console.warn("createObject => error creating object \"" + qmlFile + "\"");
            }
            callback(obj);
        } else if (3 === com.status) {
            console.warn("createObject => error loading component \"" + qmlFile + "\", " + com.errorString());
            callback(null);
        }
    }
    if (0 === com.status) {
        console.warn("createObject => no data is available for the component \"" + qmlFile + "\"");
        callback(null);
    } else if (1 === com.status) {
        finishCreation();
    } else if (3 === com.status) {
        console.warn("createObject => error loading component \"" + qmlFile + "\", " + com.errorString());
        callback(null);
    } else {
        com.statusChanged.connect(finishCreation);
    }
}
//----------------------------------------------------------------------
// 创建透明度动画
function createOpacityAnimation(object, from, to, duration, callback) {
    object.visible = true;
    object.opacity = from;
    var ani = Qt.createQmlObject('\
    import QtQuick 2.6;
    OpacityAnimator {
        property var callback: null;
        onStopped: function() {
            if ("function" === typeof(callback)) {
                callback();
            }
        }
    }', object);
    ani.target = object;
    ani.from = from;
    ani.to = to;
    ani.duration = duration;
    ani.callback = callback;
    return ani;
}
//----------------------------------------------------------------------
// 创建缩放动画
function createScaleAnimation(object, from, to, duration, callback) {
    object.scale = from;
    var ani = Qt.createQmlObject('\
    import QtQuick 2.6;
    ScaleAnimator {
        property var callback: null;
        onStopped: function() {
            if ("function" === typeof(callback)) {
                callback();
            }
        }
    }', object);
    ani.target = object;
    ani.from = from;
    ani.to = to;
    ani.duration = duration;
    ani.callback = callback;
    return ani;
}
//----------------------------------------------------------------------
// 创建旋转动画
function createRotationAnimation(object, from, to, duration, callback) {
    object.rotation = from;
    var ani = Qt.createQmlObject('\
    import QtQuick 2.6;
    RotationAnimator {
        property var callback: null;
        onStopped: function() {
            if ("function" === typeof(callback)) {
                callback();
            }
        }
    }', object);
    ani.target = object;
    ani.from = from;
    ani.to = to;
    ani.duration = duration;
    ani.callback = callback;
    return ani;
}
//----------------------------------------------------------------------
// 创建x位移动画
function createXOffsetAnimation(object, from, to, duration, callback) {
    object.x = from;
    var ani = Qt.createQmlObject('\
    import QtQuick 2.6;
    PropertyAnimation {
        property var callback: null;
        property: "x";
        onStopped: function() {
            if ("function" === typeof(callback)) {
                callback();
            }
        }
    }', object);
    ani.target = object;
    ani.from = from;
    ani.to = to;
    ani.duration = duration;
    ani.callback = callback;
    return ani;
}
//----------------------------------------------------------------------
// 创建y位移动画
function createYOffsetAnimation(object, from, to, duration, callback) {
    object.y = from;
    var ani = Qt.createQmlObject('\
    import QtQuick 2.6;
    PropertyAnimation {
        property var callback: null;
        property: "y";
        onStopped: function() {
            if ("function" === typeof(callback)) {
                callback();
            }
        }
    }', object);
    ani.target = object;
    ani.from = from;
    ani.to = to;
    ani.duration = duration;
    ani.callback = callback;
    return ani;
}
//----------------------------------------------------------------------
// 透明度
function opacityFromto(object, from, to, duration, callback) {
    var ani = createOpacityAnimation(object, from, to, duration, function() {
        ani.destroy();
        if ('function' === typeof(callback)) {
            callback();
        }
    });
    ani.start();
}
//----------------------------------------------------------------------
// 淡入
function fadeIn(object, duration, callback) {
    opacityFromto(object, 0, 1, duration, callback);
}
//----------------------------------------------------------------------
// 淡出
function fadeOut(object, duration, callback) {
    opacityFromto(object, 1, 0, duration, callback);
}
//----------------------------------------------------------------------
// 缩放
function scaleFromTo(object, from, to, duration, callback) {
    var ani = createScaleAnimation(object, from, to, duration, function() {
        ani.destroy();
        if ('function' === typeof(callback)) {
            callback();
        }
    });
    ani.start();
}
//----------------------------------------------------------------------
// 旋转
function rotateFromTo(object, from, to, duration, callback) {
    var ani = createRotationAnimation(object, from, to, duration, function() {
        ani.destroy();
        if ('function' === typeof(callback)) {
            callback();
        }
    });
    ani.start();
}
//----------------------------------------------------------------------
// 位移
function moveBy(object, x, y, duration, callback) {
    var xFlag = false;
    if ('number' === typeof(x) && 0 !== x) {
        xFlag = true;
        var aniX = createXOffsetAnimation(object, object.x, object.x + x, duration, function() {
            aniX.destroy();
            if ('function' === typeof(callback)) {
                callback();
            }
        })
        aniX.start();
    }
    if ('number' === typeof(y) && 0 !== y) {
        var aniY = createYOffsetAnimation(object, object.y, object.y + y, duration, function() {
            aniY.destroy();
            if (!xFlag && 'function' === typeof(callback)) {
                callback();
            }
        })
        aniY.start();
    }
}
//----------------------------------------------------------------------
// 位移x
function moveByX(object, x, duration, callback) {
    moveBy(object, x, null, duration, callback);
}
//----------------------------------------------------------------------
// 位移y
function moveByY(object, y, duration, callback) {
    moveBy(object, null, y, duration, callback);
}
//----------------------------------------------------------------------
// 移动
function moveFromTo(object, fromX, fromY, toX, toY, duration, callback) {
    var xFlag = false;
    if ('number' === typeof(fromX) || 'number' === typeof(toX)) {
        if ('number' !== typeof(fromX)) {
            fromX = object.x;
        }
        if ('number' !== typeof(toX)) {
            toX = object.x;
        }
        if (fromX !== toX) {
            xFlag = true;
            var aniX = createXOffsetAnimation(object, fromX, toX, duration, function() {
                aniX.destroy();
                if ('function' === typeof(callback)) {
                    callback();
                }
            })
            aniX.start();
        }
    }
    if ('number' === typeof(fromY) || 'number' === typeof(toY)) {
        if ('number' !== typeof(fromY)) {
            fromY = object.y;
        }
        if ('number' !== typeof(toY)) {
            toY = object.y;
        }
        if (fromY !== toY) {
            var aniY = createYOffsetAnimation(object, fromY, toY, duration, function() {
                aniY.destroy();
                if (!xFlag && 'function' === typeof(callback)) {
                    callback();
                }
            })
            aniY.start();
        }
    }
}
//----------------------------------------------------------------------
// 移动x
function moveFromToX(object, fromX, toX, duration, callback) {
    moveFromTo(object, fromX, null, toX, null, duration, callback);
}
//----------------------------------------------------------------------
// 移动y
function moveFromToY(object, fromY, toY, duration, callback) {
    moveFromTo(object, null, fromY, null, toY, duration, callback);
}
//----------------------------------------------------------------------

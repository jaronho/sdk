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
var mAnimationMap = {};
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
    var timer = mTimerManager.createTimer(interval, count, runCF, function(tm) {
        if (mTimerMap[id]) {
            delete mTimerMap[id];
        }
        if ('function' === typeof(overCF)) {
            overCF(tm);
        }
    }, null, null);
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
// 销毁所有定时器
function destroyAllTimers() {
    for (var key in mTimerMap) {
        if (mTimerMap.hasOwnProperty(key)) {
            mTimerMap[key].stop(false);
        }
    }
    mTimerMap = {};
}
//----------------------------------------------------------------------
// 延迟调用
function delayWith(time, callCF, id) {
    if (time <= 0) {
        if ('function' === typeof(callCF)) {
            callCF();
        }
        return;
    }
    return createTimer(time, 1, null, callCF, id);
}
//----------------------------------------------------------------------
// 开始闪烁
function startBlink(viewList, showTime, hideTime, count, showAtLast, overCF, id) {
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
        if (count > 0 && showFlag !== showAtLast) {
            delayWith(showFlag ? showTime : hideTime, function() {
                setViewListShow(showAtLast);
                if (mBlinkMap[id]) {
                    delete mBlinkMap[id];
                }
                if ('function' === typeof(overCF)) {
                    overCF();
                }
            }, id);
        } else {
            setViewListShow(showAtLast);
            if (mBlinkMap[id]) {
                delete mBlinkMap[id];
            }
            if ('function' === typeof(overCF)) {
                overCF();
            }
        }
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
            mBlinkMap[id].setParam(["stop", showAtLast]);
        } else {
            setViewListShow(showAtLast);
        }
    }
}
//----------------------------------------------------------------------
// 播放帧动画
function playFrameAnimation(image, frameList, duration, count, callback, id) {
    if (frameList.length <= 1 || duration <= 0) {
        return;
    }
    image.source = frameList[0];
    var index = 1;
    createTimer(duration, frameList.length * count - 1, function(tm, runCount) {
        image.source = frameList[index];
        if (index === frameList.length - 1) {
            index = 0;
        } else {
            ++index;
        }
    }, function(tm) {
        if ('function' === typeof(callback)) {
            callback();
        }
    }, id);
}
//----------------------------------------------------------------------
// 停止帧动画
function stopFrameAnimation(id) {
    destroyTimer(id);
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
            com.destroy();
            callback(null);
        } else if (1 === com.status) {
            var obj = com.createObject(parent);
            if (null === obj) {
                console.warn("createObject => error creating object \"" + qmlFile + "\"");
            }
            com.destroy();
            callback(obj);
        } else if (3 === com.status) {
            console.warn("createObject => error loading component \"" + qmlFile + "\", " + com.errorString());
            com.destroy();
            callback(null);
        } else {
            com.destroy();
            callback(null);
        }
    }
    if (0 === com.status) {
        console.warn("createObject => no data is available for the component \"" + qmlFile + "\"");
        com.destroy();
        callback(null);
    } else if (1 === com.status) {
        finishCreation();
    } else if (3 === com.status) {
        console.warn("createObject => error loading component \"" + qmlFile + "\", " + com.errorString());
        com.destroy();
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
    ani.callback = function() {
        object.opacity = to;
        if ('function' === typeof(callback)) {
            callback();
        }
    };
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
    ani.callback = function () {
        object.scale = to;
        if ('function' === typeof(callback)) {
            callback();
        }
    };
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
    ani.callback = function () {
        object.rotation = to;
        if ('function' === typeof(callback)) {
            callback();
        }
    };
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
    ani.callback = function () {
        object.x = to;
        if ('function' === typeof(callback)) {
            callback();
        }
    };
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
    ani.callback = function () {
        object.y = to;
        if ('function' === typeof(callback)) {
            callback();
        }
    };
    return ani;
}
//----------------------------------------------------------------------
// 完成动画
function completeAnimation(id) {
    if ('string' === typeof(id) && id.length > 0) {
        if (mAnimationMap[id]) {
            mAnimationMap[id].stop();
            delete mAnimationMap[id];
        } else {
            if (mAnimationMap[id + "_x"]) {
                mAnimationMap[id + "_x"].stop();
                delete mAnimationMap[id + "_x"];
            }
            if (mAnimationMap[id + "_y"]) {
                mAnimationMap[id + "_y"].stop();
                delete mAnimationMap[id + "_y"];
            }
        }
    }
}
//----------------------------------------------------------------------
// 动画是否在播放
function isAnimationPlay(id) {
    if ('string' === typeof(id) && id.length > 0 && (mAnimationMap[id] || mAnimationMap[id + "_x"] || mAnimationMap[id + "_y"])) {
        return true;
    }
    return false;
}
//----------------------------------------------------------------------
// 透明度
function opacityFromto(object, from, to, duration, callback, id) {
    if ('string' === typeof(id) && id.length > 0 && mAnimationMap[id]) {
        return;
    }
    if (from === to) {
        if ('function' === typeof(callback)) {
            callback();
        }
        return;
    }
    var ani = createOpacityAnimation(object, from, to, duration, function() {
        ani.destroy();
        if ('string' === typeof(id) && id.length > 0) {
            if (mAnimationMap[id]) {
                delete mAnimationMap[id];
                if ('function' === typeof(callback)) {
                    callback();
                }
            }
        } else if ('function' === typeof(callback)) {
            callback();
        }
    });
    if ('string' === typeof(id) && id.length > 0) {
        mAnimationMap[id] = ani;
    }
    ani.start();
}
//----------------------------------------------------------------------
// 淡入
function fadeIn(object, duration, callback, id) {
    if (object.visible && 1 === object.opacity) {
        if ('function' === typeof(callback)) {
            callback();
        }
        return;
    }
    opacityFromto(object, 0, 1, duration, callback, id);
}
//----------------------------------------------------------------------
// 淡出
function fadeOut(object, duration, callback, id) {
    if (!object.visible || 0 === object.opacity) {
        if ('function' === typeof(callback)) {
            callback();
        }
        return;
    }
    opacityFromto(object, 1, 0, duration, function() {
        object.visible = false;
        if ('function' === typeof(callback)) {
            callback();
        }
    }, id);
}
//----------------------------------------------------------------------
// 缩放
function scaleFromTo(object, from, to, duration, callback, id) {
    if ('string' === typeof(id) && id.length > 0 && mAnimationMap[id]) {
        return;
    }
    if (from === to) {
        if ('function' === typeof(callback)) {
            callback();
        }
        return;
    }
    var ani = createScaleAnimation(object, from, to, duration, function() {
        ani.destroy();
        if ('string' === typeof(id) && id.length > 0) {
            if  (mAnimationMap[id]) {
                delete mAnimationMap[id];
                if ('function' === typeof(callback)) {
                    callback();
                }
            }
        } else if ('function' === typeof(callback)) {
            callback();
        }
    });
    if ('string' === typeof(id) && id.length > 0) {
        mAnimationMap[id] = ani;
    }
    ani.start();
}
//----------------------------------------------------------------------
// 缩放到
function scaleTo(object, to, duration, callback, id) {
    scaleFromTo(object, object.scale, to, duration, callback, id);
}
//----------------------------------------------------------------------
// 旋转
function rotateFromTo(object, from, to, duration, callback, id) {
    if ('string' === typeof(id) && id.length > 0 && mAnimationMap[id]) {
        return;
    }
    if (from === to) {
        if ('function' === typeof(callback)) {
            callback();
        }
        return;
    }
    var ani = createRotationAnimation(object, from, to, duration, function() {
        ani.destroy();
        if ('string' === typeof(id) && id.length > 0) {
            if (mAnimationMap[id]) {
                delete mAnimationMap[id];
                if ('function' === typeof(callback)) {
                    callback();
                }
            }
        } else if ('function' === typeof(callback)) {
            callback();
        }
    });
    if ('string' === typeof(id) && id.length > 0) {
        mAnimationMap[id] = ani;
    }
    ani.start();
}
//----------------------------------------------------------------------
// 旋转到
function rotateTo(object, to, duration, callback, id) {
    rotateFromTo(object, object.rotation, to, duration, callback, id);
}
//----------------------------------------------------------------------
// 位移
function moveBy(object, x, y, duration, callback, id) {
    if (0 === x && 0 === y) {
        if ('function' === typeof(callback)) {
            callback();
        }
        return;
    }
    var xFlag = false;
    if ('number' === typeof(x) && 0 !== x) {
        if ('string' !== typeof(id) || id.length <= 0 || mAnimationMap[id + "_x"]) {
            xFlag = true;
            var aniX = createXOffsetAnimation(object, object.x, object.x + x, duration, function() {
                aniX.destroy();
                if ('string' === typeof(id) && id.length > 0) {
                    if (mAnimationMap[id + "_x"]) {
                        delete mAnimationMap[id + "_x"];
                        if ('function' === typeof(callback)) {
                            callback();
                        }
                    }
                } else if ('function' === typeof(callback)) {
                    callback();
                }
            })
            if ('string' === typeof(id) && id.length > 0) {
                mAnimationMap[id + "_x"] = aniX;
            }
            aniX.start();
        }
    }
    if ('number' === typeof(y) && 0 !== y) {
        if ('string' !== typeof(id) || id.length <= 0 || mAnimationMap[id + "_y"]) {
            var aniY = createYOffsetAnimation(object, object.y, object.y + y, duration, function() {
                aniY.destroy();
                if (xFlag) {
                    return;
                }
                if ('string' === typeof(id) && id.length > 0) {
                    if (mAnimationMap[id + "_y"]) {
                        delete mAnimationMap[id + "_y"];
                        if ('function' === typeof(callback)) {
                            callback();
                        }
                    }
                } else if ('function' === typeof(callback)) {
                    callback();
                }
            })
            if ('string' === typeof(id) && id.length > 0) {
                mAnimationMap[id + "_y"] = aniY;
            }
            aniY.start();
        }
    }
}
//----------------------------------------------------------------------
// 位移x
function moveByX(object, x, duration, callback, id) {
    moveBy(object, x, null, duration, callback, id);
}
//----------------------------------------------------------------------
// 位移y
function moveByY(object, y, duration, callback, id) {
    moveBy(object, null, y, duration, callback, id);
}
//----------------------------------------------------------------------
// 移动
function moveFromTo(object, fromX, fromY, toX, toY, duration, callback, id) {
    if (fromX === toX && fromY === toY) {
        if ('function' === typeof(callback)) {
            callback();
        }
        return;
    }
    var xFlag = false;
    if ('number' === typeof(fromX) || 'number' === typeof(toX)) {
        if ('number' !== typeof(fromX)) {
            fromX = object.x;
        }
        if ('number' !== typeof(toX)) {
            toX = object.x;
        }
        if (fromX !== toX) {
            if ('string' !== typeof(id) || id.length <= 0 || mAnimationMap[id + "_x"]) {
                xFlag = true;
                var aniX = createXOffsetAnimation(object, fromX, toX, duration, function() {
                    aniX.destroy();
                    if ('string' === typeof(id) && id.length > 0) {
                        if (mAnimationMap[id + "_x"]) {
                            delete mAnimationMap[id + "_x"];
                            if ('function' === typeof(callback)) {
                                callback();
                            }
                        }
                    } else if ('function' === typeof(callback)) {
                        callback();
                    }
                })
                if ('string' === typeof(id) && id.length > 0) {
                    mAnimationMap[id + "_x"] = aniX;
                }
                aniX.start();
            }
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
            if ('string' !== typeof(id) || id.length <= 0 || mAnimationMap[id + "_y"]) {
                var aniY = createYOffsetAnimation(object, fromY, toY, duration, function() {
                    aniY.destroy();
                    if (xFlag) {
                        return;
                    }
                    if ('string' === typeof(id) && id.length > 0) {
                        if (mAnimationMap[id + "_y"]) {
                            delete mAnimationMap[id + "_y"];
                            if ('function' === typeof(callback)) {
                                callback();
                            }
                        }
                    } else if ('function' === typeof(callback)) {
                        callback();
                    }
                })
                if ('string' === typeof(id) && id.length > 0) {
                    mAnimationMap[id + "_y"] = aniY;
                }
                aniY.start();
            }
        }
    }
}
//----------------------------------------------------------------------
// 移动x
function moveFromToX(object, fromX, toX, duration, callback, id) {
    moveFromTo(object, fromX, null, toX, null, duration, callback, id);
}
//----------------------------------------------------------------------
// 移动y
function moveFromToY(object, fromY, toY, duration, callback, id) {
    moveFromTo(object, null, fromY, null, toY, duration, callback, id);
}
//----------------------------------------------------------------------
// 移动到
function moveTo(object, toX, toY, duration, callback, id) {
    moveFromTo(object, object.x, object.y, toX, toY, duration, callback, id);
}
//----------------------------------------------------------------------
// 移动x到
function moveToX(object, toX, duration, callback, id) {
    moveFromTo(object, object.x, null, toX, null, duration, callback, id);
}
//----------------------------------------------------------------------
// 移动y到
function moveToY(object, toY, duration, callback, id) {
    moveFromTo(object, null, object.y, null, toY, duration, callback, id);
}
//----------------------------------------------------------------------

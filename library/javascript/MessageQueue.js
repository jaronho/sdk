/***********************************************************************
** Author:  jaron.ho
** Date:    2019-03-28
** Brief:   消息管理器
**          消息弹出算法:
**          1.优先级高的队列先取,直到队列完全取完,再取优先级低的消息队列
**          2.同等优先级,类型大的队列先取,取完一条消息(该消息次数减一,当次数为0时,从队列中移除),
**          取下一个类型,直到所有类型都取完一个消息后,重新从类型大的队列中取,此次取值从上一轮所取的消息接着往后取
**          PS:优先级值越大,级别越高;同理,类型值越大,级别越高;例:5>4>3>2>1
***********************************************************************/
function createMessageQueue() {
    var mMsgTable = [];			    /* 消息表(二维数组),先根据优先级存放,再根据类型存放 */
    var mHistoryMsgSize = 30;       /* 历史消息表大小 */
    var mHistoryMsgTable = [];      /* 历史消息表(一维数组) */
    var mLastPriority = 0;          /* 上一条弹出消息的优先级 */
    var mLastMsgType = 0;           /* 上一条弹出消息的类型 */
    var mLastIndexTable = {};       /* 上一条弹出消息的索引值 */
    var message = {};               /* 消息对象 */
    /* 构造一个消息结构体 */
    function createMsg(priority, msgtype, times, data) {
        var msg = {};
        msg.priority = priority;    /* 优先级 */
        msg.msgtype = msgtype;      /* 类型 */
        msg.times = times;          /* 播放次数 */
        msg.data = data;            /* 消息数据,任意结构体 */
        return msg;
    }
    /* 获取指定优先级的表索引值 */
    function getPriorityIndex(msgTable, priority) {
        function innerFunc() {
            for (var i = 0; i < msgTable.length; ++i) {
                if (priority === msgTable[i].priority) {
                    return i;
                }
            }
            var priorityInfo = {};
            priorityInfo.priority = priority;       /* 消息优先级 */
            priorityInfo.typelist = [];             /* 消息类型列表 */
            msgTable.push(priorityInfo);
            msgTable.sort(function(a, b) {
                return a.priority < b.priority ? 1 : -1;
            });
            return innerFunc();
        }
        return innerFunc();
    }
    /* 获取指定类型的表索引值 */
    function getTypeIndex(msgTable, priority, msgtype) {
        function innerFunc() {
            var priorityIndex = getPriorityIndex(msgTable, priority);
            for (var i = 0; i < msgTable[priorityIndex].typelist.length; ++i) {
                if (msgtype === msgTable[priorityIndex].typelist[i].msgtype) {
                    return [priorityIndex, i];
                }
            }
            var typeInfo = {};
            typeInfo.msgtype = msgtype;         /* 消息类型 */
            typeInfo.msglist = [];              /* 消息列表 */
            msgTable[priorityIndex].typelist.push(typeInfo);
            msgTable[priorityIndex].typelist.sort(function(a, b) {
                return a.msgtype < b.msgtype ? 1 : -1;
            });
            return innerFunc();
        }
        return innerFunc();
    }
    /* 获取要弹出的条件:优先级,类型,索引值 */
    function getPopCondition(msgTable, lastPriority, lastMsgType, lastIndexTable) {
        for (var i = 0; i < msgTable.length; ++i) {
            var val = msgTable[i];
            var typeInfoList = [];
            for (var j = 0; j < msgTable[i].typelist.length; ++j) {
                if (msgTable[i].typelist[j].msglist.length > 0) {
                    typeInfoList.push(msgTable[i].typelist[j]);
                }
            }
            if (typeInfoList.length > 0) {
                if (lastPriority !== msgTable[i].priority) {
                    lastMsgType = 0;
                    lastIndexTable = {};
                }
                var typeInfo = typeInfoList[0];
                for (var k = 0; k < typeInfoList.length; ++k) {
                    if (typeInfoList[k].msgtype < lastMsgType) {
                        typeInfo = typeInfoList[k];
                        break;
                    }
                }
                var lastIndex = lastIndexTable[typeInfo.msgtype];
                lastIndex = 'number' === typeof(lastIndex) ? lastIndex : 0;
                var msgIndex = 0;
                for (var n = 0; n < typeInfo.msglist.length; ++n) {
                    if (n > lastIndex) {
                        msgIndex = n;
                        break;
                    }
                }
                return [msgTable[i].priority, typeInfo.msgtype, msgIndex];
            }
        }
        return null;
    }
    /* 保存到历史消息队列 */
    function saveToHistory(msg) {
        var removeCount = mHistoryMsgTable.length - mHistoryMsgSize + 1;
        for (var i = 0; i < removeCount; ++i) {
            mHistoryMsgTable.splice(0, 1);
        }
        mHistoryMsgTable.push(msg);
    }
    /* 初始化消息管理器 */
    message.init = function(historyMsgSize) {
        mMsgTable = [];
        mHistoryMsgSize = historyMsgSize;
        mHistoryMsgTable = [];
        mLastPriority = 0;
        mLastMsgType = 0;
        mLastIndexTable = {};
    };
    /* 是否存在消息 */
    message.exist = function() {
        for (var i = 0; i < mMsgTable.length; ++i) {
            for (var j = 0; j < mMsgTable[i].typelist.length; ++j) {
                if (mMsgTable[i].typelist[j].msglist.length > 0) {
                    return true;
                }
            }
        }
        return false;
    };
    /* 插入一条消息 */
    message.insert = function(priority, msgtype, times, data, limitCount) {
        if (priority <= 0 || msgtype <= 0 || times <= 0) {
            throw new Error("message -> insert -> priority, msgtype, times must > 0");
        }
        limitCount = 'number' === typeof(limitCount) ? limitCount : 0;
        var indexs = getTypeIndex(mMsgTable, priority, msgtype);
        var priorityIndex = indexs[0];
        var typeIndex = indexs[1];
        if (limitCount > 0) {			/* 限制队列容量 */
            var count = mMsgTable[priorityIndex].typelist[typeIndex].msglist.length;
            var removeCount = count - limitCount + 1;
            for (var i = 0; i < removeCount; ++i) {
                mMsgTable[priorityIndex].typelist[typeIndex].msglist.splice(0, 1);
            }
        }
        mMsgTable[priorityIndex].typelist[typeIndex].msglist.push(createMsg(priority, msgtype, times, data));
    };
    /* 弹出一条消息 */
    message.pop = function() {
        var conds = getPopCondition(mMsgTable, mLastPriority, mLastMsgType, mLastIndexTable);
        if (undefined === msg || null === conds) {
            return null;
        }
        var popPriority = conds[0];
        var popType = conds[1];
        var msgIndex = conds[2];
        var indexs = getTypeIndex(mMsgTable, popPriority, popType);
        var priorityIndex = indexs[0];
        var typeIndex = indexs[1];
        var msg = mMsgTable[priorityIndex].typelist[typeIndex].msglist[msgIndex];
        if (undefined === msg || null === msg) {
            return null;
        }
        if (popPriority !== mLastPriority) {
            mLastIndexTable = {};
        }
        mLastPriority = popPriority;
        mLastMsgType = popType;
        msg.times = msg.times - 1;
        if (msg.times <= 0) {
            mLastIndexTable[popType] = msgIndex - 1;
            mMsgTable[priorityIndex].typelist[typeIndex].msglist.splice(msgIndex, 1);
        } else {
            mLastIndexTable[popType] = msgIndex;
            mMsgTable[priorityIndex].typelist[typeIndex].msglist[msgIndex] = msg;
        }
        var retMsg = createMsg(msg.priority, msg.msgtype, msg.times, msg.data);
        saveToHistory(retMsg);
        return retMsg;
    };
    /* 获取历史消息队列 */
    message.getHistory = function() {
        return mHistoryMsgTable;
    };
    return message;
}
//----------------------------------------------------------------------
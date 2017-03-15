----------------------------------------------------------------------
-- Author:	jaron.ho
-- Date:	2014-09-19
-- Brief:	消息管理器(系统公告,普通公告,聊天消息)
--			消息弹出算法:
--			1.优先级高的队列先取,直到队列完全取完,再取优先级低的消息队列
--			2.同等优先级,类型大的队列先取,取完一条消息(该消息次数减一,当次数为0时,从队列中移除),
--			取下一个类型,直到所有类型都取完一个消息后,重新从类型大的队列中取,此次取值从上一轮所取的消息接着往后取
--			PS:优先级值越大,级别越高;同理,类型值越大,级别越高;例:5>4>3>2>1
----------------------------------------------------------------------
function CreateMessage()
	local mMsgTable = {}			-- 消息表(二维数组),先根据优先级存放,再根据类型存放
	local mHistoryMsgSize = 30		-- 历史消息表大小
	local mHistoryMsgTable = {}		-- 历史消息表(一维数组)
	local mLastPriority = 0			-- 上一条弹出消息的优先级
	local mLastMsgType = 0			-- 上一条弹出消息的类型
	local mLastIndexTable = {}		-- 上一条弹出消息的索引值
	local message = {}				-- 消息对象
	-- 构造一个消息结构体
	local function createMsg(priority, msgtype, times, data)
		local msg = {}
		msg.priority = priority			-- 优先级
		msg.msgtype = msgtype			-- 类型
		msg.times = times				-- 播放次数
		msg.data = data					-- 消息数据,任意结构体
		return msg
	end
	-- 获取指定优先级的表索引值
	local function getPriorityIndex(msgTable, priority)
		local function innerFunc()
			for key, val in pairs(msgTable) do
				if priority == val.priority then
					return key
				end
			end
			local priorityInfo = {}
			priorityInfo.priority = priority		-- 消息优先级
			priorityInfo.typelist = {}				-- 消息类型列表
			table.insert(msgTable, priorityInfo)
			table.sort(msgTable, function(a, b) return a.priority > b.priority end)
			return innerFunc()
		end
		return innerFunc()
	end
	-- 获取指定类型的表索引值
	local function getTypeIndex(msgTable, priority, msgtype)
		local function innerFunc()
			local priorityIndex = getPriorityIndex(msgTable, priority)
			for key, val in pairs(msgTable[priorityIndex].typelist) do
				if msgtype == val.msgtype then
					return priorityIndex, key
				end
			end
			local typeInfo = {}
			typeInfo.msgtype = msgtype			-- 消息类型
			typeInfo.msglist = {}				-- 消息列表
			table.insert(msgTable[priorityIndex].typelist, typeInfo)
			table.sort(msgTable[priorityIndex].typelist, function(a, b) return a.msgtype > b.msgtype end)
			return innerFunc()
		end
		return innerFunc()
	end
	-- 获取要弹出的条件:优先级,类型,索引值
	local function getPopCondition(msgTable, lastPriority, lastMsgType, lastIndexTable)
		for key, val in pairs(msgTable) do
			local typeInfoList = {}
			for k, v in pairs(val.typelist) do
				if #(v.msglist) > 0 then
					table.insert(typeInfoList, v)
				end
			end
			if #typeInfoList > 0 then
				if lastPriority ~= val.priority then
					lastMsgType = 0
					lastIndexTable = {}
				end
				local typeInfo = typeInfoList[1]
				for i=1, #typeInfoList do
					if typeInfoList[i].msgtype < lastMsgType then
						typeInfo = typeInfoList[i]
						break
					end
				end
				local msgIndex = 1
				for j=1, #(typeInfo.msglist) do
					if j > (lastIndexTable[typeInfo.msgtype] or 0) then
						msgIndex = j
						break
					end
				end
				return val.priority, typeInfo.msgtype, msgIndex
			end
		end
	end
	-- 保存到历史消息队列
	local function saveToHistory(msg)
		local removeCount = #mHistoryMsgTable - mHistoryMsgSize + 1
		for i=1, removeCount do
			table.remove(mHistoryMsgTable, 1)
		end
		table.insert(mHistoryMsgTable, msg)
	end
	-- 初始化消息管理器
	function message:init(historyMsgSize)
		mMsgTable = {}
		mHistoryMsgSize = historyMsgSize
		mHistoryMsgTable = {}
		mLastPriority = 0
		mLastMsgType = 0
		mLastIndexTable = {}
	end
	-- 是否存在消息
	function message:exist()
		for key, val in pairs(mMsgTable) do
			for k, v in pairs(val.typelist) do
				if #(v.msglist) > 0 then
					return true
				end
			end
		end
		return false
	end
	-- 插入一条消息
	function message:insert(priority, msgtype, times, data, limitCount)
		assert(priority > 0 and msgtype > 0 and times > 0, "message -> insert -> priority, msgtype, times must > 0")
		local priorityIndex, typeIndex = getTypeIndex(mMsgTable, priority, msgtype)
		limitCount = limitCount or 0
		if limitCount > 0 then			-- 限制队列容量
			local count = #(mMsgTable[priorityIndex].typelist[typeIndex].msglist)
			local removeCount = count - limitCount + 1
			for i=1, removeCount do
				table.remove(mMsgTable[priorityIndex].typelist[typeIndex].msglist, 1)
			end
		end
		table.insert(mMsgTable[priorityIndex].typelist[typeIndex].msglist, createMsg(priority, msgtype, times, data))
	end
	-- 弹出一条消息
	function message:pop()
		local popPriority, popType, msgIndex = getPopCondition(mMsgTable, mLastPriority, mLastMsgType, mLastIndexTable)
		if nil == popPriority or nil == popType or nil == msgIndex then
			return
		end
		local priorityIndex, typeIndex = getTypeIndex(mMsgTable, popPriority, popType)
		local msg = mMsgTable[priorityIndex].typelist[typeIndex].msglist[msgIndex]
		if nil == msg then
			return
		end
		if popPriority ~= mLastPriority then
			mLastIndexTable = {}
		end
		mLastPriority = popPriority
		mLastMsgType = popType
		msg.times = msg.times - 1
		if msg.times <= 0 then
			mLastIndexTable[popType] = msgIndex - 1
			table.remove(mMsgTable[priorityIndex].typelist[typeIndex].msglist, msgIndex)
		else
			mLastIndexTable[popType] = msgIndex
			mMsgTable[priorityIndex].typelist[typeIndex].msglist[msgIndex] = msg
		end
		local retMsg = createMsg(msg.priority, msg.msgtype, msg.times, msg.data)
		saveToHistory(retMsg)
		return retMsg
	end
	-- 获取历史消息队列
	function message:getHistory()
		return mHistoryMsgTable
	end
	return message
end
----------------------------------------------------------------------
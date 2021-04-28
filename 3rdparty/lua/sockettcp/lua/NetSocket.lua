----------------------------------------------------------------------
-- 作者：hezhr
-- 日期：2013-11-27
-- 描述：网络对接模块
----------------------------------------------------------------------
require "NetEnumDef"
require "NetMsgType"
require "NetPacket"
----------------------------------------------------------------------
local HEAD_SIZE = 4						-- 包头大小,固定4个字节
local mClient = nil						-- 客户端网络对象
local mGetHead = true					-- 当前是否取包头
local mReceiveLen = HEAD_SIZE			-- 当前接收长度
local mHasReceived = ""					-- 已接收的数据
local mClosedCallFunc = nil				-- 断开回调函数
local mNetMsgHandlers = {}				-- 网络消息处理句柄
local mPacketCrypto = false				-- 发送给服务端的消息是否加密
----------------------------------------------------------------------
-- 加密数据包
local function encodePacketCrypto(packet)
	if true == mPacketCrypto then
		packet = packet_crypto_encode(packet, string.len(packet))
	end
	return packet
end
----------------------------------------------------------------------
-- 功  能：连接服务端
-- 参  数：host(string)-服务器ip地址;port(number)-端口号;connectTimeout(number)-连接超时;closedCallFunc(function)-断开回调;crypto(bool)-加密
-- 返回值：boolean
----------------------------------------------------------------------
function NetSocket_connect(host, port, connectTimeout, closedCallFunc, crypto)
	if mClient then
		print("connect already, please close before ...")
		return false
	end
	mClient, err = require("socket").connect(host, port, nil, nil, connectTimeout)
	if nil == mClient then
		print("connect to host='"..host.."', port="..port..", crypto="..tostring(crypto).." failed ["..(err or "").."] ...")
		return false
	end
	print("connect to host='"..host.."', port="..port..", crypto="..tostring(crypto).." success ...")
	mGetHead = true
	mReceiveLen = HEAD_SIZE
	mHasReceived = ""
	mClosedCallFunc = closedCallFunc
	mPacketCrypto = crypto
	return true
end
----------------------------------------------------------------------
-- 功  能：关闭连接
-- 参  数：无
-- 返回值：无返回值
----------------------------------------------------------------------
function NetSocket_close()
	if mClient then
		mClient:shutdown("both")
		mClient:close()
		mClient = nil
		print("close connect ...")
	end
end
----------------------------------------------------------------------
-- 功  能：发送消息
-- 参  数：msgStruct(table类型)-发送的网络消息结构
-- 返回值：boolean
----------------------------------------------------------------------
function NetSocket_send(msgStruct, serverStep, clientStep)
	if nil == mClient then
		return false
	end
	-- 数据包字节流
	local byteArray = ByteArray()
	byteArray = msgStruct.build(byteArray)
	-- 包体(服务端步进值+客户端步进值+数据包字节流)
	local body = string.pack("b", serverStep)..string.pack("b", clientStep)..byteArray.getBytes()
	-- 加密
	body = encodePacketCrypto(body)
	-- 包头(大小端转换后的包体长度)
	local bodyLength = string.len(body)							-- 包体长度
	local head = string.pack("i", string.swab32(bodyLength))	-- 包头四个字节,这里要用"i"
	-- 发送
	local allLen, err, sentLen = mClient:send(head..body)
	local msgId = msgStruct.getMsgID()
	local msgName = NetSocket_getMsgName(msgId)
	if nil == allLen or "closed" == err or "timeout" == err then
		print("----- send ["..msgName..", "..msgId.."], step [server="..serverStep..", client="..clientStep.."], failed ["..(err or "").."] ...")
		return false
	end
	print("----- send ["..msgName..", "..msgId.."], step [server="..serverStep..", client="..clientStep.."]")
	return true
end
----------------------------------------------------------------------
-- 功  能：监听消息
-- 参  数：无
-- 返回值：无返回值
----------------------------------------------------------------------
function NetSocket_update()
	if nil == mClient then
		return
	end
	local content, status, recevied = mClient:receive(mReceiveLen)
	if "closed" == status then
		NetSocket_close()
		if "function" == type(mClosedCallFunc) then
			mClosedCallFunc()
		end
		return
	end
	-- 处理网络分包
	if nil ~= recevied then
		mHasReceived = mHasReceived..recevied
		if string.len(mHasReceived) == mReceiveLen then
			content = mHasReceived
		end
	end
	-- 没有收到数据
	if nil == content then
		return
	end
	-- 收到服务端发送过来的数据
	if true == mGetHead then	-- 当前取出来的是包头
		mGetHead = false		-- 设置下一步取包体
		mReceiveLen = string.swab32_array(content)	-- 解析包体长度
		print("----- recv head [bodysize="..mReceiveLen.."]")
	else						-- 当前取出来的是包体
		mGetHead = true			-- 设置下一步取包头
		mReceiveLen = HEAD_SIZE
		mHasReceived = ""
		-- 解密,由于服务端发给客户端是不加密的,所以下面代码需要注释掉
		-- content = packet_crypto_decode(content, string.len(content))
		-- 解析数据包
		local byteArray = ByteArray()
		byteArray.setBytes(content)
		-- 读取服务端步进值
		local serverStep = byteArray.read_uchar()
		-- 读取客户端步进值
		local clientStep = byteArray.read_uchar()
		-- 读取消息id
		local msgId = byteArray.read_uint16()
		print("----- recv body ["..NetSocket_getMsgName(msgId)..", "..msgId.."], step [server="..serverStep..", client="..clientStep.."]")
		-- 网络事件派发
		NetSocket_post(msgId, byteArray, serverStep, clientStep)
	end
end
----------------------------------------------------------------------
-- 功  能：注册网络消息事件
-- 参  数：msgId(number类型)-网络消息id;msgBuilder(function类型)-网络消息构造器;msgHandler(function类型)-网络消息处理句柄
-- 返回值：无返回值
----------------------------------------------------------------------
function NetSocket_registerHandler(msgId, msgBuilder, msgHandler)
	if "number" ~= type(msgId) or "function" ~= type(msgHandler) then
		assert(false, "NetSocket -> registerHandler -> msgId is not number or msgHandler is not function")
		return
	end
	for k, v in pairs(mNetMsgHandlers) do
		if msgId == v.msgId then	-- 网络事件已注册
			assert(false, "NetSocket -> registerHandler -> msg id ["..msgId.."] is exist")
			return
		end
	end
	-- 网络事件未注册
	local event = {}		-- 构造一个网络事件
	event.msgId = msgId				-- 消息id
	event.msgBuilder = msgBuilder	-- 消息构造器
	event.msgHandler = msgHandler	-- 消息处理句柄
	table.insert(mNetMsgHandlers, event)
end
----------------------------------------------------------------------
-- 功  能：取消网络消息事件注册
-- 参  数：msgId(number类型)-网络消息id
-- 返回值：无返回值
----------------------------------------------------------------------
function NetSocket_unregisterHandler(msgId)
	for k, v in pairs(mNetMsgHandlers) do
		if msgId == v.msgId then	-- 事件已注册
			table.remove(mNetMsgHandlers, k)
			return
		end
	end
end
----------------------------------------------------------------------
-- 功  能：派发网络消息事件
-- 参  数：msgId(number类型)-网络消息id;byteArray-数据包字节流;serverStep-服务端步进值;clientStep-客户端步进值
-- 返回值：无返回值
----------------------------------------------------------------------
function NetSocket_post(msgId, byteArray, serverStep, clientStep)
	-- step1:广播消息事件,0指定为该事件的id
	local broadcastEvent = NetSocket_getEvent(0)
	if "table" == type(broadcastEvent) and "function" == type(broadcastEvent.msgHandler) then
		broadcastEvent.msgHandler(msgId, serverStep, clientStep)
	end
	-- step2:匹配消息事件
	local matchingEvent = NetSocket_getEvent(msgId)
	if "table" == type(matchingEvent) then
		local msgStruct = nil
		if "function" == type(matchingEvent.msgBuilder) then
			msgStruct = matchingEvent.msgBuilder()
			msgStruct.decode(byteArray)
		end
		if "function" == type(matchingEvent.msgHandler) then
			matchingEvent.msgHandler(msgStruct)
		end
	end
end
----------------------------------------------------------------------
-- 功  能：获取网络消息事件
-- 参  数：msgId(number类型)-网络消息id
-- 返回值：网络消息事件(table类型)
----------------------------------------------------------------------
function NetSocket_getEvent(msgId)
	for k, v in pairs(mNetMsgHandlers) do
		if msgId == v.msgId then	-- 事件已注册
			return v
		end
	end
	return nil
end
----------------------------------------------------------------------
-- 功  能：获取消息名字
-- 参  数：msgId(number)-网络消息id
-- 返回值：网络消息名
----------------------------------------------------------------------
function NetSocket_getMsgName(msgId)
	for k, v in pairs(NetMsgType) do
		if msgId == v then
			return k
		end
	end
	return ""
end
----------------------------------------------------------------------


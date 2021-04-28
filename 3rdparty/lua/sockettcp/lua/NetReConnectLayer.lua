-- 断线重连界面
local mLayerRoot = nil 
local mLayerOrder = 1991 
local mPriority = -2147483648
local mConnectFlag = false
NetReConnectLayer = {}

local function onTouch(eventType, x, y)
	if "began" == eventType then 
		return true
	end
end

-- 点击重新连接按钮
local function onClickReconnectButton(typeName, widget)
	if "releaseUp" == typeName then
		widget:setTouchEnabled(false)
		NetReConnectLayer.hide()
		NetSendLoadLayer.show()
		local function delayDone()
			mConnectFlag = true
			local data = chooseServerDataCache.getServerData()
			NetHelper.connect(data.ip, data.port, data.crypto, 0)
		end
		local delayTimer = CreateTimer(0.07, 1, nil, delayDone)
		delayTimer.start()
	end
end

-- 点击退出游戏按钮
local function onClickExitgameButton(typeName, widget)
	if "releaseUp" == typeName then
		widget:setTouchEnabled(false)
		CCDirector:sharedDirector():endToLua()
	end
end

-- 0.显示重新连接按钮,退出游戏按钮;1.显示重新连接按钮;2.显示退出游戏按钮
local function initView(viewType, tipString)
	if mLayerRoot then
		return
	end
	mLayerRoot = UILayer:create()
	mLayerRoot:setTouchEnabled(true)
	mLayerRoot:registerScriptTouchHandler(onTouch, false, mPriority, true)
	--
	local jsonRoot = GUIReader:shareReader():widgetFromJsonFile("reconnect.json")
	-- 重新连接按钮
	local reconnectBtn = jsonRoot:getChildByName("Button_reconnect")
	reconnectBtn:registerEventScript(onClickReconnectButton)	
	-- 退出游戏按钮
	local exitgameBtn = jsonRoot:getChildByName("Button_exitgame")
	exitgameBtn:registerEventScript(onClickExitgameButton)
	-- 提示文本
	local tipLabel = tolua.cast(jsonRoot:getChildByName("Label_tip"), "UILabel")
	tipLabel:ignoreContentAdaptWithSize(true)
	tipLabel:setText(tipString)
	--
	if 0 == viewType then
		reconnectBtn:setEnabled(true)
		exitgameBtn:setEnabled(true)
	elseif 1 == viewType then
		reconnectBtn:setEnabled(true)
		setWidget_Horizontal_Center(reconnectBtn)
		exitgameBtn:setEnabled(false)
	elseif 2 == viewType then
		reconnectBtn:setEnabled(false)
		exitgameBtn:setEnabled(true)
		setWidget_Horizontal_Center(exitgameBtn)
	end
	FightAnimation_openX(jsonRoot:getChildByName("panel"))
	--
	mLayerRoot:addWidget(jsonRoot)
	g_rootNode:addChild(mLayerRoot, mLayerOrder)
end

-- 显示断线提示:0.重连界面;1.飘字提示;2.退出界面
NetReConnectLayer.show = function(showType, showStr)
	if nil == showStr then
		return
	end
	NetSendLoadLayer.dismiss()
	if 1 == showType then
		Toast.show(showStr)
	elseif 2 == showType then
		initView(2, showStr)
	else	-- 0 == showType
		initView(1, showStr)
	end
end

-- 隐藏断线提示
NetReConnectLayer.hide = function()
	if mLayerRoot then
		mLayerRoot:unregisterScriptTouchHandler()
		mLayerRoot:removeFromParentAndCleanup(true)
		mLayerRoot = nil
	end
end

-- 处理版本验证
local function handleCheckVersion(success)
	if false == mConnectFlag then
		return
	end
	mConnectFlag = false
	if true == success then
		if 0 == ModelPlayer.getId() then
			NetHelper.loginCheck(0)
		else
			NetHelper.sendReconnect()
		end
	end
end

EventCenter_subscribe(EventDef["ED_CHECK_VERSION"], handleCheckVersion)

NetSendLoadLayer = {}

local mSendLayer = nil 
local LAYER_TAG = 200
local mIsWaitting = false

local function onTouchBegan(x, y)
	return true
end

local function onTouchMoved(x, y)
end

local function onTouchEnded(x, y)
end

local function onTouch(eventType, x, y)
	if "began" == eventType then
		return onTouchBegan(x, y)
	elseif "moved" == eventType then
		return onTouchMoved(x, y)
	else
		return onTouchEnded(x, y)	
	end	
end

local function loadingUpdateCB(sender)
	local rotate = sender:getRotation()
	rotate = rotate + 36
	if rotate > 360 then
		rotate = rotate - 360
	end
	sender:setRotation(rotate)
end

local function createLoadingLayer()
	local layer = CCLayer:create()
	local sz = CCDirector:sharedDirector():getVisibleSize()
	local sprite = CCSprite:create("loading.png")
	sprite:setPosition(ccp(sz.width/2, sz.height/2))
	local rotate = CCRotateBy:create(0.5, 360)
	
	local arr = CCArray:create()
	arr:addObject(CCDelayTime:create(0.03))
	arr:addObject(CCCallFuncN:create(loadingUpdateCB))
	sprite:runAction(CCRepeatForever:create(CCSequence:create(arr)))
	
	--
	layer:setTag(LAYER_TAG)
	layer:addChild(sprite)
	layer:setTouchEnabled(true)
	layer:registerScriptTouchHandler(onTouch, false, -2147483647 - 1, true)
	return layer
end 

NetSendLoadLayer.show = function()
	mIsWaitting = true
	if nil == mSendLayer then
		mSendLayer = createLoadingLayer()
		g_rootNode:addChild(mSendLayer, 1992)
		return
	end
	mSendLayer:setVisible(true)
	mSendLayer:registerScriptTouchHandler(onTouch, false, -2147483647 - 1, true)
	g_sceneRoot:setTouchEnabled(false)
end

NetSendLoadLayer.dismiss = function()
	if mSendLayer and true == mSendLayer:isVisible() then 	
		mSendLayer:unregisterScriptTouchHandler()
		mSendLayer:setVisible(false)
		g_sceneRoot:setTouchEnabled(false)
		mIsWaitting = false
	end
end

NetSendLoadLayer.isWaitMessage = function()
	return mIsWaitting
end



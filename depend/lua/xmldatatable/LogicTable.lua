----------------------------------------------------------------------
-- 作者：hezhr
-- 日期：2013-11-21
-- 描述：游戏逻辑数据表
----------------------------------------------------------------------
require "XmlTable"

-- 各数据表
local mRandomNameTable = XmlTable_load("random_name_tplt.xml", "id")					-- 随机姓字库
local mRandomSecondNameTable = XmlTable_load("random_secondname_tplt.xml", "id")		-- 随机名字库
local mGame_discrete_date = XmlTable_load("game_discrete_date.xml", "gamedate")
local mTaskTable = XmlTable_load("task_tplt.xml", "id")								-- 任务数据表
local mGiftBagTable = XmlTable_load("gift_bag_tplt.xml", "id")						-- 礼包表
local mDailyAwardTable = XmlTable_load("daily_award_tplt.xml", "level_range")			-- 每日奖励表

local mError_Code = XmlTable_load("error_code_tplt.xml", "id") 
local mItem_Pack = XmlTable_load("item_tplt.xml", "id")
local mEquipment = XmlTable_load("equipment_tplt.xml", "id")

local mExtendPack = XmlTable_load("extend_pack_price.xml", "the_time") --拓展背包价格表

-- 商城物品
local mTbProductsInfo = XmlTable_load("mall_item.xml", "id")
-- 商城公告xml 
local mTbPostInfo = XmlTable_load("mall_post.xml", "id")
-- (商城)分页按钮 
local mtbRadioBtnInfo = XmlTable_load("mall_radio.xml", "id")
-- 积分商城物品
local mTbGameRankProductsInfo = XmlTable_load("point_mall_tplt.xml", "id")
-- 音频
local mTbAudioInfo = XmlTable_load("music_tplt.xml", "id")

LogicTable = {
}


---  背包转到各种 类型 的中转表（通过物品id，获得物品信息）
LogicTable.getItemById = function(id)
	local res = XmlTable_getRow(mItem_Pack, id, true)
	local row = {}

	row.id = res.id
	row.type = res.type + 0	-- 1装备, 2符文, 3宝石, 4其他
	row.name = res.name
	row.overlay_count = res.overlay_count
	row.sell_price = res.sell_price
	row.sub_id = res.sub_id
	row.icon = res.icon
	row.describe = res.describe
	row.quality = res.quality + 0
	row.bind_type = res.bind_type

	return row
end



--通过装备id，获得装备的信息
LogicTable.getEquipById = function(id)
	local row = XmlTable_getRow(mEquipment, id, true)
	local equip = {}

	equip.id = row.id
	equip.type = row.type
	equip.gem_trough = row.gem_trough 
	equip.life = row.life
	equip.atk = row.atk
	equip.speed = row.speed
	equip.hit_ratio = row.hit_ratio
	equip.miss_ratio = row.miss_ratio
	equip.critical_ratio = row.critical_ratio
	equip.tenacity = row.tenacity
	equip.mf_rule = row.mf_rule
	equip.attr_ids = CommonFunc_split(row.attr_ids, ",")
	equip.strengthen_id = row.strengthen_id
	equip.equip_level = row.equip_level + 0
	equip.equip_use_level = row.equip_use_level + 0

	return equip
end

--通过装备数字类型，获得装备名字
LogicTable.getEquipTypeByType = function(index)
    local equipType = math.floor(index/10)
	local strEquip = "空"
    if equipType== equipment_type["weapon"]  then
		strEquip = "武器"
	elseif equipType == equipment_type["armor"] then 
		strEquip = "护甲"
	elseif equipType == equipment_type["necklace"] then 
		strEquip = "项链"
	elseif equipType == equipment_type["ring"] then 
		strEquip = "戒指"
	elseif equipType == equipment_type["jewelry"] then 
		strEquip = "饰品"
	elseif equipType == equipment_type["medal"] then 
		strEquip = "勋章"
	end
    return strEquip
end
    

-- 错误代码
LogicTable.getErrorById = function(id)
	local res = XmlTable_getRow(mError_Code, id, false)
	if res == nil then 
		return nil
	end
	local row = {}

	row.id = res.id
	row.type = res.type
	row.text = res.text
	row.eventtype = res.eventtype

	return row
end


--获得扩展背包费用
LogicTable.getExpandPrice = function(the_time)
	local res = XmlTable_getRow(mExtendPack, the_time, true)
	local row = {}

	row.the_time = res.the_time	
	row.price = res.price + 0

	return row.price
end

-- 获取姓
LogicTable.getRadomNameTable = function(id)
	local res = XmlTable_getRow(mRandomNameTable, id, true)
	local row = {}

	row.id = res.id
	row.relate_id = res.relate_id
	row.probability = res.probability
	row.content = CommonFunc_split(res.content, ",")

	return row
end
-- 获取名
LogicTable.getRadomSecondNameTable = function(id)
	local res = XmlTable_getRow(mRandomSecondNameTable, id, true)
	local row = {}

	row.id = res.id
	row.content = CommonFunc_split(res.content, ",")

	return row
end

-- 游戏离散数据
LogicTable.getGameDateTable = function()
	local res = XmlTable_getRow(mGame_discrete_date, "gamedate", true)
	local row = {}

	row.speed = res.speed
	row.attack = res.attack

	return row
end

local mGame_CopyGroup = XmlTable_load("copy_group_tplt.xml", "id")  -- 副本群
local mGame_Copy = XmlTable_load("copy_tplt.xml", "id")  -- 副本

--获取所有副本群
LogicTable.getAllCopyGroup = function()
	local tb = {}
	for k, v in pairs(mGame_CopyGroup.map) do
		local row = {}

		row.id = v.id
		row.name = v.name
		row.type = v.type
		row.next_group_id = v.next_group_id
		row.icon = v.icon
		row.first_copy_id = v.first_copy_id

		table.insert(tb, row)
	end
	table.sort(tb, function(a, b) return tonumber(a.id) < tonumber(b.id) end)

	return tb
end

--获取所有副本
LogicTable.getGameAllFB = function()
	local tb = {}
	for k, v in pairs(mGame_Copy.map) do
		local row = {}

		row.id = v.id + 0
		row.type = v.type
		row.name = v.name
		row.icon = v.icon
		row.copy_group_id = v.copy_group_id + 0
		row.need_power = v.need_power
		row.gold = v.gold
		row.exp = v.exp
		row.award = v.award
		row.describe = v.describe
		row.first_map_id = v.first_copy_id
		row.pre_copy = CommonFunc_split(v.pre_copy, ",")
		row.need_stone = v.need_stone + 0
		row.dropitems = CommonFunc_split(v.dropitems, ",")

		table.insert(tb, row)
	end
	table.sort(tb, function(a, b) return tonumber(a.id) < tonumber(b.id) end)

	return tb
end

LogicTable.getCopyGroupById = function(id)
	if id == nil then return end
	local res = XmlTable_getRow(mGame_CopyGroup, id, true)
	local row = {}

	row.id = res.id	
	row.type = res.type
	row.name = res.name
	row.next_group_id = res.next_group_id
	row.icon = res.icon
	row.first_copy_id = res.first_copy_id
	row.last_copy_id = res.last_copy_id

	return row
end

LogicTable.getCopyById = function(id)
	local res = XmlTable_getRow(mGame_Copy, id, true)
	local row = {}

	row.id = res.id
	row.type = res.type
	row.name = res.name
	row.icon = res.icon
	row.copy_group_id = res.copy_group_id
	row.need_power = res.need_power
	row.gold = res.gold
	row.exp = res.exp
	row.award = res.award
	row.describe = res.describe
	row.first_map_id = res.first_map_id
	row.pre_copy = CommonFunc_split(res.pre_copy, ",")
	row.need_stone = res.need_stone
	row.small_icon = res.small_icon
	row.dropitems = CommonFunc_split(res.dropitems, ",")

	return row
end


LogicTable.getAllItems = function()
	local tb = {}
	for k, v in pairs(mItem_Pack.map) do
		local row = {}

		row.id = v.id + 0
		row.type = v.type + 0
		row.name = v.name
		row.overlay_count = v.overlay_count + 0
		row.sell_price = v.sell_price + 0
		row.sub_id = v.sub_id + 0
		row.icon = v.icon
		row.describe = v.describe
		row.bind_type = v.bind_type + 0
		row.quality = v.quality + 0

        table.insert(tb, row)
	end
	table.sort(tb, function(a, b) return tonumber(a.id) < tonumber(b.id) end)
	return tb
end

-- 根据任务id获取任务信息
LogicTable.getTaskRow = function(task_id)
	local res = XmlTable_getRow(mTaskTable, task_id, true)
	local row = {}
	
	row.id = tonumber(res.id)						-- 任务id
	row.title = res.title							-- 任务标题
	row.main_type = tonumber(res.main_type)			-- 任务主类型,1-主线,2-支线
	row.need_level = tonumber(res.need_level)		-- 任务等级要求
	row.next_ids = tonumber(res.next_ids)			-- 后置任务id	
	row.sub_type = tonumber(res.sub_type)			-- 任务子类型,1-击杀怪物,2-通关副本,3-收集物品	
	row.monster_id = tonumber(res.monster_id)		-- 怪物id		
	row.clear_type = tonumber(res.clear_type)		-- 通关类型	
	row.collect_id = tonumber(res.collect_id)		-- 收集物品id		
	row.location = tonumber(res.location)			-- 任务地点		
	row.number = tonumber(res.number)				-- 任务要求数量		
	row.text = res.text								-- 任务描述	
	row.exp_reward = tonumber(res.exp_reward)		-- 经验奖励
	row.money_reward = tonumber(res.money_reward)	-- 金钱奖励
	row.item_reward = tonumber(res.item_reward)		-- 物品奖励

	return row
end

-- 获取商城物品信息
LogicTable.getProductData = function()
	local function func(row)
		return "1" == row.show
	end
	return XmlTable_getRowArray(mTbProductsInfo, func)
end

-- 通过物品id获取商城物品信息
LogicTable.getProductInfoById = function(id)
	local row = XmlTable_getRow(mTbProductsInfo, id, true)
	local tbRow = {}

	tbRow.id = row.id
	tbRow.mall_item_type = row.mall_item_type		-- 1 物品, 2 符文
	tbRow.item_id = row.item_id
	tbRow.item_amount = row.item_amount
	tbRow.price_type = row.price_type
	tbRow.price = row.price + 0
	tbRow.vip_discount = row.vip_discount + 0
	tbRow.mark = row.mark
	tbRow.buy_limit = row.buy_limit + 0
	tbRow.tag_id = row.tag_id
	tbRow.show = row.show
	
	return tbRow
end

-- 获取积分商城物品信息
LogicTable.getGameRankProductData = function()
	local function func(row)
		return "1" == row.show
	end
	return XmlTable_getRowArray(mTbGameRankProductsInfo, func)
end

-- 通过物品id获取积分商城物品信息
LogicTable.getGameRankProductInfoById = function(id)
	local row = XmlTable_getRow(mTbGameRankProductsInfo, id, true)
	local tbRow = {}
	
	tbRow.id = row.id
	tbRow.mall_item_type = row.mall_item_type	-- 1 物品, 2 符文
	tbRow.item_id = row.item_id
	tbRow.item_amount = row.item_amount
	tbRow.price = row.price + 0
	tbRow.tag_id = row.tag_id
	tbRow.need_rank = row.need_rank + 0
	tbRow.show = row.show
		
	return tbRow
end

-- 公告
LogicTable.getPostsData = function()
	return mTbPostInfo.map
end

-- 通过公告id获取公告信息
LogicTable.getPostsInfoById = function(id)
	local row = XmlTable_getRow(mTbPostInfo, id, true)
	local tbRow = {}
	
	tbRow.id = row.id
	tbRow.text = row.text
	tbRow.display = row.display
	
	return tbRow
end

-- 通过type获取(商城)分页按钮, type:1->商城, 2->积分商城, ...
LogicTable.getRadioDataByType = function(type)
	local function func(row)
		return tostring(type) == row.mall_type
	end
	return XmlTable_getRowArray(mtbRadioBtnInfo, func)
end

-- 根据礼包id获取礼包信息
LogicTable.getGiftBagRow = function(gift_bag_id)
	local res = XmlTable_getRow(mGiftBagTable, gift_bag_id, true)
	local row = {}
				
	row.id = res.id						-- 礼包id			
	row.name = res.name					-- 礼包名称			
	row.icon = res.icon					-- 礼包图标		
	row.gold = res.gold					-- 金币数量	
	row.gem_amount = res.gem_amount		-- 宝石数量	
	if "-1" == res.gem_list then		-- 宝石列表
		row.gem_list = {}
	else
		row.gem_list = CommonFunc_split(res.gem_list, ",")
	end	
	row.chip_amount = res.chip_amount	-- 符文碎片数量	
	row.chip_id = res.chip_id			-- 符文碎片id

	return row
end

-- 根据玩家等级获取每日奖励信息
LogicTable.getDailyAwardRow = function(player_level)
	local row = {}
	for k, v in pairs(mDailyAwardTable.map) do
		row.level_range = CommonFunc_split(v.level_range, ",")	-- 玩家等级范围
		row.days1_award = v.days1_award		-- 登录1天礼包id
		row.days3_award = v.days3_award		-- 连续登录3天礼包id
		row.days7_award = v.days7_award		-- 连续登录7天礼包id		
		row.days15_award = v.days15_award		-- 连续登录15天礼包id
		if 1 == #row.level_range then
			if tonumber(player_level) >= tonumber(row.level_range[1]) then
				break
			end
		elseif 2 == #row.level_range then
			if tonumber(player_level) >= tonumber(row.level_range[1]) and 
			   tonumber(player_level) <= tonumber(row.level_range[2]) then
				break
			end
		end
	end
	return row
end

-- 通过类型获取所有音频文件
LogicTable.getAudioFileTbByType = function(type)
	local function func(row)
		return tostring(type) == row.type
	end
	return XmlTable_getRowArray(mTbAudioInfo, func)
end
	
-- 通过id获取音频文件信息
LogicTable.getAudioInfoById = function(id)
	local row = XmlTable_getRow(mTbAudioInfo, id, true)
	local data = {}
	
	data.id = row.id
	data.type = row.type
	data.name = row.name
	
	return data
end

-- 通过id获取音频文件名
LogicTable.getAudioFileNameById = function(id)
	id = tostring(id)
	return LogicTable.getAudioInfoById(id).name
end







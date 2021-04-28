----------------------------------------------------------------------
-- Author:	jaron.ho
-- Date:	2013-11-19
-- Brief:	event define
----------------------------------------------------------------------
local function EventDef_enum(tb)
	local enumTb = {}
	for key, val in pairs(tb) do
		assert(nil == enumTb[val], "duplicate event: "..val)
		enumTb[val] = key
	end
	return enumTb
end
----------------------------------------------------------------------
EventDef = {
	"ED_GAME_INIT",							-- 游戏初始
	"ED_CHECK_VERSION",						-- 检查版本
	"ED_LOGIN_SUCCESS",						-- 登录成功
	"ED_POWER_LEFT_TIME",					-- 体力倒计时
	"ED_POWER",								-- 体力改变
	--"ED_NEW_DAY",							-- 12时，刷新数据（体力）
	"ED_TOUCH_GRID_BEGIN",					-- 触摸格子开始
	"ED_TOUCH_GRID_MOVE",					-- 触摸格子移动
	"ED_TOUCH_GRID_END",					-- 触摸格子结束
	"ED_CLEAR_BEGIN",						-- 消除开始
	"ED_CLEAR_GRID",						-- 消除格子
	"ED_CLEAR_END",							-- 消除结束
	"ED_DROP_BEGIN",						-- 掉落开始
	"ED_DROP_END",							-- 掉落结束
	"ED_ROUND_OVER",						-- 回合结束
	"ED_GAME_OVER",							-- 游戏结束
	"ED_KILL_MONSTER",						-- 杀死怪物
	"ED_GAME_SUCCESS",						-- 游戏成功
	"ED_UPDATA_COPY_BTN",					-- 更新主页按钮状态
	"ED_CHANGE_REWARD_DATA",				-- 中间界面数字跳动动画
}
----------------------------------------------------------------------
EventDef = EventDef_enum(EventDef)
----------------------------------------------------------------------
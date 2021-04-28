----------------------------------------------------------------------
-- Author:	jaron.ho
-- Date:	2016-02-24
-- Brief:	公式
----------------------------------------------------------------------
Formula = {}
----------------------------------------------------------------------
-- 计算夹角
function Formula:calcAngle(v1, v2)
	local x = v1.x - v2.x
	local y = v1.y - v2.y
	local side = math.sqrt(x*x + y*y)
	local radian = math.acos(x/side)
	local angle = 180/(math.pi/radian)
	if y < 0 then
		angle = 360 - angle
	elseif 0 == y and x < 0 then
		angle = 180
	end
	return angle
end
----------------------------------------------------------------------
-- 计算距离
function Formula:calcDistance(p1, p2)
	local x = math.abs(p1.x - p2.x)
	local y = math.abs(p1.y - p2.y)
	return math.sqrt(x*x + y*y)
end
----------------------------------------------------------------------
-- 点是否在矩形区域
function Formula:isPointInRect(rect, p)
	local xMin = rect.x
	local xMax = rect.x + rect.width
	local yMin = rect.y
	local yMax = rect.y + rect.height
	return p.x >= xMin and p.x <= xMax and p.y >= yMin and p.y <= yMax
end
----------------------------------------------------------------------
-- 判断点在线段的哪一侧(0.线段所在直线上,1.线段所在直线的左侧,2.线段所在直线的右侧)
function Formula:checkPointSideWithSegment(sX, sY, eX, eY, x, y)
    local factor = (y - sY)*(eX - sX)/(eY - sY) + sY
    if factor < x then
        return 2
    else if factor > x then
        return 1
    end
    return 0
end
----------------------------------------------------------------------
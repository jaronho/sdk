----------------------------------------------------------------------
-- Author:	jaron.ho
-- Date:	2016-03-26
-- Brief:	A star
----------------------------------------------------------------------
-- 计算H值
local function calcHScore(rowA, colA, rowB, colB, costB)
	local rowDiff = math.abs(rowA - rowB)
	local colDiff = math.abs(colA - colB)
	return math.sqrt(math.pow(rowDiff, 2) + math.pow(colDiff, 2)) + (costB or 0)
end
----------------------------------------------------------------------
-- 查找列表中,F值最低的节点
local function lowestFScore(coordMap, fScoreMap)
	local lowestScore = 1/0		-- 初始INF(无穷大)
	local lowestCoord = nil
	for row, colTb in pairs(coordMap) do
		for col, _ in pairs(colTb) do
			if fScoreMap[row] and fScoreMap[row][col] then
				if fScoreMap[row][col] < lowestScore then
					lowestScore = fScoreMap[row][col]
					lowestCoord = {row=row, col=col}
				end
			end
		end
	end
	return lowestCoord
end
----------------------------------------------------------------------
-- 反向查找得到路径
local function unwindPath(parentMap, row, col, path)
	if parentMap[row] and parentMap[row][col] then
		local parent = parentMap[row][col]
		table.insert(path, 1, {row=parent.row, col=parent.col}) 
		unwindPath(parentMap, parent.row, parent.col, path)
	end
end
----------------------------------------------------------------------
-- 列表大小
local function mapSize(map)
	local count = 0
    for _, v in pairs(map) do
        count = count + 1
    end
    return count
end
----------------------------------------------------------------------
-- A*算法搜索
function AStar(rowS, colS, rowE, colE, costCF)
	if "function" ~= type(costCF) then
		costCF = function() return 0 end
	end
	-- 开启列表
	local openMap = {[rowS]={[colS]=true}}
	-- 关闭列表
	local closedMap = {}
	-- 父节点列表
	local parentMap = {}
	-- G值列表
	local gScoreMap = {[rowS]={[colS]=0}}
	-- F值列表,F(初始点经由该点到目标点的估价) = G(初始点到该点的实际代价) + H(从点A到点B的估价)
	local fScoreMap = {[rowS]={[colS]=gScoreMap[rowS][colS] + calcHScore(rowS, colS, rowE, colE)}}
	-- 遍历节点
	local path = {}
	while mapSize(openMap) > 0 do
		local coord = lowestFScore(openMap, fScoreMap)	-- 找到F最小值
		if rowE == coord.row and colE == coord.col then	-- 找到目标点
			unwindPath(parentMap, rowE, colE, path)
			table.insert(path, {row=rowE, col=colE})
			return path
		end
		-- 从开启列表中删除选中节点	
		openMap[coord.row] = openMap[coord.row] or {}
		openMap[coord.row][coord.col] = nil
		-- 把选中节点放入到关闭列表中
		closedMap[coord.row] = closedMap[coord.row] or {}
		closedMap[coord.row][coord.col] = true
		-- 获取周围节点
		local aroundList = {
			up = {row=coord.row - 1, col=coord.col},				-- 上边
			down = {row=coord.row + 1, col=coord.col},				-- 下边
			left = {row=coord.row, col=coord.col - 1},				-- 左边
			right = {row=coord.row, col=coord.col + 1},				-- 右边
			left_up = {row=coord.row - 1, col=coord.col - 1},		-- 左上
			left_down = {row=coord.row + 1, col=coord.col - 1},		-- 左下
			right_up = {row=coord.row - 1, col=coord.col + 1},		-- 右上
			right_down = {row=coord.row + 1, col=coord.col + 1},	-- 右下
		}
		for _, around in pairs(aroundList) do
			-- 计算around的G值
			local aroundCost = costCF(coord.row, coord.col, around.row, around.col)
			-- 节点没有在关闭列表,说明该节点还没有被选中
			if aroundCost and (not closedMap[around.row] or not closedMap[around.row][around.col]) then
				local gScore = gScoreMap[coord.row][coord.col] + calcHScore(coord.row, coord.col, around.row, around.col, aroundCost)
				-- 节点不在开启列表,或G值更小
				if not openMap[around.row] or not openMap[around.row][around.col] or gScore < gScoreMap[around.row][around.col] then
					-- 记录父节点
					parentMap[around.row] = parentMap[around.row] or {}
					parentMap[around.row][around.col] = {row=coord.row, col=coord.col}
					-- 替换G值
					gScoreMap[around.row] = gScoreMap[around.row] or {}
					gScoreMap[around.row][around.col] = gScore
					-- 计算F值
					fScoreMap[around.row] = fScoreMap[around.row] or {}
					fScoreMap[around.row][around.col] = gScore + calcHScore(around.row, around.col, rowE, colE)
					-- 节点不在开启列表
					if not openMap[around.row] or not openMap[around.row][around.col] then
						openMap[around.row] = openMap[around.row] or {}
						openMap[around.row][around.col] = true
					end
				end
			end
		end
	end
	return path
end
----------------------------------------------------------------------
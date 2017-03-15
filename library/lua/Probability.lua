----------------------------------------------------------------------
-- Author:	jaron.ho
-- Date:	2014-12-23
-- Brief:	calculate probability by weight list
----------------------------------------------------------------------
math.randomseed(os.time())
-- create probability object,weightList:{{1,20},{2,60}},contains two random value: 1.value,20.weight;2.value,60.weight
function CreateProbability(weightList, randomFunc)
	assert("table" == type(weightList), "weight list is not table, it's type is "..type(weightList))
	local weightListInit = {}
	for i, factor in pairs(weightList) do
		assert("table" == type(factor), "weight list at index ["..i.."] is not table, it't type is "..type(factor))
		assert(nil ~= factor[1], "weight list at index ["..i.."] format is error, not support value type "..type(factor[1]))
		assert("number" == type(factor[2]), "weight list at index ["..i.."] format is error, not support weight type "..type(factor[2]))
		assert(factor[2] >= 0, "weight list at index ["..i.."] is error, not support weight value "..tostring(factor[2]).." < 0")
		weightListInit[i] = {factor[1], factor[2]}
	end
	-- private member variables
	local mThreshold = 1
	local mWeightRange = {}
	local probability = {}
	-- private methods
	local function random(min, max)
		if "function" ~= type(randomFunc) then
			randomFunc = function(min, max)
				return math.random(min, max)
			end
		end
		local randomValue = randomFunc(min, max)
		assert("number" == type(randomValue), "random value is not number")
		return randomValue
	end
	local function parseWeightList()
		for index, _ in pairs(weightList) do
			local randIndex = random(1, index)
			if randIndex ~= index then
				local temp = weightList[randIndex]
				weightList[randIndex] = weightList[index]
				weightList[index] = temp
			end
		end
		mThreshold = 1
		mWeightRange = {}
		for _, factor in pairs(weightList) do
			assert("table" == type(factor) and "number" == type(factor[2]), "weightList format is error")
			local value = factor[1]
			local weight = factor[2]
			if weight > 0 then
				local range = {["value"] = value, ["begin"] = mThreshold, ["end"] = mThreshold + weight - 1}
				table.insert(mWeightRange, range)
				mThreshold = mThreshold + weight
			end
		end
	end
	-- public methods
	function probability:getValue()
		if 1 == mThreshold then
			return
		end
		local index = random(1, mThreshold - 1)
		for _, range in pairs(mWeightRange) do
			if index >= range["begin"] and index <= range["end"] then
				return range["value"]
			end
		end
	end
	function probability:getWeight(value)
		for _, factor in pairs(weightList) do
			if value == factor[1] then
				return factor[2]
			end
		end
		return 0
	end
	function probability:getTotalWeight()
		local totalWeight = 0
		for _, factor in pairs(weightList) do
			totalWeight = totalWeight + factor[2]
		end
		return totalWeight
	end
	function probability:setWeight(value, weight)
		assert(nil ~= value, "not support value type "..type(value))
		assert("number" == type(weight), "not support weight type "..type(weight))
		assert(weight >= 0, "not support weight value "..weight.." < 0")
		local isFind = false
		for i, factor in pairs(weightList) do
			if value == factor[1] then
				weightList[i][2] = weight
				isFind = true
				break
			end
		end
		if not isFind then
			table.insert(weightList, {value, weight})
		end
		parseWeightList()
	end
	function probability:reset()
		weightList = {}
		for i, factor in pairs(weightListInit) do
			weightList[i] = {factor[1], factor[2]}
		end
		parseWeightList()
	end
	-- initialize
	parseWeightList()
	return probability
end
----------------------------------------------------------------------
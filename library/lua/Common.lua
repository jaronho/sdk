----------------------------------------------------------------------
-- Author:	jaron.ho
-- Date:	2013-11-29
-- Brief:	common functions
----------------------------------------------------------------------
math.randomseed(os.time())
Common = {}
----------------------------------------------------------------------
-- Brief:	check variable is nil
-- Param:	variable
-- Return:	true/false
----------------------------------------------------------------------
function isNil(variable)
	if "userdata" == type(variable) then
		return tolua.isnull(variable)
	end
	return nil == variable
end
----------------------------------------------------------------------
-- Brief:	check variable is object
-- Param:	variable
-- Return:	true/false
----------------------------------------------------------------------
function isObject(variable)
	if "userdata" == type(variable) then
		return not tolua.isnull(variable)
	end
	return "table" == type(variable)
end
----------------------------------------------------------------------
-- Brief:	call function
-- Param:	func - function
--			target - target 
-- Return:	true/false
----------------------------------------------------------------------
function callFunction(func, target, ...)
	if "function" == type(func) then
		if "table" == type(target) or "userdata" == type(target) then
			func(target, ...)
		else
			func(...)
		end
	end
end
----------------------------------------------------------------------
-- Brief:	run command
-- Param:	cmd - command string
-- Return:	result table
----------------------------------------------------------------------
function Common:runCmd(cmd)
	if "string" ~= type(cmd) or 0 == string.len(cmd) then
		return {}
	end
	local resultTb = {}
	local result = io.popen(cmd)
	if result then
		for k, v in result:lines() do
			table.insert(resultTb, k)
		end
		result:close()
	end
	return resultTb
end
----------------------------------------------------------------------
-- Brief:	read file
-- Param:	fileName - file name
-- Return:	file content, error msg
----------------------------------------------------------------------
function Common:readFile(fileName)
	local file, errorMsg = io.open(fileName, "rb")
	if nil == file then
		return nil, errorMsg
	end
	local str = file:read("*all")
	io.close(file)
	return str, errorMsg
end
----------------------------------------------------------------------
-- Brief:	write content to file
-- Param:	fileName - file name
--			str - content to write
-- Return:	true/false
----------------------------------------------------------------------
function Common:writeFile(fileName, str)
	local file, errorMsg = io.open(fileName, "wb")
	if nil == file then
		return false, errorMsg
	end
	if file:write(str) then
		io.close(file)
		return true, errorMsg
	end
	io.close(file)
	return false, errorMsg
end
----------------------------------------------------------------------
-- Brief:	rename file name
-- Param:	oldFileName - old file name
--			newFileName - new file name
--			forceRename - if exist new name file, do force rename
-- Return:	true/false
----------------------------------------------------------------------
function Common:renameFile(oldFileName, newFileName, forceRename)
	assert("string" == type(oldFileName) and string.len(oldFileName) > 0)
	assert("string" == type(newFileName) and string.len(newFileName) > 0)
	if forceRename then
		os.remove(newFileName)
	end
	return os.rename(oldFileName, newFileName)
end
----------------------------------------------------------------------
-- Brief:	reload module
-- Param:	fileName - module name
-- Return:	none
----------------------------------------------------------------------
function Common:reload(fileName, content)
	assert("string" == type(fileName) and string.len(fileName) > 0)
	if "string" == type(content) and string.len(content) > 0 then
		package.loaded[fileName] = assert(loadstring(content), content)()
		return package.loaded[fileName]
	else
		package.loaded[fileName] = nil
		return require(fileName)
	end
end
----------------------------------------------------------------------
-- Brief:	generate enum type
-- Param:	tb - {"aaaa", "bbbb", "cccc", ...}
-- Return:	{"aaaa"=1, "bbbb"=2, "cccc"=3, ...}, index start from 1
----------------------------------------------------------------------
function Common:enum(tb)
	local enumObj = {}
	for k, v in pairs(tb) do
		enumObj[v] = k
	end
	return enumObj
end
----------------------------------------------------------------------
-- Brief:	calculate size of table
-- Param:	tb - table format
--			isExceptNull - is except null
-- Return:	size
----------------------------------------------------------------------
function Common:size(tb, isExceptNull)
	local capacity = 0
	if "table" == type(tb) then
		for _, v in pairs(tb) do
			if not isExceptNull or nil ~= v then
				capacity = capacity + 1
			end
		end
	end
	return capacity
end
----------------------------------------------------------------------
-- Brief:	query data from table
-- Param:	tb - table format
--			conditionFunc - condition function, receive 2 params(k:key, v:value), return booelan
--			isQueryList - query list if true, or query single if false
-- Return:	return table if isQueryList is true, or return single value
----------------------------------------------------------------------
function Common:query(tb, conditionFunc, target, isQueryList)
	local resultList = {}
	if "table" == type(tb) and "function" == type(conditionFunc) then
		for k, v in pairs(tb) do
			if "table" == type(target) or "userdata" == type(target) then
				if conditionFunc(target, k, v) then
					if isQueryList then
						table.insert(resultList, v)
					else
						return v
					end
				end
			else
				if conditionFunc(k, v) then
					if isQueryList then
						table.insert(resultList, v)
					else
						return v
					end
				end
			end
		end
	end
	if isQueryList then
		return resultList
	end
end
----------------------------------------------------------------------
-- Brief:	clone
-- Param:	obj - any data type
-- Return:	a clone objet of obj
----------------------------------------------------------------------
function Common:clone(obj)
    local lookupTable = {}
    local function copyObj(obj)
        if "table" ~= type(obj) then
            return obj
        elseif lookupTable[obj] then
            return lookupTable[obj]
        end
        local newTable = {}
        lookupTable[obj] = newTable
        for k, v in pairs(obj) do
            newTable[copyObj(k)] = copyObj(v)
        end
        return setmetatable(newTable, copyObj(getmetatable(obj)))
    end
    return copyObj(obj)
end
----------------------------------------------------------------------
-- Brief:	add arguments to new tb
-- Param:	... - arguments
-- Return:	tb
----------------------------------------------------------------------
function Common:join(...)
	local tb = {}
	for _, tbOrValue in pairs({...}) do
		if "table" == type(tbOrValue) then
			for _, v in pairs(tbOrValue) do
				table.insert(tb, v)
			end
		elseif nil ~= tbOrValue then
			table.insert(tb, tbOrValue)
		end
	end
	return tb
end
----------------------------------------------------------------------
-- Brief:	delete string left/right space, e.g. "  a b " => "a b"
-- Param:	str - a string
-- Return:	new string
----------------------------------------------------------------------
function Common:stringDelLRS(str)
	assert("string" == type(str), "not support type "..type(str))
	return string.match(str,"%s*(.-)%s*$")
end
----------------------------------------------------------------------
-- Brief:	split a string
-- Param:	str - a string
--			delimiter - separate character
--			numberType - is value number type
-- Return:	a array of strings
----------------------------------------------------------------------
function Common:stringSplit(str, delimiter, numberType)
    if "string" ~= type(str) or 0 == string.len(str) or "string" ~= type(delimiter) or 0 == string.len(delimiter) then
		if numberType then
			return {tonumber(str)}
		else
			return {str}
		end
	end
	for _, speChar in pairs({"(", ")", ".", "%", "+", "-", "*", "?", "[", "^", "$"}) do
		if delimiter == speChar then
			delimiter = "%"..delimiter
			break
		end
	end
	local arr = {}
	while true do
		local pos = string.find(str, delimiter)
		if (not pos) then
			if numberType then
				arr[#arr + 1] = tonumber(str)
			else
				arr[#arr + 1] = str
			end
			break
		end
		local value = string.sub(str, 1, pos - 1)
		if numberType then
			arr[#arr + 1] = tonumber(value)
		else
			arr[#arr + 1] = value
		end
		str = string.sub(str, pos + 1, #str)
	end
	return arr
end
----------------------------------------------------------------------
-- Brief:	check character placeholder
-- Param:	ch - a character(number type)
-- Return:	placeholder
----------------------------------------------------------------------
function Common:characterPlaceholder(ch)
	local charsets = {0, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc}
	local i = #charsets
	while charsets[i] do
		if ch >= charsets[i] then
			return i
		end
		i = i - 1
	end
end
----------------------------------------------------------------------
-- Brief:	calculate the length of string
-- Param:	str - a string
-- Return:	total count, single byte count, multibyte count
----------------------------------------------------------------------
function Common:stringLength(str)
	if "string" ~= type(str) or 0 == string.len(str) then
		return 0, 0, 0
	end
	local totalCount, singleCount, multiCount = 0, 0, 0
	local startPos = 1
	while startPos <= string.len(str) do
		local placeholder = self:characterPlaceholder(string.byte(str, startPos))
		startPos = startPos + placeholder
		if 1 == placeholder then	-- single byte character
			singleCount = singleCount + 1
		else						-- multibyte character
			multiCount = multiCount + 1
		end
		totalCount = totalCount + 1
	end
	return totalCount, singleCount, multiCount
end
----------------------------------------------------------------------
-- Brief:	split string to unicode char list
-- Param:	str - a string
-- Return:	unicode char list
----------------------------------------------------------------------
function Common:unicodeList(str)
	local list = {}
	if "string" == type(str) and string.len(str) > 0 then
		for uchar in string.gmatch(str, "([%z\1-\127\194-\244][\128-\191]*)") do
			table.insert(list, uchar)
		end
	end
	return list
end
----------------------------------------------------------------------
-- Brief:	strip file info
-- Param:	fullFileName - full file name, e.g. d:/a.txt
-- Return:	{dirname = "d:/", filename = "a.txt", basename = "a", extname = ".txt"}
----------------------------------------------------------------------
function Common:stripFileInfo(fullFileName)
	assert("string" == type(fullFileName), "not support type "..type(fullFileName))
	local pos = string.len(fullFileName)
	local extpos = pos + 1
	while pos > 0 do
		local b = string.byte(fullFileName, pos)
		if 46 == b then		-- "."
			extpos = pos
		elseif 47 == b or 92 == b then	-- "/","\\"
			break
		end
		pos = pos - 1
	end
	extpos = extpos - pos
	local dirname = string.sub(fullFileName, 1, pos)
	local filename = string.sub(fullFileName, pos + 1)
	local basename = string.sub(filename, 1, extpos - 1)
	local extname = string.sub(filename, extpos)
	return {dirname = dirname, filename = filename, basename = basename, extname = extname}
end
----------------------------------------------------------------------
-- Brief:	check a probability value
-- Param:	percentage - percentage value, e.g. 1 = 100%, 0.02 = 2%
-- Return:	true or false
----------------------------------------------------------------------
function Common:probability(percentage)
	assert("number" == type(percentage), "not support for "..type(percentage))
	assert(percentage >= 0 and percentage <= 1, "range of percentage is worng")
	local randomValue = math.random()
	return randomValue > 0 and randomValue <= percentage
end
----------------------------------------------------------------------
-- Brief:	get a random value, Array: from array, Integer: N digit number
-- Param:	arrayOrInteger - array or integer
-- Return:	a random value
----------------------------------------------------------------------
function Common:getRandom(arrayOrInteger)
	if "table" == type(arrayOrInteger) then
		local valueCount = #arrayOrInteger
		if 0 == valueCount then
			return nil
		end
		return arrayOrInteger[math.random(1, valueCount)]
	elseif "number" == type(arrayOrInteger) then
		arrayOrInteger = math.ceil(math.abs(arrayOrInteger))
		local maxNum = 0
		for i=1, arrayOrInteger do
			maxNum = maxNum + math.pow(10, i - 1)*9
		end
		return math.random(0, maxNum)
	end
	assert(false, "not support for "..type(valueArray))
end
----------------------------------------------------------------------
-- Brief:	wash table
-- Param:	null
-- Return:	tb
----------------------------------------------------------------------
function Common:wash(tb)
	if "table" == type(tb) then
		local keyList = {}
		for k, v in pairs(tb) do
			table.insert(keyList, k)
		end
		for i=1, #keyList do
			local key = keyList[i]
			local rand = math.random(1, i)
			local randKey = keyList[rand]
			local temp = tb[randKey]
			tb[randKey] = tb[key]
			tb[key] = temp
		end
	end
	return tb
end
----------------------------------------------------------------------
-- Brief:	generate unique id
-- Param:	bit - a number[10, 24], e.g. 19
-- Return:	a unique id string, e.g. "1447817348611214450"
----------------------------------------------------------------------
function Common:createUniqueId(bit)
	local defaultVal = tostring(os.time())
	if "number" ~= type(bit) or bit < string.len(defaultVal) then
		return defaultVal
	end
	if bit > 24 then
		bit = 24
	end
	local nt = defaultVal..string.gsub(tostring(os.clock()), "%.", '')
	local offset = bit - string.len(nt)
	if 0 == offset then
		return nt
	elseif offset < 0 then
		return string.sub(nt, 1, bit)
	end
	local randomValue = self:getRandom(offset)
	return nt..string.format("%".."0"..offset.."d", math.random(0, randomValue))
end
----------------------------------------------------------------------
-- Brief:	math round
-- Param:	num - a number
-- Return:	a number
----------------------------------------------------------------------
function Common:mathRound(num)
    if "number" ~= type(num) then
		return 0
	end
	if num >= 0 then
		return math.floor(num + 0.5)
	end
	return math.floor(num - 0.5)
end
----------------------------------------------------------------------
-- Brief:	change decimalism to binary
-- Param:	decimalism - a decimalism number, e.g. 115
-- Return:	a binary string, e.g. "1110011"
----------------------------------------------------------------------
function Common:decimalismToBinary(decimalism)
	local binary = ""
	local function innerFunc(value)
		local divisor = math.floor(value/2)
		local mod = value % 2
		binary = mod..binary
		if 0 == divisor then
			return binary
		end
		return innerFunc(divisor)
	end
	return innerFunc(math.floor(decimalism))
end
----------------------------------------------------------------------
-- Brief:	change binary to decimalism
-- Param:	binary - a binary string, e.g. "1110011"
-- Return:	a decimalism number, e.g. 115
----------------------------------------------------------------------
function Common:binaryToDecimalism(binary)
	local decimalism = 0
	local length = string.len(binary)
	for i=1, length do
		local b = string.byte(binary, i) - 48
		decimalism = decimalism + b*(2^(length-i))
	end
	return decimalism
end
----------------------------------------------------------------------
-- Brief:	bytes to number
-- Param:	bytes - byte array
--			endian - "big" or nil
--			signed - boolean or nil
-- Return:	number
----------------------------------------------------------------------
function Common:bytesToNumber(bytes, endian, signed)
	local t = {string.byte(bytes, 1, -1)}
	-- reverse bytes
	if "big" == endian then
		local tt = {}
		for k=1, #t do
			tt[#t - k + 1] = t[k]
		end
		t = tt
	end
	local num = 0
	for k=1, #t do
		num = num + t[k]*2^((k - 1)*8)
	end
	if signed then
		-- if last bit set, negative
		num = (num > 2^(#t - 1) - 1) and (num - 2^#t) or num
	end
	return num
end
----------------------------------------------------------------------
-- Brief:	number to bytes
-- Param:	num - number
--			endian - "big" or nil
--			signed - boolean or nil
-- Return:	byte array
----------------------------------------------------------------------
function Common:numberToBytes(num, endian, signed)
	if num < 0 and not signed then
		num = -num
	end
	local res = {}
	local n = math.ceil(select(2, math.frexp(num))/8) -- number of bytes to be used
	if signed and num < 0 then
		num = num + 2^n
	end
	for k=n, 1, -1 do -- 256 = 2^8 bits per char
		local mul = 2^(8*(k - 1))
		res[k] = math.floor(num/mul)
		num = num - res[k]*mul
	end
	assert(0 == num)
	if "big" == endian then
		local t = {}
		for k=1, n do
			t[k] = res[n - k + 1]
		end
		res = t
	end
	return string.char(unpack(res))
end
----------------------------------------------------------------------
-- Brief:	change color hex string to RGB
-- Param:	colorHex - a hex string, e.g. "#FF00FF"
-- Return:	a RGB tb, e.g. {r=255, g=0, b=255}
----------------------------------------------------------------------
function Common:colorHexToRgb(colorHex)
	assert("string" == type(colorHex) and 7 == string.len(colorHex) and "#" == string.sub(colorHex, 1, 1), "colorHex = ["..colorHex.."] format error")
	local red = tonumber(string.sub(colorHex, 2, 3), 16)
	local green = tonumber(string.sub(colorHex, 4, 5), 16)
	local blue = tonumber(string.sub(colorHex, 6, 7), 16)
	assert(red and red <= 255 and green and green <= 255 and blue and blue <= 255, "colorHex = ["..colorHex.."] format error")
	return {r=red, g=green, b=blue}
end
----------------------------------------------------------------------
-- Brief:	change color RGB to hex string
-- Param:	r, g, b - a RGB value , e.g. r=255, g=0, b=255
-- Return:	a hex string, e.g. "#FF00FF"
----------------------------------------------------------------------
function Common:colorRgbToHex(r, g, b)
	assert("number" == type(r) and r >=0 and r <= 255 and "number" == type(g) and g >=0 and g <= 255 and "number" == type(b) and b >=0 and b <= 255, "r = ["..r.."], g = ["..g.."], b = ["..b.."] format error")
	local redHex = string.upper(string.format("%02x", r))
	local greenHex = string.upper(string.format("%02x", g))
	local blueHex = string.upper(string.format("%02x", b))
	return "#"..redHex..greenHex..blueHex
end
----------------------------------------------------------------------
-- Brief:	parse string to tuple
-- Param:	stringTuple - {{1,2,3},{4,5,6}} or {{"a","b","c"},{"d","e","f"}}
--			delimiter - separate character, normal is ","
--			numberType - is value number type
-- Return:	table
----------------------------------------------------------------------
function Common:parseTuple(stringTuple, delimiter, numberType)
	local function removeLRBracket(str)
		local leftB, rightB, pos, count = 0, 0, 1, 0
		while pos <= string.len(str) do
			local ch = string.sub(str, pos, pos)
			if "{" == ch then
				count = count + 1
				if 0 == leftB then
					leftB = pos
				end
			elseif "}" == ch then
				count = count - 1
			end
			if 0 == count then
				rightB = pos
				break
			end
			pos = pos + 1
		end
		if 1 == leftB and string.len(str) == rightB then
			return string.sub(str, leftB + 1, rightB - 1), true
		end
		return str, false
	end
	local function innerParse(str, tuple, index)
		index = index or 1
		local tempStr, removeFlag = removeLRBracket(str)
		local tempTuple = nil
		if removeFlag then
			tuple[index] = {}
			tempTuple = tuple[index]
		else
			tempTuple = tuple
		end
		if nil == string.find(tempStr, delimiter) then
			if removeFlag then
				if numberType then
					table.insert(tempTuple, tonumber(tempStr))
				else
					table.insert(tempTuple, tempStr)
				end
			else
				if numberType then
					tempTuple[index] = tonumber(tempStr)
				else
					tempTuple[index] = tempStr
				end
			end
		else
			local blockTable, blockStr, startPos, left, right = {}, "", 1, 0, 0
			while startPos <= string.len(tempStr) do
				local character = string.sub(tempStr, startPos, startPos)
				startPos = startPos + 1
				if delimiter == character and left == right then
					table.insert(blockTable, blockStr)
					character, blockStr, left, right = "", "", 0, 0
				elseif "{" == character then
					left = left + 1
				elseif "}" == character then
					right = right + 1
				end
				blockStr = blockStr..character
			end
			assert(left == right, stringTuple.." format is error")
			table.insert(blockTable, blockStr)
			for k, v in pairs(blockTable) do
				innerParse(v, tempTuple, k)
			end
		end
	end
	local tupleTable = {}
	innerParse(stringTuple, tupleTable)
	return tupleTable
end
----------------------------------------------------------------------
-- Brief:	serialize a data to string
-- Param:	data - any data to be serialized
-- Return:	a serialize string
----------------------------------------------------------------------
function Common:serialize(data)
	local serializeStr = ""
	local function innerSerialize(value, keyFlag)
		local valueType = type(value)
		if "nil" == valueType or "boolean" == valueType then
			assert(not keyFlag, "cannot support type: "..valueType..", value: "..tostring(value).." as key")
			serializeStr = serializeStr..tostring(value)
		elseif "number" == valueType then
			if keyFlag then
				serializeStr = serializeStr.."["..value.."]="
			else
				serializeStr = serializeStr..value
			end
		elseif "string" == valueType then
			if keyFlag then
				serializeStr = serializeStr..value.."="
			else
				serializeStr = serializeStr..string.format("%q", value)
			end
		elseif "table" == valueType then
			assert(not keyFlag, "cannot support type: "..valueType..", value: "..tostring(value).." as key")
			serializeStr = serializeStr.."{"
			local index = 0
			for k, v in pairs(value) do
				index = index + 1
				serializeStr = serializeStr..(index > 1 and "," or "")
				innerSerialize(k, true)
				innerSerialize(v, false)
			end
			serializeStr = serializeStr.."}"
		else
			assert(nil, "cannot support type: "..valueType..", value: "..tostring(value))
		end
	end
	innerSerialize(data, false)
	return serializeStr
end
----------------------------------------------------------------------
-- Brief:	deserialize a string to data
-- Param:	str - string that has been serialized
-- Return:	data
----------------------------------------------------------------------
function Common:deserialize(str)
	local deserializeStr = "do local data = "..str.." return data end"
	local func = loadstring(deserializeStr)
	if "function" == type(func) then
		return func()
	end
	return nil
end
----------------------------------------------------------------------
-- Brief:	simple xor crypto
-- Param:	str - string to crypto
--			key - crypto key
-- Return:	string
----------------------------------------------------------------------
function Common:simpleXOR(str, key)
	local function d2b(arg)
		arg = arg >= 0 and arg or (0xFFFFFFFF + arg + 1)
		local tr = {}
		for i=1, 32 do
			local data32 = 2^(32 - i)
			if arg >= data32 then tr[i], arg = 1, arg - data32 else tr[i] = 0 end
		end
		return tr
	end
	local function b2d(arg)
		local nr = 0
		for i=1, 32 do
			if 1 == arg[i] then nr = nr + 2^(32 - i) end
		end
		return nr
	end
	local function xor(a, b)
		local op1, op2, r = d2b(a), d2b(b), {}
		for i=1, 32 do
			if op1[i] == op2[i] then r[i] = 0 else r[i] = 1 end
		end
		return b2d(r)
	end
	if "string" == type(str) and string.len(str) > 0 and "string" == type(key) and string.len(key) > 0 then
		local result, keyLength = "", string.len(key)
		for i=1, string.len(str) do
			local keyIndex = i%keyLength
			if 0 == keyIndex then
				keyIndex = keyLength
			end
			local strByte = string.byte(string.sub(str, i, i))
			local keyByte = string.byte(string.sub(key, keyIndex, keyIndex))
			result = result..string.char(xor(strByte, keyByte))
		end
		return result
	end
	return str
end
----------------------------------------------------------------------
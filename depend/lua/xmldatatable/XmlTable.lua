----------------------------------------------------------------------
-- 作者：hezhr
-- 日期：2013-11-21
-- 描述：xml数据表解析
----------------------------------------------------------------------

----------------------------------------------------------------------
-- 功  能：加载xml数据表
-- 参  数：fileName(string) - xml文件名,keyName(string) - 主键名
-- 返回值：{name - 文件名, map - 数据集(非线性), size - 数据量}
----------------------------------------------------------------------
function XmlTable_load(fileName, keyName)
	local xmlTB = assert(loadXmlFile(fileName), "XmlTable -> load() -> load file "..fileName.." failed")
	local tb = {name = fileName, map = {}, size = 0}
	for k, v in pairs(xmlTB) do
		local key = nil
		for name, value in pairs(v) do
			if keyName == name then
				key = value
				break
			end
		end
		assert(key and "" ~= key, "XmlTable -> load() -> key is nil with the name "..keyName.." at row "..k.." in file "..fileName)
		assert(nil == tb.map[key], "XmlTable -> load() -> key "..key.." is duplicate at row "..k.." in file "..fileName)
		tb.map[key] = v
		tb.size = tb.size + 1
	end
	return tb
end
----------------------------------------------------------------------
-- 功  能：获取指定行数据
-- 参  数：tb(table) - XmlTable_load返回的二维数组;key(number/string) - 键值;dumpNil(bool) - true(不允许查找不到结果),false或不填(允许查找不到结果)
-- 返回值：单条数据
----------------------------------------------------------------------
function XmlTable_getRow(tb, key, dumpNil)
	assert("number" == type(key) or "string" == type(key), "XmlTable -> getRow() -> key is not number or string, it's type is "..type(key))
	local row = tb.map[tostring(key)]
	if nil == row then
		local info = "XmlTable -> getRow() -> can't find key "..key.." in file "..tb.name
		if true == dumpNil then
			assert(nil, info)
		end
		print(info)
		return nil
	end
	local temp = {}
	for name, value in pairs(row) do temp[name] = value end
	return temp
end
----------------------------------------------------------------------
-- 功  能：获取指定行数据
-- 参  数：tb(table) - XmlTable_load返回的二维数组;condition(function) - 条件函数(返回值为布尔型)
-- 返回值：数据集(线性)
----------------------------------------------------------------------
function XmlTable_getRowArray(tb, condition)
	assert("function" == type(condition), "XmlTable -> XmlTable_getRowArray -> condition is not function")
	local rowTB = {}
	for k, v in pairs(tb.map) do
		local temp = {}
		for name, value in pairs(v) do temp[name] = value end
		if true == condition(temp) then
			table.insert(rowTB, temp)
		end
	end
	return rowTB
end
----------------------------------------------------------------------

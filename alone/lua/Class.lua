----------------------------------------------------------------------
-- Author:	jaron.ho
-- Date:	2014-04-08
-- Brief:	define type of class
----------------------------------------------------------------------
function setobjectclone(obj)
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
function setobjectmetatable(object, metatable)
	local function innerImpl(obj, mt)
		if "userdata" == type(obj) then
			if "table" == type(tolua)
				and "function" == type(tolua.getpeer)
				and "function" == type(tolua.setpeer) then
				local peer = tolua.getpeer(obj)
				if not peer then
					peer = {}
					tolua.setpeer(obj, peer)
				end
				setmetatable(peer, mt)
			end
		else
			local t = getmetatable(obj)
			if not t then
				t = {}
			end
			if not t.__index then
				t.__index = mt
				setmetatable(object, t)
			elseif t.__index ~= mt then
				innerImpl(t, mt)
			end
		end
	end
	innerImpl(object, metatable)
end
----------------------------------------------------------------------
function class(className, ...)
	assert("string" == type(className), "create class with invalid name type \""..type(className).."\"")
	local cls = {__cname = className}
	for _, super in ipairs({...}) do
		if "function" == type(super) then	-- if super is function, set it to __create
			assert(not cls.__create, "create class \""..className.."\" with more than one creating function")
			cls.__create = super
		elseif "table" == type(super) then
			if super[".isclass"] then	-- super is native class
				assert(not cls.__create, "create class \""..className.."\" with more than one creating function or native class")
				cls.__create = function(...)
					if "function" == type(super.create) then
						return super:create(...)
					end
				end
			else	-- super is pure lua class
				cls.__supers = cls.__supers or {}
				cls.__supers[#cls.__supers + 1] = super
				if not cls.super then
					cls.super = super	-- set first super pure lua class as class.super
				end
			end
		else
			error("create class \""..className.."\" with invalid super type \""..type(super).."\"", 0)
		end
	end
	cls.__index = cls
	if "table" == type(cls.__supers) and #cls.__supers > 1 then
		setmetatable(cls, {__index = function(_, key)
			for i=1, #cls.__supers do
				if cls.__supers[i][key] then
					return cls.__supers[i][key]
				end
			end
		end})
	else
		setmetatable(cls, {__index = cls.super})
	end
	if not cls.ctor then
		cls.ctor = function() end	-- add default constructor
	end
	cls.new = function(...)
		local clsClone = setobjectclone(cls)
		local instance = {}
		if "function" == type(clsClone.__create) then
			instance = clsClone.__create(...)
		end
		setobjectmetatable(instance, clsClone)
		instance.class = clsClone
		instance:ctor(...)
		return instance
	end
	cls.create = function(_, ...)
		return cls.new(...)
	end
	return cls
end
----------------------------------------------------------------------
-- Example:
--[[
People = class("People")
function People:ctor(name, sex, age)
	print("people -> ctor -> name="..name..", sex="..sex..", age="..age)
	self.name = name
	self.sex = sex
	self.age = age
end
function People:say()
	print("people -> say -> my name is "..self.name..", I'm a "..self.sex..", I'm "..self.age)
end
function People:stand()
	print("people -> stand")
end

Student = class("Student", People)
function Student:ctor(name, sex, age)
	self.super:ctor(name, sex, age)
	print("Student -> ctor")
end
function Student:stand()
	print("Student -> stand")
end

local stu1 = Student.new("xiaoming", "boy", 11)
stu1:stand()
stu1:say()
]]
----------------------------------------------------------------------
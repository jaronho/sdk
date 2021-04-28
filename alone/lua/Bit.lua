----------------------------------------------------------------------
-- Author:	jaron.ho
-- Date:	2015-09-17
-- Brief:	bit operation
----------------------------------------------------------------------
local function d2b(arg)
	arg = arg >= 0 and arg or (0xFFFFFFFF + arg + 1)
	local tr = {}
	for i = 1, 32 do
		local data32 = 2 ^ (32 - i)
		if arg >= data32 then
			tr[i] = 1
			arg = arg - data32
		else
			tr[i] = 0
		end
	end
	return tr
end
local function b2d(arg)
	local nr = 0
	for i = 1, 32 do
		if 1 == arg[i] then
			nr = nr + 2 ^ (32 - i)
		end
	end
	return nr
end
----------------------------------------------------------------------
function BitNot(a)
	local op1 = d2b(a)
	local r = {}
	for i = 1, 32 do
		if 1 == op1[i]then
			r[i] = 0
		else
			r[i] = 1
		end
	end
	return b2d(r)
end
----------------------------------------------------------------------
function BitAnd(a, b)
	local op1 = d2b(a)
	local op2 = d2b(b)
	local r = {}
	for i = 1, 32 do
		if 1 == op1[i] and 1 == op2[i] then
			r[i] = 1
		else
			r[i] = 0
		end
	end
	return b2d(r)
end
----------------------------------------------------------------------
function BitOr(a, b)
	local op1 = d2b(a)
	local op2 = d2b(b)
	local r = {}
	for i = 1, 32 do
		if 1 == op1[i] or 1 == op2[i]then
			r[i] = 1
		else
			r[i] = 0
		end
	end
	return b2d(r)
end
----------------------------------------------------------------------
function BitXor(a, b)
	local op1 = d2b(a)
	local op2 = d2b(b)
	local r = {}
	for i = 1, 32 do
		if op1[i] == op2[i] then
			r[i] = 0
		else
			r[i] = 1
		end
	end
	return b2d(r)
end
----------------------------------------------------------------------
function BitLShift(a, n)
	local op1 = d2b(a)
	local r = d2b(0)
	if n < 32 and n > 0 then
		for i = 1, n do
			for i = 1, 31 do
				op1[i] = op1[i + 1]
			end
			op1[32] = 0
		end
		r = op1
	end
	return b2d(r)
end
----------------------------------------------------------------------
function BitRShift(a, n)
	local op1 = d2b(a)
	local r = d2b(0)
	if n < 32 and n > 0 then
		for i = 1, n do
			for i = 31, 1, -1 do
				op1[i + 1] = op1[i]
			end
			op1[1] = 0
		end
		r = op1
	end
	return b2d(r)
end
----------------------------------------------------------------------
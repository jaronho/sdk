----------------------------------------------------------------------
-- Author:	jaron.ho
-- Date:	2015-10-23
-- Brief:	RC4 algorithm
----------------------------------------------------------------------
function rc4(data, key)
	assert("string" == type(data), "not support for type '"..type(data).."'")
	if 0 == string.len(data) or "string" ~= type(key) or 0 == string.len(key) then
		return data
	end
	local function KSA(key)
		local keyLen, s, keyByte, j = string.len(key), {}, {}, 0
		for i = 0, 255 do
			s[i] = i
		end
		for i = 1, keyLen do
			keyByte[i - 1] = string.byte(key, i, i)
		end
		for i = 0, 255 do
			j = (j + s[i] + keyByte[i % keyLen]) % 256
			s[i], s[j] = s[j], s[i]
		end
		return s
	end
	local function PRGA(s, length)
		local i, j, k = 0, 0, {}
		for n = 1, length do
			i = (i + 1) % 256
			j = (j + s[i]) % 256
			s[i], s[j] = s[j], s[i]
			k[n] = s[(s[i] + s[j]) % 256]
		end
		return k
	end
	local function bitxor(a, b)
		if a < b then
			a, b = b, a
		end
		local res, s = 0, 1
		while 0 ~= a do
			local r_a = a % 2
			local r_b = b % 2
			res = s * ((1 == r_a + r_b) and 1 or 0) + res 
			s = s * 2
			a = math.modf(a / 2)
			b = math.modf(b / 2)
		end
		return res
	end
	local function output(s, data)
		local res, length = {}, string.len(data)
		for i = 1, length do
			res[i] = string.char(bitxor(s[i], string.byte(data, i, i)))
		end
		return table.concat(res)
	end
	local s = KSA(key)
	local k = PRGA(s, string.len(data))
	return output(k, data)
end
----------------------------------------------------------------------
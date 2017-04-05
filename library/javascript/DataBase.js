/***********************************************************************
 ** Author:	jaron.ho
 ** Date:	2015-08-12
 ** Brief:	database for native
 ***********************************************************************/
function CreateDatabase(name, isForceCreate, errorFunc, readFunc, writeFunc, destroyFunc, cryptoFunc, target) {
	if ('string' != typeof(name)) {
		throw new Error("database name not support type '" + typeof(name) + "'");
	}
	if (0 == name.length) {
		throw new Error("database name is empty");
	}
	// private member variables
	var mDB = {};									// database table
	var mRand = Math.floor(Math.random()*99999);	// database rand
	var mCheck = null;								// database check
	var mIsChange = false;							// is data change
	var database = {};
	// private methods
	function cloneData(obj) {
		function copyObj(obj) {
			if (null == obj || 'object' != typeof(obj)) {
				return obj;
			}
			var tmpObj = {};
			if (obj instanceof Array) {
				tmpObj = [];
			}
			for (var key in obj) {
				if (obj.hasOwnProperty(key)) {
					tmpObj[key] = copyObj(obj[key]);
				}
			}
			return tmpObj;
		}
		return copyObj(obj);
	}
	function generateCheckValue(data, randSeed) {
		if ('number' == typeof(randSeed)) {
			randSeed = randSeed.toString();
		} else if ('string' != typeof(randSeed)) {
			randSeed = "";
		}
		function checkCode(str) {
			var strLength = str.length, total = 0;
			for (var i = 0; i < strLength; ++i) {
				total += str.charCodeAt(i);
			}
			return strLength.toString() + total.toString() + randSeed;
		}
		function innerFunc(value) {
			var valueType = typeof(value);
			if ('boolean' == valueType || 'number' == valueType || 'string' == valueType) {
				return checkCode(value.toString());
			} else if (value && 'object' == valueType) {
				var checkValue = {};
				for (var key in value) {
					if (value.hasOwnProperty(key)) {
						checkValue[key] = innerFunc(value[key]);
					}
				}
				return checkValue;
			}
		}
		return innerFunc(data);
	}
	function isEqualCheckValue(checkValue1, checkValue2) {
		function innerFunc(check1, check2) {
			var type1 = typeof(check1), type2 = typeof(check2);
			if (type1 !== type2) {
				return false;
			}
			if (check1 && 'object' == type1 && check2 && 'object' == type2) {
				for (var key in check1) {
					if (check1.hasOwnProperty(key) && !innerFunc(check1[key], check2[key])) {
						return false;
					}
				}
				return true;
			} else {
				return check1 == check2;
			}
		}
		return innerFunc(checkValue1, checkValue2);
	}
	// public methods
	database.save = function () {
		if (null == mCheck || !mIsChange) {
			return false;
		}
		var checkDB = generateCheckValue(mDB, mRand);
		for (var key in checkDB) {
			if (checkDB.hasOwnProperty(key) && !isEqualCheckValue(checkDB[key], mCheck[key])) {
				if ('function' == typeof(errorFunc)) {
					errorFunc.apply(target, [key]);
				} else {
					throw new Error("data is modified by third party plugin, key: " + key);
				}
				return false
			}
		}
		mIsChange = false;
		var localString = JSON.stringify(mDB);
		var resultString = null;
		if ('function' == typeof(cryptoFunc)) {
			resultString = cryptoFunc.apply(target, [localString, true]);
		}
		resultString = 'string' == typeof(resultString) ? resultString : localString;
		if ('function' == typeof(writeFunc)) {
			writeFunc.apply(target, [name, resultString]);
		} else if ('object' == typeof(window.localStorage) && 'function' == typeof(window.localStorage.setItem)) {
			try {
				window.localStorage.setItem(name, resultString);
			} catch (exception) {
				try {
					window.localStorage.setItem(name, localString);
				} catch (exception) {
					window.localStorage.setItem(name, '');
				}
			}
		}
		return true;
	};
	database.clear = function () {
		mDB = {};
		mRand = Math.floor(Math.random()*99999);
		mCheck = generateCheckValue(mDB, mRand);
		mIsChange = true;
		this.save();
	};
    database.destroy = function() {
        mDB = {};
        mCheck = null;
        mIsChange = false;
        if ('function' == typeof(writeFunc)) {
            destroyFunc.apply(target, [name]);
        } else if ('object' == typeof(window.localStorage) && 'function' == typeof(window.localStorage.removeItem)) {
            window.localStorage.removeItem(name);
        }
    };
	database.set = function (key, value) {
        if (null == mCheck) {
            return;
        }
		if ('string' != typeof(key)) {
			throw new Error("key not support for type '" + typeof(key) + "'");
		}
		var checkValue = generateCheckValue(mDB[key], mRand);
		if (isEqualCheckValue(checkValue, mCheck[key])) {
			if (undefined == value || null == value) {
				delete mDB[key];
				delete mCheck[key];
			} else {
				mDB[key] = cloneData(value);
				mCheck[key] = generateCheckValue(value, mRand);
			}
			mIsChange = true;
		} else {
			if ('function' == typeof(errorFunc)) {
				errorFunc.apply(target, [key]);
			} else {
				throw new Error("data is modified by third party plugin, key: " + key);
			}
		}
	};
	database.get = function (key) {
        if (null == mCheck) {
            return null;
        }
		if ('string' != typeof(key)) {
			throw new Error("key not support for type '" + typeof(key) + "'");
		}
		var value = mDB[key];
		var checkValue = generateCheckValue(value, mRand);
		if (isEqualCheckValue(checkValue, mCheck[key])) {
			return cloneData(value);
		} else {
			if ('function' == typeof(errorFunc)) {
				errorFunc.apply(target, [key]);
			} else {
				throw new Error("data is modified by third party plugin, key: " + key);
			}
		}
	};
	database.getName = function () {
		return name;
	};
	database.getAll = function () {
		return cloneData(mDB);
	};
    database.isExist = function() {
        return null != mCheck;
    };
	database.isEmpty = function() {
		for (var key in mDB) {
			if (mDB.hasOwnProperty(key)) {
				return false;
			}
		}
		return true;
	};
	// load database
	var localString = null;
	if ('function' == typeof(readFunc)) {
		localString = readFunc.apply(target, [name]);
	} else if ('object' == typeof(window.localStorage) && 'function' == typeof(window.localStorage.setItem)) {
		localString = window.localStorage.getItem(name);
	}
	if ('string' == typeof(localString) && localString.length >= 2) {
		try {
			var resultString = null;
			if ('function' == typeof(cryptoFunc)) {
				resultString = cryptoFunc.apply(target, [localString, false]);
			}
			resultString = 'string' == typeof(resultString) ? resultString : localString;
			mDB = JSON.parse(resultString);
			mCheck = generateCheckValue(mDB, mRand);
		} catch (exception) {
			try {
				mDB = JSON.parse(localString);
				mCheck = generateCheckValue(mDB, mRand);
			} catch (excepton) {
				database.clear();
			}
		}
	} else if (isForceCreate) {
        database.clear();
    }
	return database;
}
//----------------------------------------------------------------------
/***********************************************************************
 ** Author:	jaron.ho
 ** Date:	2015-08-12
 ** Brief:	calculate probability by weight list
 ***********************************************************************/
// create probability object,weightList:[[1,20],[2,60]],contains two random value: 1.value,20.weight;2.value,60.weight
function CreateProbability(weightList, randomFunc) {
	if (!(weightList instanceof Array)) {
		throw new Error("weightList is not table, it's type is " + typeof(weightList));
	}
	var weightListInit = [];
	for (var i = 0, len = weightList.length; i < len; ++i) {
		var factor = weightList[i];
		if (!(factor instanceof Array)) {
			throw new Error("weight list at index [" + i + "] is not table, it't type is " + typeof(factor));
		}
		if (undefined == factor[0] || null == factor[0]) {
			throw new Error("weight list at index [" + i + "] format is error, not support value type " + typeof(factor[0]));
		}
		if (isNaN(factor[1])) {
			throw new Error("weight list at index [" + i + "] format is error, not support weight type " + typeof(factor[1]));
		}
		if (factor[1] < 0) {
			throw new Error("weight list at index [" + i + "] is error, not support weight value " + factor[1] + " < 0");
		}
		weightListInit[i] = [factor[0], factor[1]];
	}
	// private member variables
	var mThreshold = 1;
	var mWeightRange = [];
	var probability = {};
	// private methods
	function random(min, max) {
		randomFunc = 'function' == typeof(randomFunc) ? randomFunc : function() {
			return Math.random();
		};
		var randomValue = randomFunc();
		if (isNaN(randomValue)) {
			throw new Error("random value is not number");
		}
		return Math.floor(randomValue * (max - min) + min + 0.5);
	}
	function parseWeightList() {
		for (var index = 0, count = weightList.length; index < count; ++index) {
			var randIndex = random(0, index);
			if (randIndex != index) {
				var temp = weightList[randIndex];
				weightList[randIndex] = weightList[index];
				weightList[index] = temp;
			}
		}
		mThreshold = 1;
		mWeightRange = [];
		for (var i = 0, len = weightList.length; i < len; ++i) {
			var factor = weightList[i];
			if (!(factor instanceof Array) || isNaN(factor[1])) {
				throw new Error("weightList format is error");
			}
			var value = factor[0];
			var weight = factor[1];
			if (weight > 0) {
				var range = {value: value, begin: mThreshold, end: mThreshold + weight - 1};
				mWeightRange.push(range);
				mThreshold += weight;
			}
		}
	}
	// public methods
	probability.getValue = function() {
		if (1 == mThreshold) {
			return;
		}
		var index = random(1, mThreshold - 1);
		for (var i = 0, len = mWeightRange.length; i < len; ++i) {
			var range = mWeightRange[i];
			if (index >= range.begin && index <= range.end) {
				return range.value;
			}
		}
	};
	probability.getWeight = function(value) {
		for (var i = 0, len = weightList.length; i < len; ++i) {
			var factor = weightList[i];
			if (value == factor[0]) {
				return factor[1];
			}
		}
		return 0;
	};
	probability.getTotalWeight = function() {
		var totalWeight = 0;
		for (var i = 0, len = weightList.length; i < len; ++i) {
			var factor = weightList[i];
			totalWeight += factor[1];
		}
		return totalWeight;
	};
	probability.setWeight = function(value, weight) {
		if (undefined == value || null == value) {
			throw new Error("not support value type " + typeof(value));
		}
		if (isNaN(weight)) {
			throw new Error("not support weight type " + typeof(value));
		}
		if (weight < 0) {
			throw new Error("not support weight value " + weight + " < 0");
		}
		var isFind = false;
		for (var i = 0, len = weightList.length; i < len; ++i) {
			if (value == weightList[i][0]) {
				weightList[i][1] = weight;
				isFind = true;
				break;
			}
		}
		if (!isFind) {
			weightList.push([value, weight]);
		}
		parseWeightList();
	};
	probability.reset = function() {
		weightList = [];
		for (var i = 0, len = weightListInit.length; i < len; ++i) {
			var factor = weightListInit[i];
			weightList[i] = [factor[0], factor[1]];
		}
		parseWeightList();
	};
	// initialize weight list
	parseWeightList();
	return probability;
}
//----------------------------------------------------------------------
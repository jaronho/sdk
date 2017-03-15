//--------------------------------------------------------------------
// Author:	jaron.ho
// Date:	2016-09-19
// Brief:	线性同余随机数生成器
//			古老的LCG(linear congruential generator)代表了最好最朴素的伪随机数产生器算法
// 			主要原因是容易理解,容易实现,而且速度快
//			LCG算法数学上基于公式:
//				X(n+1) = (a * X(n) + c) % m
//			其中,各系数为:
//				系数a, 0 < a
//				增量c, 0 <= c
//				模m, m > 0
//				原始值(种子), 0 <= X(0)
//			一般而言,高LCG的m是2的指数次幂(一般2^32或者2^64),因为这样取模操作截断最右的32或64位就可以了
//			其中参数m,a,c比较敏感,都是常数(一般会取质数),或者说直接影响了伪随机数产生的质量
//			当c=0时,叫做乘同余法,引出一个概念叫seed,它会被作为X(0)被代入上式中
//--------------------------------------------------------------------
function CreateLCG(a, c, m) {
	if ('number' != typeof(a)) {
		a = 9301;
	}
	if ('number' != typeof(c)) {
		c = 49297;
	}
	if ('number' != typeof(m)) {
		m = 233280;
	}
	if (a <= 0) {
		throw new Error("a is error");
	}
	if (c < 0) {
		throw new Error("c is error");
	}
	if (m <= 0) {
		throw new Error("m is error");
	}
	var _seed = 0;
	var lcg = {};
	// private methods
	function seededRandom() {
		_seed = (_seed*a + c)%m;
		return _seed/m;
	}
	// public methods
	lcg.seed = function(seed) {
		if ('number' != typeof(seed) || seed < 0) {
			throw new Error("seed is error");
		}
		_seed = seed;
	};
	lcg.random = function(min, max) {
		if ('number' != typeof(min)) {
			min = 0;
		}
		if ('number' != typeof(max)) {
			max = 1;
		}
		return min + seededRandom()*(max - min);
	};
	return lcg;
}
//--------------------------------------------------------------------
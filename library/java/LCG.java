package com.jaronho.sdk.library;

/**
 * Author:  jaron.ho
 * Date:    2017-04-05
 * Brief:   线性同余随机数生成器
 *          古老的LCG(linear congruential generator)代表了最好最朴素的伪随机数产生器算法
 *          主要原因是容易理解,容易实现,而且速度快
 *          LCG算法数学上基于公式:
 *              X(n+1) = (a * X(n) + c) % m
 *          其中,各系数为:
 *              系数a, 0 < a
 *              增量c, 0 <= c
 *              模m, m > 0
 *              原始值(种子), 0 <= X(0)
 *          一般而言,高LCG的m是2的指数次幂(一般2^32或者2^64),因为这样取模操作截断最右的32或64位就可以了
 *          其中参数m,a,c比较敏感,都是常数(一般会取质数),或者说直接影响了伪随机数产生的质量
 *          当c=0时,叫做乘同余法,引出一个概念叫seed,它会被作为X(0)被代入上式中
 */

public class LCG {
    private long mA = 9301;
    private long mC = 49297;
    private long mM = 233280;
    private double mSeed = 0;

    public LCG(long a, long c, long m) {
        if (a <= 0) {
            throw new AssertionError("a is error");
        }
        if (c < 0) {
            throw new AssertionError("c is error");
        }
        if (m <= 0) {
            throw new AssertionError("m is error");
        }
        mA = a;
        mC = c;
        mM = m;
    }

    public LCG() {}

    private double seededRandom() {
        mSeed = (mSeed * mA + mC) % mM;
        return mSeed / mM;
    }

    public void seed(double seed) {
        if (seed < 0) {
            throw new AssertionError("seed is error");
        }
        mSeed = seed;
    }

    public double random(double min, double max) {
        return min + seededRandom() * (max - min);
    }

    public double random() {
        return random(0, 1);
    }
}

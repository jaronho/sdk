package com.jaronho.sdk.library;

import java.util.ArrayList;
import java.util.List;

/**
 * Author:  jaron.ho
 * Date:    2017-05-09
 * Brief:   Probability
 */

public class Probability {
    private List<int[]> mWeightListInit = new ArrayList<>();
    private List<int[]> mWeightList = null;
    private int mThreshold = 1;
    private List<int[]> mWeightRange = null;
    private RandomHandler mRandomHandler = null;

    public abstract static class RandomHandler {
        public abstract double random();
    }

    // weightList e.g.[[1,20],[2,60]],contains two random value: 1.value,20.weight;2.value,60.weight
    public Probability(List<int[]> weightList, RandomHandler randomHandler) {
        for (int i = 0, len = weightList.size(); i < len; ++i) {
            int[] factor = weightList.get(i);   // [0]=value,[1]weight
            if (factor.length < 2) {
                throw new AssertionError("weight list at index [" + i + "] format is error, factor must contain two value");
            }
            if (factor[1] < 0) {
                throw new AssertionError("weight list at index [" + i + "] format is error, factor[1] must >= 0, now is " + factor[1]);
            }
            mWeightListInit.add(new int[]{factor[0], factor[1]});
        }
        mWeightList = weightList;
        mRandomHandler = randomHandler;
        parseWeightList();
    }

    public Probability(List<int[]> weightList) {
        this(weightList, null);
    }

    public Probability() {
        this(new ArrayList<int[]>(), null);
    }

    // 获取随机值
    private int random(int min, int max) {
        double randomValue = null == mRandomHandler ? Math.random() : mRandomHandler.random();
        return (int)(randomValue * (max - min) + min + 0.5f);
    }

    // 解析权重列表
    private void parseWeightList() {
        for (int index = 0, count = mWeightList.size(); index < count; ++index) {
            int randIndex = random(0, index);
            if (randIndex != index) {
                int[] temp = mWeightList.get(randIndex);
                mWeightList.set(randIndex, mWeightList.get(index));
                mWeightList.set(index, temp);
            }
        }
        mThreshold = 1;
        mWeightRange = new ArrayList<>();
        for (int i = 0, len = mWeightList.size(); i < len; ++i) {
            int[] factor = mWeightList.get(i);
            int value = factor[0];
            int weight = factor[1];
            mWeightRange.add(new int[]{value, mThreshold, mThreshold + weight - 1});    // [0].value,[1].begin,[2].end
            mThreshold += weight;
        }
    }

    /**
     * 功  能: 根据权重获取随机值
     * 参  数: 无
     * 返回值: int
     */
    public int getValue() {
        if (1 == mThreshold) {
            return -1;
        }
        int index = random(1, mThreshold - 1);
        for (int i = 0, len = mWeightRange.size(); i < len; ++i) {
            int[] range = mWeightRange.get(i);
            if (index >= range[1] && index <= range[2]) {
                return range[0];
            }
        }
        return -1;
    }

    /**
     * 功  能: 获取值的权重
     * 参  数: value - 值
     * 返回值: int
     */
    public int getWeight(int value) {
        for (int i = 0, len = mWeightList.size(); i < len; ++i) {
            int[] factor = mWeightList.get(i);
            if (value == factor[0]) {
                return factor[1];
            }
        }
        return 0;
    }

    /**
     * 功  能: 设置权重
     * 参  数: value - 值
     *         weight - 权重(必须>=0)
     * 返回值: 无
     */
    public void setWeight(int value, int weight) {
        if (weight < 0) {
            throw new AssertionError("not support weight value " + weight + " < 0");
        }
        boolean isFind = false;
        for (int i = 0, len = mWeightList.size(); i < len; ++i) {
            int[] factor = mWeightList.get(i);
            if (value == factor[0]) {
                factor[1] = weight;
                mWeightList.set(i, factor);
                isFind = true;
                break;
            }
        }
        if (!isFind) {
            mWeightList.add(new int[]{value, weight});
        }
        parseWeightList();
    }

    /**
     * 功  能: 获取总权重
     * 参  数: 无
     * 返回值: int
     */
    public int getTotalWeight() {
        int totalWeight = 0;
        for (int i = 0, len = mWeightList.size(); i < len; ++i) {
            totalWeight += mWeightList.get(i)[1];
        }
        return totalWeight;
    }

    /**
     * 功  能: 重置
     * 参  数: 无
     * 返回值: 无
     */
    public void reset() {
        mWeightList = new ArrayList<>();
        for (int i = 0, len = mWeightListInit.size(); i < len; ++i) {
            int[] factor = mWeightListInit.get(i);
            mWeightList.add(new int[]{factor[0], factor[1]});
        }
        parseWeightList();
    }
}

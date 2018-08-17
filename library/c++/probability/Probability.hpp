/**********************************************************************
* Author:	jaron.ho
* Date:		2018-07-26
* Brief:	probability
**********************************************************************/
#ifndef _PROBABILITY_HPP_
#define _PROBABILITY_HPP_

#include <vector>
#include <functional>

template<class T> class Probability {
public:
    /* 权重因子 */
    class Factor {
    public:
        Factor(void) {}
        Factor(T value, unsigned int weight) {
            this->value = value;
            this->weight = weight;
        }
        T value;                /* 值 */
        unsigned int weight;    /* 权重 */
    };

private:
    class Range {
    public:
        Range(T value, unsigned int begin, unsigned int end) {
            this->value = value;
            this->begin = begin;
            this->end = end;
        }
        T value;
        unsigned int begin;
        unsigned int end;
    };

public:
    /*
     * Brief:	构造函数
     * Param:	weightList - 权重列表
     *          randomFunc - 随机函数
     * Return:
     */
    Probability(const std::vector<Factor>& weightList, std::function<unsigned int(unsigned int min, unsigned int max)> randomFunc = nullptr) {
        static bool seedFlag = false;
        if (!seedFlag && !randomFunc) {
            seedFlag = true;
            srand((unsigned int)time(NULL));
        }
        for (size_t i = 0, len = weightList.size(); i < len; ++i) {
            Factor factor = weightList[i];
            mWeightListInit.push_back(Factor(factor.value, factor.weight));
            mWeightList.push_back(Factor(factor.value, factor.weight));
        }
        mThreshold = 1;
        mRandomFunc = randomFunc;
        parseWeightList();
    }

    /*
     * Brief:	获取随机值
     * Param:	void
     * Return:  T
     */
    T getValue(void) {
        if (1 == mThreshold) {
            return nullptr;
        }
        unsigned int index = random(1, mThreshold - 1);
        for (size_t i = 0, len = mRangeList.size(); i < len; ++i) {
            Range range = mRangeList[i];
            if (index >= range.begin && index <= range.end) {
                return range.value;
            }
        }
        return nullptr;
    }

    /*
     * Brief:	获取权重值
     * Param:	value - 值
     * Return:  unsigned int
     */
    unsigned int getWeight(T value) {
        for (size_t i = 0, len = mWeightList.size(); i < len; ++i) {
            Factor factor = mWeightList[i];
            if (value == factor.value) {
                return factor.weight;
            }
        }
        return 0;
    }

    /*
     * Brief:	设置权重值
     * Param:	value - 值
     *          weight - 权重值
     * Return:  void
     */
    void setWeight(T value, unsigned int weight) {
        bool isFind = false;
        for (size_t i = 0, len = mWeightList.size(); i < len; ++i) {
            Factor factor = mWeightList[i];
            if (value == factor.value) {
                factor.weight = weight;
                mWeightList[i] = factor;
                isFind = true;
                break;
            }
        }
        if (!isFind) {
            mWeightList.push_back(Factor(value, weight));
        }
        parseWeightList();
    }

    /*
     * Brief:	获取总权重值
     * Param:	void
     * Return:  unsigned int
     */
    unsigned int getTotalWeight(void) {
        unsigned int totalWeight = 0;
        for (size_t i = 0, len = mWeightList.size(); i < len; ++i) {
            totalWeight += mWeightList[i].weight;
        }
        return totalWeight;
    }

    /*
     * Brief:	重置
     * Param:	void
     * Return:  void
     */
    void reset(void) {
        mWeightList.clear();
        for (size_t i = 0, len = mWeightListInit.size(); i < len; ++i) {
            Factor factor = mWeightListInit[i];
            mWeightList.push_back(Factor(factor.value, factor.weight));
        }
        parseWeightList();
    }

private:
    unsigned int random(unsigned int min, unsigned int max) {
        if (min > max) {
            unsigned int temp = min;
            min = max;
            max = temp;
        }
        if (mRandomFunc) {
            return mRandomFunc(min, max);
        }
        return rand() % (max - min + 1) + min;
    }

    void parseWeightList(void) {
        for (size_t i = 0, len = mWeightList.size(); i < len; ++i) {
            unsigned int randIndex = random(0, i);
            if (randIndex != i) {
                Factor temp = mWeightList[randIndex];
                mWeightList[randIndex] = mWeightList[i];
                mWeightList[i] = temp;
            }
        }
        mThreshold = 1;
        mRangeList.clear();
        for (size_t j = 0, l = mWeightList.size(); j < l; ++j) {
            Factor factor = mWeightList[j];
            mRangeList.push_back(Range(factor.value, mThreshold, mThreshold + factor.weight - 1));
            mThreshold += factor.weight;
        }
    }

private:
    std::vector<Factor> mWeightListInit;
    std::vector<Factor> mWeightList;
    unsigned int mThreshold;
    std::vector<Range> mRangeList;
    std::function<unsigned int(unsigned int min, unsigned int max)> mRandomFunc;
};

#endif	// _PROBABILITY_HPP_

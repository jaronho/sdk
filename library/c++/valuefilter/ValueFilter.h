/**********************************************************************
 * Author:	jaron.ho
 * Date:    2019-07-19
 * Brief:	值过滤
 **********************************************************************/
#ifndef _VALUE_FILTER_H_
#define _VALUE_FILTER_H_

#include <functional>

template <typename T>
#define VALUE_FILTER_COMPARE_CALLBACK std::function<bool(T a, T b)>
class ValueFilter {
public:
    ValueFilter(void) {
        mMaxNum = 1;
        mCurNum = 1;
        mCompareCallback = nullptr;
    }
    
    void init(T value, int maxNum = 1, VALUE_FILTER_COMPARE_CALLBACK callback = nullptr) {
        mOldValue = value;
        mPreValue = value;
        mMaxNum = maxNum > 0 ? maxNum : 1;
        mCurNum = 1;
        mCompareCallback = callback;
    }
    
    void setCompareCallback(VALUE_FILTER_COMPARE_CALLBACK callback) {
        mCompareCallback = callback;
    }
    
    bool set(T nowValue) {
        bool preEqualNow = false;
        if (mCompareCallback) {
            preEqualNow = mCompareCallback(mPreValue, nowValue);
        } else {
            preEqualNow = (mPreValue == nowValue);
        }
        if (preEqualNow) {
            ++mCurNum;
        } else {
            mCurNum = 1;
        }
        mPreValue = nowValue;
        if (mCurNum >= mMaxNum) {
            mCurNum = 1;
            bool oldEqualNow = false;
            if (mCompareCallback) {
                oldEqualNow = mCompareCallback(mOldValue, nowValue);
            } else {
                oldEqualNow = (mOldValue == nowValue);
            }
            if (oldEqualNow) {
                return false;
            }
            mOldValue = nowValue;
            return true;
        }
        return false;
    }
    
    T get(void) {
        return mOldValue;
    }
    
private:
    T mOldValue;
    T mPreValue;
    int mMaxNum;
    int mCurNum;
    VALUE_FILTER_COMPARE_CALLBACK mCompareCallback;
};

#endif	/* _VALUE_FILTER_H_ */

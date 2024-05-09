#pragma once
#include <functional>
#include <map>
#include <stdexcept>
#include <vector>

namespace algorithm
{
/**
 * @brief 混合类
 */
class Misc
{
public:
    /**
     * @brief 比较容器
     * @param aVec 容器1
     * @param bVec 容器2
     * @param func 比较函数, 参数: a-值1, b-值2, 返回值: true-相同, false-不相同
     * @param ordinal 是否要顺序一致
     * @return true-相同, false-不相同
     */
    template<typename T>
    static bool compareVector(const std::vector<T>& aVec, const std::vector<T>& bVec, std::function<bool(const T& a, const T& b)> func,
                              bool ordinal = true)
    {
        if (!func)
        {
            throw std::logic_error(std::string("[") + __FILE__ + " " + std::to_string(__LINE__) + " " + __FUNCTION__
                                   + "] arg 'func' is empty");
        }
        if (aVec.size() != bVec.size())
        {
            return false;
        }
        if (ordinal) /* 顺序要一致 */
        {
            for (size_t i = 0; i < aVec.size(); ++i)
            {
                if (!func(aVec[i], bVec[i]))
                {
                    return false;
                }
            }
        }
        else
        {
            std::map<int, int> bFlag;
            for (size_t i = 0; i < aVec.size(); ++i)
            {
                bool foundFlag = false;
                for (size_t n = 0; n < bVec.size(); ++n)
                {
                    if (bFlag.end() == bFlag.find(n) && func(aVec[i], bVec[n]))
                    {
                        bFlag[n] = 1;
                        foundFlag = true;
                        break;
                    }
                }
                if (!foundFlag)
                {
                    return false;
                }
            }
        }
        return true;
    }

    /**
     * @brief 比较映射表
     * @param aMap 表1
     * @param bMap 表2
     * @param func 比较函数, 参数: a-值1, b-值2, 返回值: true-相同, false-不相同
     * @return true-相同, false-不相同
     */
    template<typename KT, typename KV>
    static bool compareMap(const std::map<KT, KV>& aMap, const std::map<KT, KV>& bMap, std::function<bool(const KV& a, const KV& b)> func)
    {
        if (!func)
        {
            throw std::logic_error(std::string("[") + __FILE__ + " " + std::to_string(__LINE__) + " " + __FUNCTION__
                                   + "] arg 'func' is empty");
        }
        if (aMap.size() != bMap.size())
        {
            return false;
        }
        for (const auto& kv : aMap)
        {
            auto iter = bMap.find(kv.first);
            if (bMap.end() == iter || !func(kv.second, iter->second))
            {
                return false;
            }
        }
        for (const auto& kv : bMap)
        {
            auto iter = aMap.find(kv.first);
            if (aMap.end() == iter || !func(kv.second, iter->second))
            {
                return false;
            }
        }
        return true;
    }
};
} // namespace algorithm

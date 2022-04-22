#pragma once
#include <string>
#include <vector>

namespace utility
{
class Util final
{
public:
    /**
     * @brief 从数组中获取指定数据
     * @param arr 数组
     * @param beginIndex 开始索引(从0开始)
     * @param num 要获取的个数
     * @return 数据
     */
    template<typename T>
    static std::vector<T> getArray(const std::vector<T>& arr, size_t beginIndex, size_t num)
    {
        std::vector<T> vec;
        size_t len = arr.size();
        if (0 == len || beginIndex < 0 || beginIndex >= len)
        {
            return vec;
        }
        size_t endIndex = beginIndex + num;
        if (endIndex >= len)
        {
            endIndex = len;
        }
        for (size_t i = beginIndex; i < endIndex; ++i)
        {
            vec.emplace_back(arr[i]);
        }
        return vec;
    }

    /**
     * @brief 获取数组分页数, 例如: [1,2,3,4,5,6,7,8,9], 每页3个则总页数为3
     * @param arr 数组
     * @param numPerPage 每页个数
     * @return 总页数
     */
    template<typename T>
    static size_t getPageCount(const std::vector<T>& arr, size_t numPerPage)
    {
        if (0 == numPerPage)
        {
            return 0;
        }
        size_t len = arr.size();
        if (0 == len)
        {
            return 0;
        }
        size_t div = len / numPerPage;
        size_t rem = len % numPerPage;
        return 0 == rem ? div : div + 1;
    }

    /**
     * @brief 获取数组分页数据, 例如: [1,2,3,4,5,6,7,8,9], 每页3个则第2页为[4,5,6]
     * @param arr 数组
     * @param numPerPage 每页个数
     * @param pageIndex 要获取的分页索引值(从0开始)
     * @return 分页数据
     */
    template<typename T>
    static std::vector<T> getPage(const std::vector<T>& arr, size_t numPerPage, size_t pageIndex)
    {
        std::vector<T> vec;
        size_t pageCount = getPageCount<T>(arr, numPerPage);
        if (0 == pageCount || pageIndex < 0 || pageIndex >= pageCount)
        {
            return vec;
        }
        size_t len = arr.size();
        size_t beginIndex = numPerPage * pageIndex;
        if (len < numPerPage)
        {
            beginIndex = 0;
        }
        size_t endIndex = beginIndex + numPerPage;
        if (endIndex > len)
        {
            endIndex = len;
        }
        for (size_t i = beginIndex; i < endIndex; ++i)
        {
            vec.push_back(arr[i]);
        }
        return vec;
    }

    /**
     * @brief URL编码, URL中传递带%的参数时，那么%就需要进行转义或编码以防止解析URL时造成歧义
     * @param srcUrl 要编码的URL
     * @return 编码后的URL
     */
    static std::string urlEncode(const std::string& srcUrl);

    /**
     * @brief URL解码, URL中传递带%的参数时，那么%就需要进行转义或编码以防止解析URL时造成歧义
     * @param srcUrl 要解码的URL
     * @return 解码后的URL
     */
    static std::string urlDecode(const std::string& srcUrl);
};
} // namespace utility

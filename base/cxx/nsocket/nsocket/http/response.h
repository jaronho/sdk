#pragma once
#include <functional>
#include <string>
#include <unordered_map>

#include "multimap.hpp"
#include "status_code.h"

namespace nsocket
{
namespace http
{
/**
 * @brief HTTP��Ӧ
 */
class Response
{
public:
    /**
     * @brief ������Ӧ����
     * @param data [���]��Ӧ����
     */
    void create(std::vector<unsigned char>& data);

public:
    std::string version = "HTTP/1.1"; /* �汾 */
    StatusCode statusCode = StatusCode::success_ok; /* ״̬�� */
    CaseInsensitiveMultimap headers; /* ͷ�� */
};
using RESPONSE_PTR = std::shared_ptr<Response>;
} // namespace http
} // namespace nsocket

#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace usb
{
/**
 * @brief USB类
 */
class Usb
{
public:
    Usb();

    virtual ~Usb() = default;

    /**
     * @brief 获取系统中USB设备列表
     * @return USB设备列表
     */
    static std::vector<Usb> getAllUsbs();

private:
};
} // namespace usb

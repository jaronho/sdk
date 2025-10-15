#include <algorithm>
#include <chrono>
#include <stdio.h>
#include <string.h>
#include <sys/timeb.h>
#include <thread>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "../usb/usb.h"

static std::vector<std::shared_ptr<usb::Usb>> s_usbList; /* USB设备列表缓存 */

std::string getDateTime()
{
    struct tm t;
    time_t now;
    time(&now);
#ifdef _WIN32
    localtime_s(&t, &now);
#else
    t = *localtime(&now);
#endif
    char buf1[20] = {0};
    strftime(buf1, sizeof(buf1), "%Y-%m-%d %H:%M:%S", &t);
    char buf2[4] = {0};
    struct timeb tb;
    ftime(&tb);
#ifdef _WIN32
    sprintf_s(buf2, sizeof(buf2), "%03d", tb.millitm);
#else
    sprintf(buf2, "%03d", tb.millitm);
#endif
    return std::string(buf1).append(".").append(buf2);
}

/**
 * @brief 显示USB设备列表
 * @param usbList USB设备列表
 * @param treeFlag 是否树形显示
 */
void displayUsbList(const std::vector<std::shared_ptr<usb::Usb>>& usbList, bool treeFlag)
{
    bool firstLine = true;
    std::string usbListJson;
    usbListJson += "[";
    for (size_t i = 0; i < usbList.size(); ++i)
    {
        const auto& info = usbList[i];
        if (treeFlag)
        {
            if (!info->getParent())
            {
                if (firstLine)
                {
                    firstLine = false;
                }
                else
                {
                    usbListJson += ",";
                }
                usbListJson += "\n";
                usbListJson += info->describe(true, 0, 4);
            }
        }
        else
        {
            if (firstLine)
            {
                firstLine = false;
            }
            else
            {
                usbListJson += ",";
            }
            usbListJson += "\n";
            usbListJson += info->describe(false, 0, 4);
        }
    }
    if (!firstLine)
    {
        usbListJson += "\n";
    }
    usbListJson += "]";
    printf("%s", usbListJson.c_str());
}

/**
 * @brief 比较USB设备列表差异
 * @param preList 之前的USB设备列表
 * @param nowList 当前的USB设备列表
 * @param addedList 新增的USB设备列表
 * @param updatedList 更新的USB设备列表
 * @param removedList 移除的USB设备列表
 */
void diffUsbList(const std::vector<std::shared_ptr<usb::Usb>>& preList, const std::vector<std::shared_ptr<usb::Usb>>& nowList,
                 std::vector<std::shared_ptr<usb::Usb>>& addedList, std::vector<std::shared_ptr<usb::Usb>>& updatedList,
                 std::vector<std::shared_ptr<usb::Usb>>& removedList)
{
    addedList.clear();
    updatedList.clear();
    removedList.clear();
    /* 计算新插入 */
    for (const auto& now : nowList)
    {
        auto iter = std::find_if(preList.begin(), preList.end(), [&](const std::shared_ptr<usb::Usb>& info) {
            return (now->getPath() == info->getPath() && now->getVid() == info->getVid() && now->getPid() == info->getPid()
                    && (now->getSerial() == info->getSerial() || now->getAddress() == info->getAddress()));
        });
        if (preList.end() == iter)
        {
            addedList.emplace_back(now);
        }
        else
        {
            const auto& preDevNodeList = (*iter)->getDevNodes();
            const auto& nowDevNodeList = now->getDevNodes();
            bool sameDevNode = true;
            if (nowDevNodeList.size() == preDevNodeList.size())
            {
                for (size_t i = 0; i < nowDevNodeList.size(); ++i)
                {
#ifdef _WIN32
                    if (nowDevNodeList[i].getMountpoint() != preDevNodeList[i].getMountpoint())

#else
                    if (nowDevNodeList[i].name != preDevNodeList[i].name)
#endif
                    {
                        sameDevNode = false;
                    }
                }
            }
            else
            {
                sameDevNode = false;
            }
            if (!sameDevNode)
            {
                updatedList.emplace_back(now);
            }
        }
    }
    /* 计算已拔出 */
    for (const auto& info : preList)
    {
        auto iter = std::find_if(nowList.begin(), nowList.end(),
                                 [&](const std::shared_ptr<usb::Usb>& now) { return (now->getPath() == info->getPath()); });
        if (nowList.end() == iter)
        {
            removedList.emplace_back(info);
        }
    }
}

#ifdef _WIN32
bool isRunAsAdmin()
{
    BOOL isAdmin = FALSE;
    PSID administratorsGroup = nullptr;
    /* 创建"Administrators"组的SID */
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
                                 &administratorsGroup))
    {
        CheckTokenMembership(NULL, administratorsGroup, &isAdmin); /* 检查当前线程是否属于该组 */
        FreeSid(administratorsGroup);
    }
    return (TRUE == isAdmin);
}
#endif

/**
 * USB设备查看工具
 * 用法: ./usb_watch [选项]
 * 选项: -h 帮助信息
 *       -t 是否树形显示
 *       -m 是否实时监听拔插
 * 例如: ./usb_watch -t
 */
int main(int argc, char** argv)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    if (!isRunAsAdmin())
    {
        printf("请使用管理员权限运行该程序\n");
        return 0;
    }
#endif
    /* 参数值 */
    bool treeFlag = false;
    bool monitorFlag = false;
    /* 解析参数 */
    for (int i = 1; i < argc;)
    {
        std::string key = argv[i];
        if (0 == key.compare("-h")) /* 帮助 */
        {
            printf("USB设备查看工具\n");
            printf("用法: ./usb_watch [选项]\n");
            printf("选项: -h    帮助信息\n");
            printf("      -t    是否树形显示(使用 -m 选项时无法树形显示)\n");
            printf("      -m    是否实时监听拔插\n");
            return 0;
        }
        else if (0 == key.compare("-t")) /* 树形 */
        {
            treeFlag = true;
            i += 1;
            continue;
        }
        else if (0 == key.compare("-m")) /* 实时监听 */
        {
            monitorFlag = true;
            i += 1;
            continue;
        }
    }
    s_usbList = usb::Usb::getAllUsbs(true);
    if (monitorFlag)
    {
        std::vector<std::shared_ptr<usb::Usb>> addedUsbList, updatedList, removedUsbList;
        while (1)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            auto nowUsbList = usb::Usb::getAllUsbs(true);
            diffUsbList(s_usbList, nowUsbList, addedUsbList, updatedList, removedUsbList);
            s_usbList = nowUsbList;
            if (!addedUsbList.empty())
            {
                printf("[%s] ++++++++++++++++++++ USB设备插入 ++++++++++++++++++++\n", getDateTime().c_str());
                displayUsbList(addedUsbList, false);
                printf("\n");
            }
            if (!updatedList.empty())
            {
                printf("[%s] ++++++++++++++++++++ USB设备更新 ++++++++++++++++++++\n", getDateTime().c_str());
                displayUsbList(updatedList, false);
                printf("\n");
            }
            if (!removedUsbList.empty())
            {
                printf("[%s] -------------------- USB设备拔出 --------------------\n", getDateTime().c_str());
                displayUsbList(removedUsbList, false);
                printf("\n");
            }
        }
    }
    else
    {
        displayUsbList(s_usbList, treeFlag);
    }
    return 0;
}

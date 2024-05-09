#include <Windows.h>
//
#include <Dbt.h>
#include <stdio.h>
#include <thread>

#include "usb/usb.h"

#define THRD_MESSAGE_EXIT WM_USER + 1
static const TCHAR CLASS_NAME[] = "Example Win Detect";
static HWND s_hwnd = NULL;
static std::vector<std::shared_ptr<usb::Usb>> s_usbList;

void syncUsbList(const std::vector<std::shared_ptr<usb::Usb>>& nowList, std::vector<std::shared_ptr<usb::Usb>>* addedList,
                 std::vector<std::shared_ptr<usb::Usb>>* removedList)
{
    /* 计算新插入 */
    if (addedList)
    {
        addedList->clear();
        for (const auto& now : nowList)
        {
            bool addedFlag = true;
            for (const auto& info : s_usbList)
            {
                if (now->getAddress() == info->getAddress())
                {
                    addedFlag = false;
                    break;
                }
            }
            if (addedFlag)
            {
                addedList->emplace_back(now);
            }
        }
    }
    /* 计算已拔出 */
    if (removedList)
    {
        removedList->clear();
        for (const auto& info : s_usbList)
        {
            bool removedFlag = true;
            for (const auto& now : nowList)
            {
                if (info->getAddress() == now->getAddress())
                {
                    removedFlag = false;
                    break;
                }
            }
            if (removedFlag)
            {
                removedList->emplace_back(info);
            }
        }
    }
    /* 保存最新 */
    s_usbList = nowList;
}

void handleDeviceArrived()
{
    auto nowList = usb::Usb::getAllUsbs(true);
    std::vector<std::shared_ptr<usb::Usb>> addedList;
    syncUsbList(nowList, &addedList, nullptr);
    if (addedList.empty())
    {
        return;
    }
    printf("++++++++++++++++++++ Usb Plugin ++++++++++++++++++++\n");
    for (size_t i = 0; i < addedList.size(); ++i)
    {
        auto item = addedList[i];
        auto devNodes = item->getDevNodes();
        std::string storageVolumes;
        for (size_t n = 0; n < devNodes.size(); ++n)
        {
            auto mountpoint = devNodes[n].getMountpoint();
            if (!mountpoint.empty())
            {
                if (n > 0)
                {
                    storageVolumes += ", ";
                }
                auto label = devNodes[n].label;
                label = label.empty() ? mountpoint : label;
                storageVolumes += label + "(" + mountpoint + ") " + devNodes[n].fstype;
            }
        }
        printf("[%02d] busNum: %d, portNum: %d, address: %d\n     class: %d, classDesc: %s, subClass: %d, protocol: %d\n     vid: %s, pid: "
               "%s, serial: %s, product: %s, manufacturer: %s, model: %s, vendor: %s, group: %s%s\n",
               (i + 1), item->getBusNum(), item->getPortNum(), item->getAddress(), item->getClassCode(), item->getClassDesc().c_str(),
               item->getSubClassCode(), item->getProtocolCode(), item->getVid().c_str(), item->getPid().c_str(), item->getSerial().c_str(),
               item->getProduct().c_str(), item->getManufacturer().c_str(), item->getModel().c_str(), item->getVendor().c_str(),
               item->getGroup().c_str(), item->isStorage() ? (", storageVolume: " + storageVolumes).c_str() : "");
    }
}

void handleDeviceRemoved()
{
    auto nowList = usb::Usb::getAllUsbs(true);
    std::vector<std::shared_ptr<usb::Usb>> removedList;
    syncUsbList(nowList, nullptr, &removedList);
    if (removedList.empty())
    {
        return;
    }
    printf("---------- Usb Pull Out ----------\n");
    for (size_t i = 0; i < removedList.size(); ++i)
    {
        auto item = removedList[i];
        auto devNodes = item->getDevNodes();
        std::string storageVolumes;
        for (size_t n = 0; n < devNodes.size(); ++n)
        {
            auto mountpoint = devNodes[n].getMountpoint();
            if (!mountpoint.empty())
            {
                if (n > 0)
                {
                    storageVolumes += ", ";
                }
                auto label = devNodes[n].label;
                label = label.empty() ? mountpoint : label;
                storageVolumes += label + "(" + mountpoint + ") " + devNodes[n].fstype;
            }
        }
        printf("[%02d] busNum: %d, portNum: %d, address: %d\n     class: %d, classDesc: %s, subClass: %d, protocol: %d\n     vid: %s, pid: "
               "%s, serial: %s, product: %s, manufacturer: %s, model: %s, vendor: %s, group: %s%s\n",
               (i + 1), item->getBusNum(), item->getPortNum(), item->getAddress(), item->getClassCode(), item->getClassDesc().c_str(),
               item->getSubClassCode(), item->getProtocolCode(), item->getVid().c_str(), item->getPid().c_str(), item->getSerial().c_str(),
               item->getProduct().c_str(), item->getManufacturer().c_str(), item->getModel().c_str(), item->getVendor().c_str(),
               item->getGroup().c_str(), item->isStorage() ? (", storageVolume: " + storageVolumes).c_str() : "");
    }
}

LRESULT CALLBACK wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DEVICECHANGE:
        if (DBT_DEVICEARRIVAL == wParam) /* 设备插入 */
        {
            handleDeviceArrived();
        }
        else if (DBT_DEVICEREMOVECOMPLETE == wParam) /* 设备拔出 */
        {
            handleDeviceRemoved();
        }
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

DWORD WINAPI threadFunc(LPVOID lpParam)
{
    /* 步骤1. 注册窗体 */
    WNDCLASS wc = {0};
    wc.lpfnWndProc = wndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    if (0 == RegisterClass(&wc))
    {
        printf("register class fail\n");
        return -1;
    }
    /* 步骤2. 创建窗体 */
    s_hwnd = CreateWindowEx(0, CLASS_NAME, "", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                            NULL, // Parent window
                            NULL, // Menu
                            GetModuleHandle(NULL), // Instance handle
                            NULL // Additional application data
    );
    if (NULL == s_hwnd)
    {
        printf("create window fail\n");
        return -1;
    }
    /* 步骤3. 注册设备通知消息 */
    if (!usb::Usb::registerDeviceNotify(s_hwnd))
    {
        printf("register device notify fail\n");
        return -1;
    }
    /* 循环监听Windows消息 */
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.message == THRD_MESSAGE_EXIT)
        {
            printf("worker receive the exiting Message\n");
            return 0;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

int main()
{
    /* 初始USB列表 */
    s_usbList = usb::Usb::getAllUsbs(true);
    /* 创建线程(控制台程序主线程无法接收Windows消息) */
    DWORD threadId;
    HANDLE threadHandle = CreateThread(NULL, 0, threadFunc, NULL, 0, &threadId);
    if (!threadHandle)
    {
        printf("create thread fail\n");
        return -1;
    }
    PostThreadMessage(threadId, THRD_MESSAGE_EXIT, 0, 0);
    WaitForSingleObject(threadHandle, INFINITE);
    CloseHandle(threadHandle);
    return 0;
}
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "utility/cmdline/cmdline.h"
#include "utility/filesystem/file_info.h"
#include "utility/strtool/strtool.h"

int main(int argc, char* argv[])
{
    SetConsoleOutputCP(CP_UTF8);
    /* 关闭控制台程序的快速编辑模式, 否则会出现点击界面, 程序将会变成阻塞状态, 不按回车无法继续运行 */
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    mode &= ~ENABLE_QUICK_EDIT_MODE; /* 移除快速编辑模式 */
    SetConsoleMode(hStdin, mode);
    /* 命令参数 */
    cmdline::parser parser;
    parser.header("文本文件转代码字符串");
    parser.add<std::string>("infile", 'i', "输入文本文件.", true);
    parser.add<int>("language", 't', "代码语言, 值: 0 - C++, 默认:", false, 0);
    parser.parse_check(argc, argv, "用法", "选项", "显示帮助信息并退出");
    printf("%s\n", parser.usage().c_str());
    /* 参数解析 */
    auto inFileName = parser.get<std::string>("infile");
    auto language = parser.get<int>("language");
    utility::FileInfo inFi(inFileName);
    if (!inFi.exist())
    {
        printf("输入文件 %s 不存在\n", inFileName.c_str());
        return 0;
    }
    if (!inFi.isTextFile())
    {
        printf("输入文件 %s 非文本文件\n", inFileName.c_str());
        return 0;
    }
    std::string outContent;
    std::string suffix;
    if (0 == language) /* C++ */
    {
        suffix = "cpp";
        auto f = fopen(inFi.name().c_str(), "rb");
        if (f)
        {
            size_t lineNum = 0;
            std::string preline, line, bomFlag, endFlag;
            while (utility::FileInfo::readLine(f, line, bomFlag, endFlag))
            {
                ++lineNum;
                if (lineNum >= 2)
                {
                    outContent += "\"" + preline + "\\n\"" + "\n";
                }
                preline = utility::StrTool::replace(line, "\"", "\\\"");
            }
            fclose(f);
            if (lineNum > 0)
            {
                outContent += "\"" + preline + "\"";
            }
        }
    }
    else
    {
        printf("转换失败: 暂不支持代码类型[%d]\n", language);
        return 0;
    }
    if (outContent.empty())
    {
        printf("转换失败: 文件内容为空\n");
    }
    else
    {
        utility::FileInfo outFi(inFi.path() + inFi.basename() + "-out" + suffix + "." + inFi.extname());
        outFi.write(outContent);
        printf("转换成功, 输出文件: %s\n", outFi.name().c_str());
    }
    return 0;
}

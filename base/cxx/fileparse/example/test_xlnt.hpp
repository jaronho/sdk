#pragma once
#include <iostream>

#include "../fileparse/xlnt/include/xlnt/xlnt.hpp"

xlnt::workbook g_excel;
xlnt::worksheet g_excelSheet = g_excel.active_sheet();

/* 设置excel标题头 */
void setExcelHeader()
{
    /* 日期 */
    g_excelSheet.column_properties(xlnt::column_t("A")).width = 19.38;
    g_excelSheet.cell("A1").alignment(
        xlnt::alignment().horizontal(xlnt::horizontal_alignment::center).vertical(xlnt::vertical_alignment::center));
    g_excelSheet.cell("A1").font(xlnt::font().bold(true));
    g_excelSheet.cell("A1").value("Datetime");
    g_excelSheet.merge_cells("A1:A2");
    /* 内存 */
    g_excelSheet.column_properties(xlnt::column_t("B")).width = 14.75;
    g_excelSheet.cell("B1").alignment(
        xlnt::alignment().horizontal(xlnt::horizontal_alignment::center).vertical(xlnt::vertical_alignment::center));
    g_excelSheet.cell("B1").font(xlnt::font().bold(true));
    g_excelSheet.cell("B1").value("Memory(Mb)");
    g_excelSheet.merge_cells("B1:D1");
    /* 工作集(内存) */
    g_excelSheet.cell("B2").alignment(
        xlnt::alignment().horizontal(xlnt::horizontal_alignment::center).vertical(xlnt::vertical_alignment::center));
    g_excelSheet.cell("B2").font(xlnt::font().bold(true));
    g_excelSheet.cell("B2").value("WorkingSetSize");
    /* 内存(专用工作集) */
    g_excelSheet.column_properties(xlnt::column_t("C")).width = 14.38;
    g_excelSheet.cell("C2").alignment(
        xlnt::alignment().horizontal(xlnt::horizontal_alignment::center).vertical(xlnt::vertical_alignment::center));
    g_excelSheet.cell("C2").font(xlnt::font().bold(true));
    g_excelSheet.cell("C2").value("WorkSetPrivate");
    /* 内存(共享工作集) */
    g_excelSheet.column_properties(xlnt::column_t("D")).width = 14.5;
    g_excelSheet.cell("D2").alignment(
        xlnt::alignment().horizontal(xlnt::horizontal_alignment::center).vertical(xlnt::vertical_alignment::center));
    g_excelSheet.cell("D2").font(xlnt::font().bold(true));
    g_excelSheet.cell("D2").value("WorkSetShared");
    /* 句柄数 */
    g_excelSheet.column_properties(xlnt::column_t("E")).width = 7.88;
    g_excelSheet.cell("E1").alignment(
        xlnt::alignment().horizontal(xlnt::horizontal_alignment::center).vertical(xlnt::vertical_alignment::center));
    g_excelSheet.cell("E1").font(xlnt::font().bold(true));
    g_excelSheet.cell("E1").value("Handles");
    g_excelSheet.merge_cells("E1:E2");
    /* 线程数 */
    g_excelSheet.column_properties(xlnt::column_t("F")).width = 7.63;
    g_excelSheet.cell("F1").alignment(
        xlnt::alignment().horizontal(xlnt::horizontal_alignment::center).vertical(xlnt::vertical_alignment::center));
    g_excelSheet.cell("F1").font(xlnt::font().bold(true));
    g_excelSheet.cell("F1").value("Threads");
    g_excelSheet.merge_cells("F1:F2");
}

/* 设置excel行内容 */
void setExcelRow(unsigned int index, int workingSetSizeKb, int diffWorkingSetSizeKb, int workSetPrivateKb, int diffWorkSetPrivateKb,
                 int workSetSharedKb, int diffWorkSetSharedKb, int handes, int threads)
{
    /* 日期 */
    time_t now;
    time(&now);
    struct tm t = *localtime(&now);
    char datetime[22] = {0};
    strftime(datetime, sizeof(datetime), "%Y-%m-%d %H:%M:%S", &t);
    g_excelSheet.cell("A" + std::to_string(index))
        .alignment(xlnt::alignment().horizontal(xlnt::horizontal_alignment::center).vertical(xlnt::vertical_alignment::center));
    g_excelSheet.cell("A" + std::to_string(index)).value(datetime);
    /* 工作集(内存) */
    char workingSetSizeMbStr[32] = {0};
#if _WIN32
    sprintf_s(workingSetSizeMbStr, "%0.1f", (double)workingSetSizeKb / 1024);
#else
    sprintf(workingSetSizeMbStr, "%0.1f", (double)workingSetSizeKb / 1024);
#endif
    char workingSetSizeComment[128] = {0};
    if (3 == index || 0 == diffWorkingSetSizeKb) /* 第1行或者无差异 */
    {
#if _WIN32
        sprintf_s(workingSetSizeComment, "(%d Kb)", workingSetSizeKb);
#else
        sprintf(workingSetSizeComment, "(%d Kb)", workingSetSizeKb);
#endif
    }
    else
    {
#if _WIN32
        sprintf_s(workingSetSizeComment, "(%d Kb)\n%s%0.1f Mb\n(%d Kb)", workingSetSizeKb, (diffWorkingSetSizeKb > 0 ? "+" : "-"),
                  (double)abs(diffWorkingSetSizeKb) / 1024, abs(diffWorkingSetSizeKb));
#else
        sprintf(workingSetSizeComment, "(%d Kb)\n%s%0.1f Mb\n(%d Kb)", workingSetSizeKb, (diffWorkingSetSizeKb > 0 ? "+" : "-"),
                (double)abs(diffWorkingSetSizeKb) / 1024, abs(diffWorkingSetSizeKb));
#endif
    }
    g_excelSheet.cell("B" + std::to_string(index))
        .alignment(xlnt::alignment().horizontal(xlnt::horizontal_alignment::center).vertical(xlnt::vertical_alignment::center));
    g_excelSheet.cell("B" + std::to_string(index)).number_format(xlnt::number_format::general());
    g_excelSheet.cell("B" + std::to_string(index)).value(std::atof(workingSetSizeMbStr));
    g_excelSheet.cell("B" + std::to_string(index)).comment(xlnt::comment(workingSetSizeComment, ""));
    /* 内存(专用工作集) */
    char workSetPrivateMbStr[32] = {0};
#if _WIN32
    sprintf_s(workSetPrivateMbStr, "%0.1f", (double)workSetPrivateKb / 1024);
#else
    sprintf(workSetPrivateMbStr, "%0.1f", (double)workSetPrivateKb / 1024);
#endif
    char workSetPrivateComment[128] = {0};
    if (3 == index || 0 == diffWorkSetPrivateKb) /* 第1行或者无差异 */
    {
#if _WIN32
        sprintf_s(workSetPrivateComment, "(%d Kb)", workSetPrivateKb);
#else
        sprintf(workSetPrivateComment, "(%d Kb)", workSetPrivateKb);
#endif
    }
    else
    {
#if _WIN32
        sprintf_s(workSetPrivateComment, "(%d Kb)\n%s%0.1f Mb\n(%d Kb)", workSetPrivateKb, (diffWorkSetPrivateKb > 0 ? "+" : "-"),
                  (double)abs(diffWorkSetPrivateKb) / 1024, abs(diffWorkSetPrivateKb));
#else
        sprintf(workSetPrivateComment, "(%d Kb)\n%s%0.1f Mb\n(%d Kb)", workSetPrivateKb, (diffWorkSetPrivateKb > 0 ? "+" : "-"),
                (double)abs(diffWorkSetPrivateKb) / 1024, abs(diffWorkSetPrivateKb));
#endif
    }
    g_excelSheet.cell("C" + std::to_string(index))
        .alignment(xlnt::alignment().horizontal(xlnt::horizontal_alignment::center).vertical(xlnt::vertical_alignment::center));
    g_excelSheet.cell("C" + std::to_string(index)).number_format(xlnt::number_format::general());
    g_excelSheet.cell("C" + std::to_string(index)).value(std::atof(workSetPrivateMbStr));
    g_excelSheet.cell("C" + std::to_string(index)).comment(xlnt::comment(workSetPrivateComment, ""));
    /* 内存(共享工作集) */
    char workSetSharedMbStr[32] = {0};
#if _WIN32
    sprintf_s(workSetSharedMbStr, "%0.1f", (double)workSetSharedKb / 1024);
#else
    sprintf(workSetSharedMbStr, "%0.1f", (double)workSetSharedKb / 1024);
#endif
    char workSetSharedComment[128] = {0};
    if (3 == index || 0 == diffWorkSetSharedKb) /* 第1行或者无差异 */
    {
#if _WIN32
        sprintf_s(workSetSharedComment, "(%d Kb)", workSetSharedKb);
#else
        sprintf(workSetSharedComment, "(%d Kb)", workSetSharedKb);
#endif
    }
    else
    {
#if _WIN32
        sprintf_s(workSetSharedComment, "(%d Kb)\n%s%0.1f Mb\n(%d Kb)", workSetSharedKb, (diffWorkSetSharedKb > 0 ? "+" : "-"),
                  (double)abs(diffWorkSetSharedKb) / 1024, abs(diffWorkSetSharedKb));
#else
        sprintf(workSetSharedComment, "(%d Kb)\n%s%0.1f Mb\n(%d Kb)", workSetSharedKb, (diffWorkSetSharedKb > 0 ? "+" : "-"),
                (double)abs(diffWorkSetSharedKb) / 1024, abs(diffWorkSetSharedKb));
#endif
    }
    g_excelSheet.cell("D" + std::to_string(index))
        .alignment(xlnt::alignment().horizontal(xlnt::horizontal_alignment::center).vertical(xlnt::vertical_alignment::center));
    g_excelSheet.cell("D" + std::to_string(index)).number_format(xlnt::number_format::general());
    g_excelSheet.cell("D" + std::to_string(index)).value(std::atof(workSetSharedMbStr));
    g_excelSheet.cell("D" + std::to_string(index)).comment(xlnt::comment(workSetSharedComment, ""));
    /* 句柄数 */
    g_excelSheet.cell("E" + std::to_string(index))
        .alignment(xlnt::alignment().horizontal(xlnt::horizontal_alignment::center).vertical(xlnt::vertical_alignment::center));
    g_excelSheet.cell("E" + std::to_string(index)).value(handes);
    /* 线程数 */
    g_excelSheet.cell("F" + std::to_string(index))
        .alignment(xlnt::alignment().horizontal(xlnt::horizontal_alignment::center).vertical(xlnt::vertical_alignment::center));
    g_excelSheet.cell("F" + std::to_string(index)).value(threads);
}

/* 设置excel标记 */
void setExcelTag(unsigned int index, const std::string& tag)
{
    /* 标记拼接 */
    std::string newTag = g_excelSheet.cell("G" + std::to_string(index)).to_string();
    newTag += (newTag.empty() ? "" : ",") + tag;
    g_excelSheet.cell("G" + std::to_string(index))
        .alignment(xlnt::alignment().horizontal(xlnt::horizontal_alignment::center).vertical(xlnt::vertical_alignment::center));
    g_excelSheet.cell("G" + std::to_string(index))
        .fill(xlnt::fill::solid(xlnt::color(xlnt::rgb_color(255, 192, 0)))); /* 标记单元格用颜色填充 */
    g_excelSheet.cell("G" + std::to_string(index)).value(newTag);
}

void testXlnt()
{
    printf("\n============================== test xlnt =============================\n");
    setExcelHeader();
    size_t index = 2; /* 由于标题占用了2行, 所以这里需要从第3行开始 */
    for (size_t i = 0; i < 10; ++i)
    {
        ++index;
        size_t sz = i * 1024;
        setExcelRow(index, sz, sz, sz, sz, sz, sz, i, i);
    }
    setExcelTag(index, "MyTag");
    g_excel.save("test1.xlsx");
}

cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_CXX_STANDARD 14) # `std::make_unique`要求最低C++14

include(cmake/tools.cmake)

#########################################[ 依赖 ]#########################################
#[[
[宏定义]
add_definitions(-DXLNT_STATIC)
]]

#########################################[ 输出 ]#########################################
#[[
[源目录]
${base_fileparse_dir}

[源文件]
${base_fileparse_files}

[源文件]
${base_fileparse_ini_files}

[源文件]
${base_fileparse_nlohmannjson_files}

[源文件]
${base_fileparse_rapidjson_files}

[源文件]
${base_fileparse_xlnt_files}
]]

##################################### 添加fileparse库 ####################################
set(base_fileparse_dir ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

get_cxx_files(fileparse/ini src_list)
set(base_fileparse_ini_files ${src_list} CACHE INTERNAL "")

get_cxx_files(fileparse/nlohmann src_list)
set(base_fileparse_nlohmannjson_files ${src_list} CACHE INTERNAL "")

set(rapidjson_src_list)
list(APPEND rapidjson_subdirs error internal msinttypes)
foreach (dir ${rapidjson_subdirs})
    get_cxx_files(fileparse/rapidjson/${dir} src_list)
    list(APPEND rapidjson_src_list ${src_list})
endforeach ()
get_cxx_files(fileparse/rapidjson src_list)
list(APPEND rapidjson_src_list ${src_list})
set(base_fileparse_rapidjson_files ${rapidjson_src_list} CACHE INTERNAL "")

set(xlnt_src_list)
list(APPEND xlnt_subdirs include/xlnt
                         include/xlnt/cell
                         include/xlnt/drawing
                         include/xlnt/packaging
                         include/xlnt/styles
                         include/xlnt/utils
                         include/xlnt/workbook
                         include/xlnt/worksheet
                         source/cell
                         source/detail
                         source/detail/cryptography
                         source/detail/external
                         source/detail/header_footer
                         source/detail/implementations
                         source/detail/number_format
                         source/detail/serialization
                         source/drawing
                         source/packaging
                         source/styles
                         source/utils
                         source/workbook
                         source/worksheet
                         third-party/libstudxml/libstudxml
                         third-party/libstudxml/libstudxml/details
                         third-party/libstudxml/libstudxml/details/expat
                         third-party/libstudxml/libstudxml/details/genx
                         third-party/miniz
                         third-party/utfcpp
                         third-party/utfcpp/utf8)
foreach (dir ${xlnt_subdirs})
    get_cxx_files(fileparse/xlnt/${dir} src_list)
    list(APPEND xlnt_src_list ${src_list})
endforeach ()
set(base_fileparse_xlnt_files ${xlnt_src_list} CACHE INTERNAL "")

# 组装文件列表
set(base_fileparse_files
    ${base_fileparse_ini_files}
    ${base_fileparse_nlohmannjson_files}
    ${base_fileparse_rapidjson_files}
    ${base_fileparse_xlnt_files}
    CACHE INTERNAL "")

# 打印文件列表
print_info(BODY "base fileparse files:")
foreach(filename ${base_fileparse_files})
    message("    " ${filename})
endforeach()

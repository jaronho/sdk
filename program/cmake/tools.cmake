###############################################################################
# @brief: 打印信息
###############################################################################
function(print_info)
    cmake_parse_arguments(INFO "" "TAG" "BODY" ${ARGN})
    string(TIMESTAMP NOW_TIME "%Y-%m-%d %H:%M:%S")
    if (INFO_TAG)
        if (INFO_BODY)
            message("[${NOW_TIME}] [${INFO_TAG}] ${INFO_BODY}")
        else()
            message("[${NOW_TIME}] [${INFO_TAG}]")
        endif()
    else()
        if (INFO_BODY)
            message("[${NOW_TIME}] ${INFO_BODY}")
        else()
            message("[${NOW_TIME}]")
        endif()
    endif()
endfunction()

###############################################################################
# @brief: 执行命令并获取输出结果, 格式: run_cmd(COMMAND <CMD_LIST> [OUTPUT_VAR <OUTPUT>] [WORKING_DIRECTORY <WORKDIR>] [ALLOW_FAIL]
# @param: COMMAND <CMD_LIST> [必填]命令
# @param: OUTPUT_VAR <OUTPUT> [输出, 选填]输出结果, OUTPUT为接收的变量名
# @param: WORKING_DIRECTORY <WORKDIR> [选填]指定工作目录
# @param: ALLOW_FAIL [选填]是否允许命令执行失败, 若包含该指令则运行失败, 若不包含则不允许命令执行失败
###############################################################################
function(run_cmd)
    cmake_parse_arguments(RUN "ALLOW_FAIL" "OUTPUT_VAR;WORKING_DIRECTORY" "COMMAND" ${ARGN})
    if(NOT RUN_WORKING_DIRECTORY)
        set(RUN_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
    execute_process(COMMAND ${RUN_COMMAND} WORKING_DIRECTORY ${RUN_WORKING_DIRECTORY} RESULT_VARIABLE CMD_RESULT OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(CMD_RESULT)
        if(RUN_ALLOW_FAIL)
          message("Command '${RUN_COMMAND}' run in '${RUN_WORKING_DIRECTORY}' failed with code '${CMD_RESULT}'\nFailed Command Info:\n${CMD_OUTPUT}")
        else()
          message(FATAL_ERROR "Command '${RUN_COMMAND}' run in '${RUN_WORKING_DIRECTORY}' failed with code '${CMD_RESULT}'\nFailed Command Info:\n${CMD_OUTPUT}")
        endif()
    endif()
    if(RUN_OUTPUT_VAR)
        set(${RUN_OUTPUT_VAR} ${CMD_OUTPUT} PARENT_SCOPE)
    endif(RUN_OUTPUT_VAR)
endfunction()

###############################################################################
# @brief: 递归复制文件夹
# @param: SRC_DIR 源目录
# @param: DEST_DIR 目标目录(当该目录不存在时, 会自动创建)
###############################################################################
function(copy_directory SRC_DIR DEST_DIR)
    if(NOT EXISTS ${SRC_DIR})
        return()
    endif()
    make_directory(${DEST_DIR})
    file(GLOB_RECURSE SRC_FILES RELATIVE ${SRC_DIR} ${SRC_DIR}/*)
    foreach(FILE ${SRC_FILES})
        set(SRC_FILE ${SRC_DIR}/${FILE})
        if(NOT IS_DIRECTORY ${SRC_FILE})
            configure_file(${SRC_FILE} ${DEST_DIR}/${FILE} COPYONLY)
        endif()
    endforeach()
endfunction()

###############################################################################
# @brief: 添加所有(带有'CMakeLists.txt'文件的)子目录(不递归)
###############################################################################
function(add_subdirectories)
    cmake_parse_arguments(SUBDIR "" "" "EXCLUDE" ${ARGN})
    file(GLOB ALL_FILES *)
    foreach(subdir ${ALL_FILES})
        # 仅添加带有'CMakeLists.txt'文件的文件夹
        if((IS_DIRECTORY ${subdir}) AND (EXISTS ${subdir}/CMakeLists.txt))
            file(RELATIVE_PATH relPath ${CMAKE_CURRENT_SOURCE_DIR} ${subdir})
            if(SUBDIR_EXCLUDE)
                list(FIND SUBDIR_EXCLUDE ${relPath} _index)
                if(NOT _index EQUAL -1)
                    if(CMAKE_DEBUG)
                        message(STATUS "Exclude [${relPath}] when add_subdirectories")
                    endif()
                    continue()
                endif()
            endif()
            add_subdirectory(${subdir})
        endif()
    endforeach()
endfunction()

###############################################################################
# @brief: 获取指定目录下的指定文件(不递归)
# @param: SEARCH_PATH 要搜索的目录
# @param: EXP 过滤表达式, 例如: *.h
# @param: SOURCE_LIST [输出]文件列表
###############################################################################
function(get_files SEARCH_PATH EXP SOURCE_LIST)
    set(SOURCE_EXP_WITH_PATH)
    foreach(EXP ${EXP_LIST})
        list(APPEND SOURCE_EXP_WITH_PATH ${SEARCH_PATH}/${EXP})
    endforeach()
    file(GLOB FILES ${SEARCH_PATH}/${EXP})
    set(${SOURCE_LIST} ${FILES} PARENT_SCOPE)
endfunction()

###############################################################################
# @brief: 获取指定目录下的所有cxx文件(不递归)
# @param: SEARCH_PATH 要搜索的目录
# @param: SOURCE_LIST [输出]文件列表
###############################################################################
function(get_cxx_files SEARCH_PATH SOURCE_LIST)
    set(SOURCE_EXP *.h *.hpp *.c *.cc *.cpp)
    set(SOURCE_EXP_WITH_PATH)
    foreach(EXP ${SOURCE_EXP})
        list(APPEND SOURCE_EXP_WITH_PATH ${SEARCH_PATH}/${EXP})
    endforeach()
    file(GLOB FILES ${SOURCE_EXP_WITH_PATH})
    set(${SOURCE_LIST} ${FILES} PARENT_SCOPE)
endfunction()

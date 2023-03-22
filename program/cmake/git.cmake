include(${CMAKE_CURRENT_LIST_DIR}/tools.cmake)

###############################################################################
# @brief: 获取目录所在分支名
# @param: WORK_DIR 目录
# @param: BRANCH_NAME [输出]分支名
###############################################################################
function(get_git_branch WORK_DIR BRANCH_NAME)
    # Get branch name
    run_cmd(OUTPUT_VAR CURRENT_BRANCH COMMAND git rev-parse --abbrev-ref HEAD WORKING_DIRECTORY ${WORK_DIR})
    if(NOT "${CURRENT_BRANCH}" STREQUAL "HEAD")
        set(${BRANCH_NAME} ${CURRENT_BRANCH} PARENT_SCOPE)
    else()
        message(STATUS "Current commit belongs to unknown branch")
    endif()
endfunction()

###############################################################################
# @brief: 获取目录所在的tag
# @param: WORK_DIR 目录
# @param: TAG_NAME [输出]tag名
###############################################################################
function(get_git_tag WORK_DIR TAG_NAME)
    # Last tag name
    run_cmd(OUTPUT_VARLAST_TAG COMMAND git describe --tags --abbrev=0 WORKING_DIRECTORY ${WORK_DIR} ALLOW_FAIL)
    # Describe current commit
    get_git_describe(CURRENT_DESC ${WORK_DIR})
    if("${LAST_TAG}" STREQUAL "${CURRENT_DESC}")
        set(${TAG_NAME} ${CURRENT_DESC} PARENT_SCOPE)
    else()
        message(STATUS "Current commit is not a tag")
    endif()
endfunction()

###############################################################################
# @brief: 获取目录commit的ID
# @param: WORK_DIR 目录
# @param: COMMIT_ID [输出]提交ID
###############################################################################
function(yl_git_commit_id WORK_DIR COMMIT_ID)
    run_cmd(OUTPUT_VAR COMMIT_SHA1 COMMAND git rev-parse HEAD WORKING_DIRECTORY ${WORK_DIR})
    string(SUBSTRING "${COMMIT_SHA1}" 0 6 SHORT_SHA1)
    set(${COMMIT_ID} ${SHORT_SHA1} PARENT_SCOPE)
endfunction(yl_git_commit_id)

###############################################################################
# @brief: 获取目录的describe
# @param: WORK_DIR 目录
# @param: DESCRIBE [输出]描述
###############################################################################
function(get_git_describe WORK_DIR DESCRIBE)
    run_cmd(OUTPUT_VAR COMMIT_DESC COMMAND git describe --tags WORKING_DIRECTORY ${WORK_DIR} ALLOW_FAIL)
    set(${DESCRIBE} ${COMMIT_DESC} PARENT_SCOPE)
endfunction()

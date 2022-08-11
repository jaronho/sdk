#pragma once

/* 网络接入控制(Network Access Control) */
namespace nac
{
/**
 * @brief 业务码
 */
enum class BizCode
{
    /*********************************************************************
     * 客户端请求(`req_`), 取值: 1xxx
     *********************************************************************/
    req_hearbeat = 1001, /* 心跳 */
    req_auth = 1002, /* 鉴权 */
    req_login = 1003, /* 登录 */
    req_loggout = 1004, /* 退出 */
    req_report_board_status = 1005, /* 上报板卡校准状态 */
    req_report_terminal_info = 1006, /* 上报终端信息 */
    req_report_audit_log = 1007, /* 上报审计日志 */
    req_report_alarm_info = 1008, /* 上报告警信息 */
    req_report_km_lock_status = 1009, /* 上报终端键鼠口锁定状态 */
    req_query_server_time = 1010, /* 查询服务器时间 */
    req_query_policy = 1011, /* 查询策略 */
    req_query_server_disk_free_space = 1012, /* 查询服务器磁盘剩余空间*/
    req_request_approve = 1013, /* 申请审批 */
    req_query_approve_process = 1014, /* 查询审批进度 */
    req_report_terminal_lock_status = 1015, /* 上报终端锁定状态 */
    req_report_log_upload_end = 1016, /* 上传日志完毕 */

    /*********************************************************************
     * 服务端通知(`notify_`), 取值: 2xxx
     *********************************************************************/
    notify_proc_upgrade = 2001, /* 终端程序版本升级 */
    notify_viruslib_upgrade = 2002, /* 病毒库版本升级 */
    notify_policy = 2003, /* 分发策略 */
    notify_net_change = 2004, /* 通知终端网络改变 */
    notify_user_freeze = 2005, /* 通知用户被冻结 */
    notify_policy_update = 2007, /* 通知策略更新 */
    notify_cmd = 2008, /* 通知终端执行命令 */
    notify_terminal_restart = 2009, /* 通知终端重启 */
    notify_reset_admin_password = 2010, /* 通知终端充值管理员密码 */
    notify_upload_log_file = 2011, /* 通知上传日志文件 */
    notify_terminal_info = 2012, /* 通知终端信息 */
    notify_alarm_audio_url = 2013, /* 通知告警音频被修改 */

};
} // namespace nac

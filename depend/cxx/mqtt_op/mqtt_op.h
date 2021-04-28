/**********************************************************************
* Author:	jaron.ho
* Date:		2020-05-07
* Brief:	MQTT通信接口封装
**********************************************************************/
#ifndef MQTT_OP_H
#define MQTT_OP_H

#ifdef __cplusplus
extern "C"
{
#endif

/* MQTT日志等级定义 */
#define MQTT_LOG_ERR			       1                                       /* 错误情况 */
#define MQTT_LOG_WARNING		       2                                       /* 警告情况 */
#define MQTT_LOG_NOTICE			       3                                       /* 一般重要情况 */
#define MQTT_LOG_INFO			       4                                       /* 普通信息 */
#define MQTT_LOG_DEBUG			       5                                       /* 调试信息 */

/* MQTT状态码 */
#define MQTT_CODE_SUCCESS              0                                       /* 成功 */
#define MQTT_CODE_NOMEM                1                                       /* 出现内存不足情况 */
#define MQTT_CODE_PROTOCOL             2                                       /* 协议方面出错 */
#define MQTT_CODE_INVAL                3                                       /* 输入参数无效 */
#define MQTT_CODE_NO_CONN              4                                       /* 未连接 */
#define MQTT_CODE_CONN_LOST            7                                       /* 与代理服务器连接丢失 */
#define MQTT_CODE_PAYLOAD_SIZE         9                                       /* 荷载长度过大 */
#define MQTT_CODE_ERRNO                14                                      /* 系统调用出错, 变量errno包含错误码 */
#define MQTT_CODE_MALFORMED_UTF8       18                                      /* 主题名不是有效的utf-8字符串 */
#define MQTT_CODE_QOS_NOT_SUPPORTED    24                                      /* QoS大于代理所支持的QoS */
#define MQTT_CODE_OVERSIZE_PACKET      25                                      /* 产生的包大于代理所支持的最大值 */

/* MQTT对象 */
typedef void* MQTT_T;

/* MQTT回调函数集合 */
typedef struct {
    /*******************************************************************
    ** 函数描述:   日志回调函数
    ** 参数:       [in] level: 日志等级, 参见"MQTT日志等级定义"
    **             [in] str: 日志信息
    ** 返回:       无
    ********************************************************************/
    void (*on_log)(int level, const char* str);

    /*******************************************************************
    ** 函数描述:   连接回调函数
    ** 参数:       [in] code: 响应码
    **                        0 - 成功
    **                        1 - 连接拒绝(不可接受的协议版本)
    **                        2 - 连接拒绝(标识符被拒绝)
    **                        3 - 连接拒绝(代理不可用)
    **                        4-255 - 预留作日后使用
    ** 返回:       无
    ********************************************************************/
    void (*on_connect)(int code);

    /*******************************************************************
    ** 函数描述:   断开回调函数
    ** 参数:       [in] code: 响应码, 如果值为0表示客户端调用断开接口, 其他值表示意外断开
    ** 返回:       无
    ********************************************************************/
    void (*on_disconnect)(int code);

    /*******************************************************************
    ** 函数描述:   订阅请求回调函数
    ** 参数:       [in] msgId: 消息id
    **             [in] qosCount: 订阅数量(grantedQoS的大小)
    **             [in] grantedQoS: 每个订阅的QoS
    ** 返回:       无
    ********************************************************************/
    void (*on_subscribe)(int msgId, int qosCount, const int* grantedQoS);

    /*******************************************************************
    ** 函数描述:   订阅取消回调函数
    ** 参数:       [in] msgId: 消息id
    ** 返回:       无
    ********************************************************************/
    void (*on_unsubscribe)(int msgId);

    /*******************************************************************
    ** 函数描述:   发布回调函数
    ** 参数:       [in] msgId: 消息id
    ** 返回:       无
    ********************************************************************/
    void (*on_publish)(int msgId);

    /*******************************************************************
    ** 函数描述:   消息回调函数
    ** 参数:       [in] msgId: 消息id
    **             [in] topic: 主题
    **             [in] data: 数据
    **             [in] dataLen: 数据长度
    **             [in] qos: 值范围[0 - 至多一次, 1 - 至少一次, 2 - 只有一次]
    **             [in] retain: 是否保留消息, 若被设置为1, 则新订阅者订阅该topic时可以收到上一条消息
    ** 返回:       无
    ********************************************************************/
    void (*on_message)(int msgId, const char* topic, const void* data, int dataLen, int qos, int retain);

} MQTT_CALLBACK_T;

/*******************************************************************
** 函数描述:   获取当前MQTT对象数量
** 参数:       无
** 返回:       MQTT对象数量
********************************************************************/
extern int mqtt_count(void);

/*******************************************************************
** 函数描述:   创建MQTT对象
** 参数:       [in] clientId: 客户端id, 如果设置为NULL则内部随机生成(为NULL时cleanSession必须设置为true)
**             [in] cleanSession: 当断开连接时是否清空所有消息和订阅
**             [in] callback: 回调函数集合
**             [in] useDefaultNewThread: 0 - 需要循环调用mqtt_loop接口, 1 - 使用默认新创建的线程处理网络消息(不能再调用mqtt_loop接口)
** 返回:       MQTT对象
********************************************************************/
extern MQTT_T mqtt_create(const char* clientId, int cleanSession, MQTT_CALLBACK_T* callback, int useDefaultNewThread);

/*******************************************************************
** 函数描述:   销毁MQTT对象
** 参数:       [in] client: MQTT对象
** 返回:       是返回1, 否返回0
********************************************************************/
extern void mqtt_destroy(MQTT_T client);

/*******************************************************************
** 函数名:     mqtt_loop
** 函数描述:   MQTT消息循环处理(在循环体中以一定时间间隔调用), 当mqtt_create接口中useDefaultNewThread参数为1时该接口无效
** 参数:       [in] client: MQTT对象
**             [in] timeout: 等待毫秒数, 0 - 立即返回, <0 - 默认1000毫秒
** 返回:       MQTT_CODE_SUCCESS
**             MQTT_CODE_INVAL
**             MQTT_CODE_NOMEM
**             MQTT_CODE_NO_CONN
**             MQTT_CODE_CONN_LOST
**             MQTT_CODE_PROTOCOL
**             MQTT_CODE_ERRNO
********************************************************************/
extern int mqtt_loop(MQTT_T client, int timeout);

/*******************************************************************
** 函数描述:   是否连接中
** 参数:       [in] client: MQTT对象
** 返回:       MQTT_CODE_SUCCESS
**             MQTT_CODE_INVAL
**             MQTT_CODE_NO_CONN
********************************************************************/
extern int mqtt_is_connected(MQTT_T client);

/*******************************************************************
** 函数描述:   配置基于SSL/TLS的证书, 注意: 必须在连接之前调用
** 参数:       [in] client: MQTT对象
**             [in] cafile: 指向包含PEM编码的受信任CA证书文件, 不能为NULL
**             [in] certfile: 指向包含此客户端PEM编码的证书文件, 如果为NULL，keyfile也必须为NULL，并且不使用客户端证书
**             [in] keyfile: 指向包含此客户端PEM编码的私钥文件, 如果为NULL，certfile也必须为NULL，并且不使用客户端证书
**             [in] pw_callback: 如果keyfile是加密的, 设置pw_callback允许客户端传递正确的密码用于解密, 如果设置为NULL,
**                               则必须在命令行上输入密码, 在回调函数必须将密码写入字节长度为size的buf中, 返回值必须是
**                               密码的长度, userdata将被设置到MQTT对象里
** 返回:       MQTT_CODE_SUCCESS
**             MQTT_CODE_INVAL
**             MQTT_CODE_NOMEM
********************************************************************/
extern int mqtt_tls_set(MQTT_T client, const char* cafile, const char* certfile, const char* keyfile,
		                int (*pw_callback)(char* buf, int size, int rwflag, void* userdata));

/*******************************************************************
** 函数描述:   配置SSL/TLS高级选项, 注意: 必须在连接之前调用
** 参数:       [in] client: MQTT对象
**             [in] certReqs: 定义客户端强加于服务器的验证需求, 值可以是:
**                            SSL_VERIFY_NONE(0) - 将不会以任何方式验证服务器
**                            SSL_VERIFY_PEER(1) - 如果验证失败, 将验证服务器证书并终止连接
**                            默认推荐使用SSL_VERIFY_PEER, 使用SSL_VERIFY_NONE不提供安全性
**             [in] tlsVersion: SSL/TLS协议的版本号, 如果为NULL, 将使用默认值, 默认值和可用值取决于编译库时使用的openssl
**                              版本, 对于openssl >= 1.0.1, 可用值有[tlsv1, tlsv1.1, tlsv1.2], 默认值是: tlsv1.2, 对于
**                              openssl < 1.0.1, 可用值只有: tlsv1
**             [in] ciphers: 有效的密码, 如果为NULL, 将使用默认密码, 更多信息可参考工具"openssl ciphers"
** 返回:       MQTT_CODE_SUCCESS
**             MQTT_CODE_INVAL
**             MQTT_CODE_NOMEM
********************************************************************/
extern int mqtt_tls_opts_set(MQTT_T client, int certReqs, const char* tlsVersion, const char* ciphers);

/*******************************************************************
** 函数描述:   连接
** 参数:       [in] client: MQTT对象
**             [in] host: 代理服务器主机地址, 例如: mqtt.eclipse.org
**             [in] port: 代理服务器端口, 通常是1883, 若支持TLS则通常是8883
**             [in] keepalive: 当没有消息交换时,客户端在此之后发送PING消息的秒数
** 返回:       MQTT_CODE_SUCCESS
**             MQTT_CODE_INVAL
**             MQTT_CODE_ERRNO
********************************************************************/
extern int mqtt_connect(MQTT_T client, const char* host, int port, int keepalive);

/*******************************************************************
** 函数描述:   断开
** 参数:       [in] client: MQTT对象
** 返回:       MQTT_CODE_SUCCESS
**             MQTT_CODE_INVAL
**             MQTT_CODE_NO_CONN
********************************************************************/
extern int mqtt_disconnect(MQTT_T client);

/*******************************************************************
** 函数描述:   发起订阅, 注意: 必须在连接成功后调用才有效
** 参数:       [in] client: MQTT对象
**             [out] msgId: 消息id
**             [in] pattern: 主题模式, 可以包含模式字符, 如: #,+
**             [in] qos: 所订阅消息的服务质量, 值为: 0, 1, 2
** 返回:       MQTT_CODE_SUCCESS
**             MQTT_CODE_INVAL
**             MQTT_CODE_NOMEM
**             MQTT_CODE_NO_CONN
**             MQTT_CODE_MALFORMED_UTF8
**             MQTT_CODE_OVERSIZE_PACKET

********************************************************************/
extern int mqtt_subscribe(MQTT_T client, int* msgId, const char* pattern, int qos);

/*******************************************************************
** 函数描述:   取消订阅, 注意: 必须在连接成功后调用才有效
** 参数:       [in] client: MQTT对象
**             [out] msgId: 消息id
**             [in] pattern: 主题模式, 可以包含模式字符, 如: #,+
** 返回:       MQTT_CODE_SUCCESS
**             MQTT_CODE_INVAL
**             MQTT_CODE_NOMEM
**             MQTT_CODE_NO_CONN
**             MQTT_CODE_MALFORMED_UTF8
**             MQTT_CODE_OVERSIZE_PACKET
********************************************************************/
extern int mqtt_unsubscribe(MQTT_T client, int* msgId, const char* pattern);

/*******************************************************************
** 函数描述:   发布消息, 注意: 必须在连接成功后调用才有效
** 参数:       [in] client: MQTT对象
**             [out] msgId: 消息id
**             [in] topic: 主题
**             [in] data: 要发送的数据
**             [in] dataLen: 数据长度(字节), 有效的值在0到268,435,455之间
**             [in] qos: 消息的服务质量, 值范围[0 - 至多一次, 1 - 至少一次, 2 - 只有一次]
**             [in] retain: 消息是否保留, 值范围[0 - 不保留, 1 - 保留]
** 返回:       MQTT_CODE_SUCCESS
**             MQTT_CODE_INVAL
**             MQTT_CODE_NOMEM
**             MQTT_CODE_NO_CONN
**             MQTT_CODE_PROTOCOL
**             MQTT_CODE_PAYLOAD_SIZE
**             MQTT_CODE_MALFORMED_UTF8
**             MQTT_CODE_QOS_NOT_SUPPORTED
**             MQTT_CODE_OVERSIZE_PACKET
********************************************************************/
extern int mqtt_publish(MQTT_T client, int* msgId, const char* topic, const void* data, int dataLen, int qos, int retain);

/*******************************************************************
** 函数描述:   获取日志类型描述字符串
** 参数:       [in] type: 日志类型
** 返回:       类型描述字符串
********************************************************************/
extern const char* mqtt_log_str(int type);

/*******************************************************************
** 函数描述:   获取响应码描述字符串
** 参数:       [in] code: 响应码
** 返回:       响应码描述字符串
********************************************************************/
extern const char* mqtt_code_str(int code);

#ifdef __cplusplus
}
#endif

#endif

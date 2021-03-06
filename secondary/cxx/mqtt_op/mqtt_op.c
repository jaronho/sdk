/**********************************************************************
* Author:	jaron.ho
* Date:		2020-05-07
* Brief:	MQTT通信接口封装
**********************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mqtt_op.h"
#include "mosquitto/include/mosquitto.h"

/* 全局总的mosq个数 */
static int s_mosq_count = 0;

/* MQTT自定义数据类型 */
struct mqttuserdata {
    int use_default_new_thread;                                                /* 是否使用默认新创建的线程处理网络消息:0 - 否, 1 - 是 */
    int is_connected;                                                          /* 是否已连接:0 - 未连接, 1 - 已连接 */
    MQTT_CALLBACK_T callback;                                                  /* 回调函数集合 */
};

/*******************************************************************
** 函数描述:   日志回调函数
** 参数:       [in] mosq: 触发此次回调的mosquitto实例
**             [in] obj: msoq的私有数据,在调用mosquitto_new方法时传入
**             [in] level: 日志等级
**                         MOSQ_LOG_INFO
**                         MOSQ_LOG_NOTICE
**                         MOSQ_LOG_WARNING
**                         MOSQ_LOG_ERR
**                         MOSQ_LOG_DEBUG
**             [in] str: 日志消息
** 返回:       无
********************************************************************/
static void mosq_log_callback(struct mosquitto* mosq, void* obj, int level, const char* str) {
    struct mqttuserdata* userdata;
    if (obj) {
        userdata = (struct mqttuserdata*)obj;
        if (userdata && userdata->callback.on_log) {
            switch (level) {
            case MOSQ_LOG_INFO:
                userdata->callback.on_log(MQTT_LOG_INFO, str);
                break;
            case MOSQ_LOG_NOTICE:
                userdata->callback.on_log(MQTT_LOG_NOTICE, str);
                break;
            case MOSQ_LOG_WARNING:
                userdata->callback.on_log(MQTT_LOG_WARNING, str);
                break;
            case MOSQ_LOG_ERR:
                userdata->callback.on_log(MQTT_LOG_ERR, str);
                break;
            case MOSQ_LOG_DEBUG:
                userdata->callback.on_log(MQTT_LOG_DEBUG, str);
                break;
            default:
                break;
            }
        }
    }
}

/*******************************************************************
** 函数描述:   连接回调函数
** 参数:       [in] mosq: 触发此次回调的mosquitto实例
**             [in] obj: msoq的私有数据,在调用mosquitto_new方法时传入
**             [in] code: 响应码
**                        0 - 成功
**                        1 - 连接拒绝(不可接受的协议版本)
**                        2 - 连接拒绝(标识符被拒绝)
**                        3 - 连接拒绝(代理不可用)
**                        4-255 - 预留作日后使用
** 返回:       无
********************************************************************/
static void mosq_connect_callback(struct mosquitto* mosq, void* obj, int code) {
    struct mqttuserdata* userdata;
    if (obj) {
        userdata = (struct mqttuserdata*)obj;
        if (userdata) {
            if (0 == code) {
                userdata->is_connected = 1;
            }
            if (userdata->callback.on_connect) {
                userdata->callback.on_connect(code);
            }
        }
    }
}

/*******************************************************************
** 函数描述:   断开回调函数
** 参数:       [in] mosq: 触发此次回调的mosquitto实例
**             [in] obj: msoq的私有数据,在调用mosquitto_new方法时传入
**             [in] code: 响应码, 如果值为0表示客户端调用断开接口, 其他值表示意外断开
** 返回:       无
********************************************************************/
static void mosq_disconnect_callback(struct mosquitto* mosq, void* obj, int code) {
    struct mqttuserdata* userdata;
    if (obj) {
        userdata = (struct mqttuserdata*)obj;
        if (userdata) {
            userdata->is_connected = 0;
            if (userdata->callback.on_disconnect) {
                userdata->callback.on_disconnect(code);
            }
        } 
    }
}

/*******************************************************************
** 函数描述:   订阅请求回调函数
** 参数:       [in] mosq: 触发此次回调的mosquitto实例
**             [in] obj: msoq的私有数据,在调用mosquitto_new方法时传入
**             [in] mid: 订阅请求的消息id
**             [in] qos_count: 授予订阅的数量(granted_qos的大小)
**             [in] granted_qos: 每个订阅所授予的QoS。
** 返回:       无
********************************************************************/
static void mosq_subscribe_callback(struct mosquitto* mosq, void* obj, int mid, int qos_count, const int* granted_qos) {
    struct mqttuserdata* userdata;
    if (obj) {
        userdata = (struct mqttuserdata*)obj;
        if (userdata && userdata->callback.on_subscribe) {
            userdata->callback.on_subscribe(mid, qos_count, granted_qos);
        }
    }
}

/*******************************************************************
** 函数描述:   订阅取消回调函数
** 参数:       [in] mosq: 触发此次回调的mosquitto实例
**             [in] obj: msoq的私有数据,在调用mosquitto_new方法时传入
**             [in] mid: 订阅取消的消息id
** 返回:       无
********************************************************************/
static void mosq_unsubscribe_callback(struct mosquitto* mosq, void* obj, int mid) {
    struct mqttuserdata* userdata;
    if (obj) {
        userdata = (struct mqttuserdata*)obj;
        if (userdata && userdata->callback.on_unsubscribe) {
            userdata->callback.on_unsubscribe(mid);
        }
    }
}

/*******************************************************************
** 函数描述:   发布回调函数
** 参数:       [in] mosq: 触发此次回调的mosquitto实例
**             [in] obj: msoq的私有数据,在调用mosquitto_new方法时传入
**             [in] mid: 发布的消息id
** 返回:       无
********************************************************************/
static void mosq_publish_callback(struct mosquitto* mosq, void* obj, int mid) {
    struct mqttuserdata* userdata;
    if (obj) {
        userdata = (struct mqttuserdata*)obj;
        if (userdata && userdata->callback.on_publish) {
            userdata->callback.on_publish(mid);
        }
    }
}

/*******************************************************************
** 函数描述:   消息回调函数
** 参数:       [in] mosq: 触发此次回调的mosquitto实例
**             [in] obj: msoq的私有数据,在调用mosquitto_new方法时传入
**             [in] message: 消息数据
** 返回:       无
********************************************************************/
static void mosq_message_callback(struct mosquitto* mosq, void* obj, const struct mosquitto_message* msg) {
    struct mqttuserdata* userdata;
    if (obj) {
        userdata = (struct mqttuserdata*)obj;
        if (userdata && userdata->callback.on_message) {
            userdata->callback.on_message(msg->mid, msg->topic, msg->payload, msg->payloadlen, msg->qos, msg->retain ? 1 : 0);
        }
    }
}

int mqtt_count(void) {
    return s_mosq_count;
}

MQTT_T mqtt_create(const char* clientId, int cleanSession, MQTT_CALLBACK_T* callback, int useDefaultNewThread) {
    struct mqttuserdata* userdata;
    struct mosquitto* mosq;
    /* 初始化库 */
    ++s_mosq_count;
    if (s_mosq_count > 0) {
        mosquitto_lib_init();
    }
    /* 创建客户端 */
    userdata = (struct mqttuserdata*)malloc(sizeof(struct mqttuserdata));
    userdata->is_connected = 0;
    if (callback) {
        userdata->callback.on_log = callback->on_log;
        userdata->callback.on_connect = callback->on_connect;
        userdata->callback.on_disconnect = callback->on_disconnect;
        userdata->callback.on_subscribe = callback->on_subscribe;
        userdata->callback.on_unsubscribe = callback->on_unsubscribe;
        userdata->callback.on_publish = callback->on_publish;
        userdata->callback.on_message = callback->on_message;
    } else {
        userdata->callback.on_log = NULL;
        userdata->callback.on_connect = NULL;
        userdata->callback.on_disconnect = NULL;
        userdata->callback.on_subscribe = NULL;
        userdata->callback.on_unsubscribe = NULL;
        userdata->callback.on_publish = NULL;
        userdata->callback.on_message = NULL;
    }
    mosq = mosquitto_new(clientId, (clientId && strlen(clientId)) ? cleanSession : 1, userdata);
    if (!mosq) {
        free(userdata);
        --s_mosq_count;
        if (0 == s_mosq_count) {
            mosquitto_lib_cleanup();
        }
        return NULL;
    }
    /* 设置回调函数 */
    mosquitto_log_callback_set(mosq, mosq_log_callback);
    mosquitto_connect_callback_set(mosq, mosq_connect_callback);
    mosquitto_disconnect_callback_set(mosq, mosq_disconnect_callback);
    mosquitto_subscribe_callback_set(mosq, mosq_subscribe_callback);
    mosquitto_unsubscribe_callback_set(mosq, mosq_unsubscribe_callback);
    mosquitto_publish_callback_set(mosq, mosq_publish_callback);
    mosquitto_message_callback_set(mosq, mosq_message_callback);
    /* 开启一个线程来处理网络信息 */
    userdata->use_default_new_thread = useDefaultNewThread;
    if (useDefaultNewThread) {
        if (MOSQ_ERR_SUCCESS != mosquitto_loop_start(mosq)) {
            free(userdata);
            mosquitto_destroy(mosq);
            --s_mosq_count;
            if (0 == s_mosq_count) {
                mosquitto_lib_cleanup();
            }
            return NULL;
        }
    }
    return mosq;
}

void mqtt_destroy(MQTT_T client) {
    struct mqttuserdata* userdata;
    struct mosquitto* mosq;
    if (client) {
        mosq = (struct mosquitto*)client;
        if (mosq) {
            mosquitto_disconnect(mosq);        
            userdata = (struct mqttuserdata*)mosquitto_userdata(mosq);
            if (userdata) {
                if (userdata->use_default_new_thread) {
                    mosquitto_loop_stop(mosq, 1);
                }
                free(userdata);
            }
            mosquitto_destroy(mosq);
            client = NULL;
            --s_mosq_count;
            if (0 == s_mosq_count) {
                mosquitto_lib_cleanup();
            }
        }
    } 
}

int mqtt_loop(MQTT_T client, int timeout) {
    struct mqttuserdata* userdata;
    struct mosquitto* mosq;
    if (client) {
        mosq = (struct mosquitto*)client;
        if (mosq) {
            userdata = (struct mqttuserdata*)mosquitto_userdata(mosq);
            if (userdata) {
                if (userdata->use_default_new_thread) {
                    return MOSQ_ERR_INVAL;
                }
                return mosquitto_loop(mosq, timeout, 1);   
            }
        }
    }
    return MQTT_CODE_INVAL;
}

int mqtt_is_connected(MQTT_T client) {
    struct mqttuserdata* userdata;
    struct mosquitto* mosq;
    mosq = (struct mosquitto*)client;
    if (client) {
        if (mosq) {
            userdata = (struct mqttuserdata*)mosquitto_userdata(mosq);
            if (userdata) {
                if (userdata->is_connected) {
                    return MQTT_CODE_SUCCESS;
                }
                return MQTT_CODE_NO_CONN;
            }
        }
    }
    return MQTT_CODE_INVAL;
}

int mqtt_tls_set(MQTT_T client, const char* cafile, const char* certfile, const char* keyfile,
		                int (*pw_callback)(char* buf, int size, int rwflag, void* userdata)) {
    struct mosquitto* mosq;
    if (client) {
        mosq = (struct mosquitto*)client;
        if (mosq) {
            return mosquitto_tls_set(mosq, cafile, NULL, certfile, keyfile, pw_callback);
        }
    }
    return MQTT_CODE_INVAL;
}

int mqtt_tls_opts_set(MQTT_T client, int certReqs, const char* tlsVersion, const char* ciphers) {
    struct mosquitto* mosq;
    if (client) {
        mosq = (struct mosquitto*)client;
        if (mosq) {
            return mosquitto_tls_opts_set(mosq, certReqs, tlsVersion, ciphers);
        }
    }
    return MQTT_CODE_INVAL;
}

int mqtt_connect(MQTT_T client, const char* host, int port, int keepalive) {
    struct mqttuserdata* userdata;
    struct mosquitto* mosq;
    if (client) {
        mosq = (struct mosquitto*)client;
        if (mosq) {
            userdata = (struct mqttuserdata*)mosquitto_userdata(mosq);
            if (userdata) {
                if (userdata->is_connected) {
                    return MQTT_CODE_SUCCESS;
                }
                if (userdata->use_default_new_thread) {
                    return mosquitto_connect_async(mosq, host, port, keepalive);
                }
                return mosquitto_connect(mosq, host, port, keepalive);
            }
        }
    }
    return MQTT_CODE_INVAL;
}

int mqtt_disconnect(MQTT_T client) {
    struct mosquitto* mosq;
    if (client) {
        mosq = (struct mosquitto*)client;
        if (mosq) {
            return mosquitto_disconnect(mosq);
        }
    }
    return MQTT_CODE_INVAL;
}

int mqtt_subscribe(MQTT_T client, int* msgId, const char* pattern, int qos) {
    struct mosquitto* mosq;
    if (client) {
        mosq = (struct mosquitto*)client;
        if (mosq) {
            return mosquitto_subscribe(mosq, msgId, pattern, qos);
        }
    }
    return MQTT_CODE_INVAL;
}

int mqtt_unsubscribe(MQTT_T client, int* msgId, const char* pattern) {
    struct mosquitto* mosq;
    if (client) {
        mosq = (struct mosquitto*)client;
        if (mosq) {
            return mosquitto_unsubscribe(mosq, msgId, pattern);
        }
    }
    return MQTT_CODE_INVAL;
}

int mqtt_publish(MQTT_T client, int* msgId, const char* topic, int payloadlen, const void* payload, int qos, int retain) {
    struct mosquitto* mosq;
    if (client) {
        mosq = (struct mosquitto*)client;
        if (mosq) {
            return mosquitto_publish(mosq, msgId, topic, payloadlen, payload, qos, retain ? 1 : 0);
        }
    }
    return MQTT_CODE_INVAL;
}

const char* mqtt_log_str(int type) {
    static char str[16];
    switch (type) {
    case MQTT_LOG_ERR:
        return "ERR";
    case MQTT_LOG_WARNING:
        return "WARNING";
    case MQTT_LOG_NOTICE:
        return "NOTICE";
    case MQTT_LOG_INFO:
        return "INFO";
    case MQTT_LOG_DEBUG:
        return "DEBUG";
    default:
        memset(str, 0, sizeof(str));
        sprintf(str, "UNKNOWN(%d)", type);
        return str;
    }
}

const char* mqtt_code_str(int code) {
    static char str[16];
    switch (code) {
    case MQTT_CODE_SUCCESS:
        return "SUCCESS";
    case MQTT_CODE_NOMEM:
        return "NOMEM";
    case MQTT_CODE_PROTOCOL:
        return "PROTOCOL";
    case MQTT_CODE_INVAL:
        return "INVAL";
    case MQTT_CODE_NO_CONN:
        return "NO_CONN";
    case MQTT_CODE_CONN_LOST:
        return "CONN_LOST";
    case MQTT_CODE_PAYLOAD_SIZE:
        return "PAYLOAD_SIZE";
    case MQTT_CODE_ERRNO:
        return "ERRNO";
    case MQTT_CODE_MALFORMED_UTF8:
        return "MALFORMED_UTF8";
    case MQTT_CODE_QOS_NOT_SUPPORTED:
        return "QOS_NOT_SUPPORTED";
    case MQTT_CODE_OVERSIZE_PACKET:
        return "OVERSIZE_PACKET";
    default:
        memset(str, 0, sizeof(str));
        sprintf(str, "UNKNOWN(%d)", code);
        return str;
    }
}

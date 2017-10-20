/*
 * Memshare, quick and easy IPC.
 * Copyright (C) 2012  Tommy Wiklund
 */

#ifndef _MEMSHARE_H_
#define _MEMSHARE_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*shm_callback_msg)(const char* proc_name, int msg_type, long msg_len, const void* data);
typedef void (*shm_callback_log)(int level, const char* format, ...);

#define DEF_PROC_NUM		2
#define DEF_SHM_KEY			0x058E
#define DEF_SHM_SIZE		1024*1024L
#define DEF_QUEUE_SIZE		1024

/*****************************************************************************/
/* Description        : Sets the verbose level for output, every thing below */
/*                      will be printed                                      */
/* Input(s)           : From 0 to 7, syslog levels can be used (int):        */
/*                      0.LOG_EMERG, 1.LOG_ALERT, 2.LOG_CRIT, 3.LOG_ERR,     */
/*                      4.LOG_WARNING, 5.LOG_NOTICE, 6.LOG_INFO, 7.LOG_DEBUG */
/*                      If an external logfunction is used this level is     */
/*                      bypassed                                             */
/* Output(s)          : None                                                 */
/* Return Value(s)    : 0.ok                                                 */
/*                      1.level value is error                               */
/*****************************************************************************/
extern int set_print_level(int level);

/*****************************************************************************/
/* Description        : This function initialises the memshare lib           */
/* Input(s)           : Proc name(string, max char count is 64)              */
/*                      Number of procs                                      */
/*                      Key of share memory ctrl area                        */
/*                      Size of memory for receiving buffer, null means that */
/*                      the proccess will not register for receive(long byte)*/
/*                      The queue size as buffer for receiving (int entries) */
/*                      A Function will be called whenever a msg is received */
/*                      A Function will be called whenever logging           */
/* Output(s)          : None                                                 */
/* Return Value(s)    : 0.ok                                                 */
/*                      1.module has been initialized                        */
/*                      2.proc name is error                                 */
/*                      3.proc num must >= 2                                 */
/*                      4.shm key must >= 0                                  */
/*                      5.shm size must > 0                                  */
/*                      6.queue capacity must > 0                            */
/*                      7.unable to create semaphore                         */
/*                      8.unable to alloc shared memory                      */
/*****************************************************************************/
extern int init_memshare(const char* proc_name, int proc_num, int shm_key, long shm_size, int queue_capacity, shm_callback_msg scbm, shm_callback_log scbl);

/*****************************************************************************/
/* Description        : This functionsn will send a msg block to a specific  */
/*                      dest proc with thread blocking                       */
/* Input(s)           : Destination proc(const char*), msg_type of msg(int)  */
/*						msg block(const void*), length of msg block(long)    */
/* Output(s)          : None                                                 */
/* Return Value(s)    : 0.ok                                                 */
/*                      1.module has not been initialized                    */
/*                      2.proc name is error                                 */
/*                      3.no dest process process available                  */
/*                      4.data size large shm size                           */
/*****************************************************************************/
extern int shm_send(const char* proc_name, int msg_type, long msg_len, const void* data);

/*****************************************************************************/
/* Description        : This functionsn will send a msg block to a specific  */
/*                      dest proc with thread Non-blocking IO                */
/* Input(s)           : Destination proc(const char*), msg_type of msg(int)  */
/*						msg block(const void*), length of msg block(long)    */
/* Output(s)          : None                                                 */
/* Return Value(s)    : None                                                 */
/*****************************************************************************/
extern void shm_send_nio(const char* proc_name, int msg_type, long msg_len, const void* data);

/*****************************************************************************/
/* Description        : This function get the index of proc                  */
/* Input(s)           : Proc name(const char*)                               */
/* Output(s)          : None                                                 */
/* Return Value(s)    : -1.module has not been initialized                   */
/*                      -2.proc name is error                                */
/*                      -3.no such process                                   */
/*                      >=0.index                                            */
/*****************************************************************************/
extern int get_proc_index(const char* proc_name);

/*****************************************************************************/
/* Description        : This function get the proc info at index             */
/* Input(s)           : Proc index                                           */
/* Output(s)          : Address of proc name(char*), address of data size    */
/* Return Value(s)    : 0.ok                                                 */
/*                      1.module has not been initialized                    */
/*                      2.no such process                                    */
/*****************************************************************************/
extern int get_proc_info(int index, char** proc_name, long* data_size);

#ifdef __cplusplus
}
#endif

#endif	// _MEMSHARE_H_

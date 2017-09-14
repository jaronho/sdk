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

typedef void (*callback_msg)(const char*, int, long, const void*);
typedef void (*callback_logfunction)(int, const char*, ...);

#define DEF_PROC_NUM		10
#define DEF_SHM_KEY			0x058E
#define DEF_SHM_SIZE		10*1024*1024L
#define DEF_QUEUE_SIZE		1024

extern int check_proc_entry(int index);
extern int get_proc_index(const char* proc_name);
extern void get_proc_info(int index, char** proc_name, long* data_size, long* long_max, long* major_send_count, long* minor_send_count, long* major_rec_count, long* minor_rec_count);

/*****************************************************************************/
/* Description        : This function initialises the memshare lib           */
/* Input(s)           : proc name(string 64 char)                            */
/*                      num of procs                                         */
/*                      key of share memory                                  */
/*                      Size of memory for receiving buffer, null means that */
/*                      the proccess will not register for receive(long byte)*/
/*                      The queue size as buffer for receiving (int entries) */
/* Output(s)          : None.                                                */
/* Return Value(s)    : 0 ok                                                 */
/*                      1 meaning semaphore allocation failure               */
/*                      2 register a proc without allocation size            */
/*                      3 A NULL pointer as a proc name                      */
/*****************************************************************************/
extern int init_memshare(const char* proc_name, int proc_num, int shm_key, long shm_size, int queue_size);

/*****************************************************************************/
/* Description        : This function registers a function that will be      */
/*                      called whenever a msg is received.                   */
/* Input(s)           : a function func(const char*, int, long, const void*) */
/* Output(s)          : None.                                                */
/* Return Value(s)    : None.                                                */
/*****************************************************************************/
extern void register_msg(callback_msg cbm);

/*****************************************************************************/
/* Description        : Add a function to be called when logging             */
/* Input(s)           : a function func(int, const char*, ...)               */
/* Output(s)          : None.                                                */
/* Return Value(s)    : None.                                                */
/*****************************************************************************/
extern void register_logfunction(callback_logfunction cblf);

/*****************************************************************************/
/* Description        : This functionsn will send a msg block to a specific  */
/*                      dest proc                                            */
/* Input(s)           : destination proc(const char*), msg_type of msg(int)  */
/*						msg block(const void*), length of msg block(long)    */
/* Output(s)          : None.                                                */
/* Return Value(s)    : 0 ok                                                 */
/*                      1 No dest process process available                  */
/*                      2 Memshare not initialized                           */
/*****************************************************************************/
extern int send_msg(const char* proc_name, int msg_type, long msg_len, const void* data);

/*****************************************************************************/
/* Description        : This function ask for the size of data a process     */
/*                      can receive                                          */
/* Input(s)           : proc_name(const char*)                               */
/* Output(s)          : None.                                                */
/* Return Value(s)    : 0 No such process                                    */
/*                      n The max size in bytes that can be sent with data   */
/*****************************************************************************/
extern int get_datasize(const char* proc_name);

/*****************************************************************************/
/* Description        : Sets the verbose level for output, every thing below */
/*                    : will be printed.                                     */
/* Input(s)           : From 0 to 7, syslog levels can be used (int):        */
/*                      0.LOG_EMERG, 1.LOG_ALERT, 2.LOG_CRIT, 3.LOG_ERR,     */
/*                      4.LOG_WARNING, 5.LOG_NOTICE, 6.LOG_INFO, 7.LOG_DEBUG */
/*                    : If an external logfunction is used this level is     */
/*                    : bypassed                                             */
/* Output(s)          : None.                                                */
/* Return Value(s)    : 0 ok                                                 */
/*****************************************************************************/
extern int set_print_level(int level);

#ifdef __cplusplus
}
#endif

#endif	// _MEMSHARE_H_

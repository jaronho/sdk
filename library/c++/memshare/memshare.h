/*
 * Memshare, quick and easy IPC.
 * Copyright (C) 2012  Tommy Wiklund
 */

#ifndef _MEMSHARE_H_
#define _MEMSHARE_H_

typedef void (*callback_msg)(const char*, int, int, const void*);
typedef void (*callback_logfunction)(int, const char*, ...);

#define DEF_PROC_NUM		10
#define DEF_SHM_KEY			0xF216C5
#define DEF_SHM_SIZE		10*1024*1024
#define DEF_QUEUE_SIZE		1024

int get_proc_index(const char* proc_name);
int get_proc_info(int index, long* send_count, long* rec_count, int* data_size, char** proc_name);
int check_proc_entry(int index);

/*****************************************************************************/
/* Description        : This function initialises the memshare lib           */
/* Input(s)           : proc name(string 64 char)                            */
/*                      num of procs                                         */
/*                      key of share memory                                  */
/*                      Size of memory for receiving buffer, null means that */
/*                      the proccess will not register for receive(int byte) */
/*                      The queue size as buffer for receiving (int entries) */
/* Output(s)          : None.                                                */
/* Return Value(s)    : 0 ok                                                 */
/*                      1 meaning semaphore allocation failure               */
/*                      2 register a proc without allocation size            */
/*                      3 A NULL pointer as a proc name                      */
/*****************************************************************************/
int init_memshare(const char* proc_name, int proc_num, int shm_key, int shm_size, int queue_size);

/*****************************************************************************/
/* Description        : This function registers a function that will be      */
/*                      called whenever a msg is received.                   */
/* Input(s)           : a function func(const char*, int, int, const void*)  */
/* Output(s)          : None.                                                */
/* Return Value(s)    : None.                                                */
/*****************************************************************************/
void register_msg(callback_msg cbm);

/*****************************************************************************/
/* Description        : Add a function to be called when logging             */
/* Input(s)           : a function func(int, const char*, ...)               */
/* Output(s)          : None.                                                */
/* Return Value(s)    : None.                                                */
/*****************************************************************************/
void register_logfunction(callback_logfunction cblf);

/*****************************************************************************/
/* Description        : This functionsn will send a msg block to a specific  */
/*                      dest proc                                            */
/* Input(s)           : destination proc(const char*), msg_type of msg(int)  */
/*						msg block(const void*), length of msg block(int)     */
/* Output(s)          : None.                                                */
/* Return Value(s)    : 0 ok                                                 */
/*                      1 No dest process process available                  */
/*                      2 Memshare not initialized                           */
/*****************************************************************************/
int send_msg(const char* proc_name, int msg_type, int msg_len, const void* data);

/*****************************************************************************/
/* Description        : This function ask for the size of data a process     */
/*                      can receive                                          */
/* Input(s)           : proc_name(const char*)                               */
/* Output(s)          : None.                                                */
/* Return Value(s)    : 0 No such process                                    */
/*                      n The max size in bytes that can be sent with data   */
/*****************************************************************************/
int get_datasize(const char* proc_name);

/*****************************************************************************/
/* Description        : Sets the verbose level for output, every thing below */
/*                    : will be printed.                                     */
/* Input(s)           : From 0 to 7, syslog levels can be used (int):        */
/*                      LOG_EMERG, LOG_ALERT, LOG_CRIT, LOG_ERR, LOG_WARNING */
/*                      LOG_NOTICE, LOG_INFO, LOG_DEBUG                      */
/*                    : If an external logfunction is used this level is     */
/*                    : bypassed                                             */
/* Output(s)          : None.                                                */
/* Return Value(s)    : 0 ok                                                 */
/*****************************************************************************/
int set_print_level(int level);

#endif	// _MEMSHARE_H_

/* Memshare, quick and easy IPC.                                                   */
/* Copyright (C) 2012  Tommy Wiklund                                               */
/* This file is part of Memshare.                                                  */
/*                                                                                 */
/* Memshare is free software: you can redistribute it and/or modify                */
/* it under the terms of the GNU Lesser General Public License as published by     */
/* the Free Software Foundation, either version 3 of the License, or               */
/* (at your option) any later version.                                             */
/*                                                                                 */
/* Memshare is distributed in the hope that it will be useful,                     */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of                  */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                   */
/* GNU Lesser General Public License for more details.                             */
/*                                                                                 */
/* You should have received a copy of the GNU Lesser General Public License        */
/* along with Memshare.  If not, see <http://www.gnu.org/licenses/>.               */

#include "memshare.h"
#include "../queue/queue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#ifndef NULL
#define NULL	0
#endif

#define LONG_MAX_VALUE	2147483647L
#define PROC_NAME_SIZE	64
#define DEF_FREQ_MILLISECOND	16

union semun {
	int val;
	struct semid_ds* buf;
	ushort* array;
};

/* struct that is passed along with the data/signal between procs */
typedef struct {
	char proc_name[PROC_NAME_SIZE];	/* orginating proc */
	int msg_type;
	long msg_len;
	long major_seq;
	long minor_seq;
} header;
#define SIZEOF_HEADER sizeof(header)

/* Every process will scan the ctrl area and map every proc entry */
/* to a mem_proc_entry for quick access to that process */
typedef struct {
	char proc_name[PROC_NAME_SIZE];
	void* shm;
	int rlock;
	int wlock;
	int active;
} mem_proc_entry;
#define SIZEOF_MEM_PROC_ENTRY sizeof(mem_proc_entry)

/* Every process will register one of these in the ctrl area */
typedef struct {
	int key_shm;		/* key = index (in ctrl area) * 4 + 43 */
	long size_shm;		/* size of the shared memory allocated */
	int key_rlock;		/* key to hold the read lock for the shared mem */
	int key_wlock;		/* key to hold the write lock for the shared mem */
	int key_active;		/* key which will go zero if process terminates */
	int active;		/* flagg for the process to signal active */
	char proc_name[PROC_NAME_SIZE];	/* process name */
	long major_sent;		/* counter for major sent data/signals */
	long minor_sent;		/* counter for minor sent data/signals */
	long major_received;		/* counter for major received data/signals */
	long minor_received;		/* counter for minor received data/signals */
} proc_entry;
#define SIZEOF_PROC_ENTRY sizeof(proc_entry)

static pthread_t write_thread_t;
static pthread_t read_thread_t;

/* global pointer to the ctrl area, initialized by get_shm() */
void* shm_ctrl_ptr = NULL;
/* global mutex protecting the ctrl area */
int lock_ctrl_sem = 0;
/* internal memory view of the ctrl area */
mem_proc_entry* mem_entry = NULL;
/* initialized flag */
int initialized = 0;
/* My own process name */
char my_proc_name[PROC_NAME_SIZE];

int my_index;
queue_st* my_queue;
long major_sequence = 0;
long minor_sequence = 0;
int num_of_procs = 0;
int shm_ctrl_key;
int sem_ctrl_key;
shm_callback_msg callbackmsg = NULL;
shm_callback_log callbacklog = NULL;
int freq_microsecond = DEF_FREQ_MILLISECOND * 1000;

static int print(int level, const char* format, ...);

static int create_lock(int key, int value);
static void lock(int sem);
static void unlock(int sem);
static int try_lock1(int sem);

static void init_mem_proc(int num);
static void populate_mem_proc(void);
static int clear_shm(int key, long size, void* data);
static int get_shm(int key, long size, int mode, void** data);

static proc_entry* get_proc_at(int index);
static int clear_proc_entry(int index);
static int get_next_free_index(void);
static void populate_mem_proc_single(int index);
static int inc_sent(void);
static int inc_received(void);

/* print functions either syslog, printf of user specific */
int current_level = LOG_ERR;

static int print(int level, const char* format, ...) {
	va_list ap;
	int retval = 0;
	/* add syslog and user specific TODO */
	va_start(ap, format);
	if (callbacklog) {
		callbacklog(level, format, ap);
	} else {
		if (level <= current_level) {
			retval = vprintf(format, ap);
		}
	}
	va_end(ap);
	return retval;
}

/********** ipc semaphore functions to be used as mutexes    ***********/
/* This function will try to create a semaphore for an index, exclusively    */
/* If it already has been created it will fail and we will open it normally  */
/* If the exclusive open succede we know we are the first process and we so  */
/* we will initialize it to work as a mutex                                  */
static int create_lock(int key, int value) {
	/* struct sembuf op[1]; */
	union semun ctrl;
	int sem;
	int mode = 0;
	print(LOG_DEBUG, "Create_lock key=%d, value=%d\n", key, value);
	if (-1 == (sem = semget(key, 1, IPC_EXCL | IPC_CREAT | 0666))) {
		/* Trying to open it exclusively failed, try normal */
		print(LOG_DEBUG, "Create_lock Key=%d already open\n", key);
		mode = 1;
		if (-1 == (sem = semget(key, 1, IPC_CREAT | 0666))) {
			print(LOG_ERR, "Unable to create semaphore, errno %d\n", errno);
			return -1;
		}
	}
	/*
	op[0].sem_num = 0;
	op[0].sem_flg = 0;
	*/
	ctrl.val = value;
	if (0 == mode) {
		print(LOG_DEBUG, "Create_lock Init sem=%d to %d\n", sem, value);
		/* Its the first time for this key, init it to work as a mutex */
		if (-1 == semctl(sem, 0, SETVAL, ctrl)) {
			print(LOG_ERR, "Unable to initialize semaphore, errno %d\n", errno);
			return -1;
		}
	}
	return sem;
}

static int destroy_lock(int key) {
	union semun ctrl;
	int sem;
	print(LOG_DEBUG, "Destroy_lock key=%d\n", key);
	if (-1 == (sem = semget(key, 1, IPC_CREAT | 0600))) {
		print(LOG_ERR, "Unable to create semaphore, errno %d\n", errno);
		return 1;
	}
	if (-1 == semctl(sem, 0, IPC_RMID, ctrl)) {
		print(LOG_ERR, "Unable to initialize semaphore, errno %d\n", errno);
		return 1;
	}
	return 0;
}

static void lock(int sem) {
	struct sembuf op[1];
	op[0].sem_num = 0;
	op[0].sem_op = -1;
	op[0].sem_flg = SEM_UNDO;
	semop(sem, op, 1);
}

static void unlock(int sem) {
	struct sembuf op[1];
	op[0].sem_num = 0;
	op[0].sem_op = 1;
	op[0].sem_flg = 0;
	semop(sem, op, 1);
}

static int try_lock1(int sem) {
	struct sembuf op[1];
	op[0].sem_num = 0;
	op[0].sem_op = 0;
	op[0].sem_flg = IPC_NOWAIT;
	if (-1 == semop(sem, op, 1) && EAGAIN == errno) {
		return 1;
	}
	return 0;
}

/********** mem_proc functions **********/
static void init_mem_proc(int num) {
	int i;
	num_of_procs = num;
	mem_entry = (mem_proc_entry*)malloc(num * sizeof(mem_proc_entry));
	for (i = 0; i < num; ++i) {
		mem_entry[i].shm = NULL;
		mem_entry[i].rlock = 0;
		mem_entry[i].wlock = 0;
		mem_entry[i].active = 0;
	}
}

/* scan through the ctrl area and copy to local memory */
static void populate_mem_proc(void) {
	int i;
	for (i = 0; i < num_of_procs; ++i) {
		populate_mem_proc_single(i);
	}
}

/********** the shared memory functions ***********/
static int clear_shm(int key, long size, void* data) {
	int shmid;
	if ((shmid = shmget(key, size, IPC_CREAT | 0666)) < 0) {
		print(LOG_ERR, "Unable get shared mem for key %d, errno %d\n", key, errno);
		return 1;
	}
	if (NULL != data) {
		shmdt(data);
	}
	shmctl(shmid, IPC_RMID, NULL);
	return 0;
}

/* This function will try to map shared memory for an index(key)  */
/* The return value is a pointer to the shared memory or NULL if  */
/* the function fail.                                             */
/* If mode is set to 1 the shared memory will be created if it    */
/* not has been done before.                                      */
/* With mode = 0 the function will fail if the index has been     */
/* created before                                                 */
static int get_shm(int key, long size, int mode, void** data) {
	if (NULL != *data) {
		return 0;
	}
	int shmid;
	if (mode) {
		if ((shmid = shmget(key, size, IPC_CREAT | IPC_EXCL | 0666)) < 0) {
			/* already open */
			mode = 0;
		} else {
			print(LOG_DEBUG, "Creating shared mem for key %d with% d\n", key, shmid);
		}
	}
	if (0 == mode) {
		if ((shmid = shmget(key, size, IPC_CREAT | 0666)) < 0) {
			print(LOG_ERR, "Unable get shared mem for key %d, errno %d\n", key, errno);
			return 1;
		} else {
			print(LOG_DEBUG, "Get shared mem for key %d with% d\n", key, shmid);
		}
	}
	if ((void*)-1 == (*data = shmat(shmid, NULL, 0))) {
		*data = NULL;
		print(LOG_ERR, "Unable to alloc shared mem for key %d, errno %d\n", key, errno);
		return 1;
	}
	return 0;
}

static proc_entry* get_proc_at(int index) {
	void* tmp_ptr;
	proc_entry* entry;
	tmp_ptr = shm_ctrl_ptr + (SIZEOF_PROC_ENTRY * index);
	entry = (proc_entry*)tmp_ptr;
	return entry;
}

static int clear_proc_entry(int index) {
	proc_entry* entry;
	print(LOG_DEBUG, "Removes proc from entry and memlist\n");
	entry = (proc_entry*)get_proc_at(index);
	clear_shm(entry->key_shm, entry->size_shm, mem_entry[index].shm);
	entry->key_shm = 0;
	entry->size_shm = 0;
	mem_entry[index].shm = NULL;
	destroy_lock(entry->key_rlock);
	entry->key_rlock = 0;
	mem_entry[index].rlock = 0;
	destroy_lock(entry->key_wlock);
	entry->key_wlock = 0;
	mem_entry[index].wlock = 0;
	destroy_lock(entry->key_active);
	entry->key_active = 0;
	mem_entry[index].active = 0;
	entry->active = 0;
	return 0;
}

static int get_next_free_index() {
	int i;
	for (i = 0; i < num_of_procs; ++i) {
		if (2 == check_proc_entry(i)) {
			return i;
		}
	}
	return num_of_procs;
}

/* Check a specific index int the ctrl area and copy to local memory  */

static void populate_mem_proc_single(int index) {
	proc_entry* entry;
	int sem;
	entry = (proc_entry*)get_proc_at(index);
	if (entry->active) {
		/* this entry should be active, if not it has crached and should be garbage collected */
		if (-1 != (sem = create_lock(entry->key_active, 0))) {
			if (try_lock1(sem)) {
				print(LOG_DEBUG, "Index %d is active by %s\n", index, entry->proc_name);
				/* active lets store the data */
				mem_entry[index].active = sem;
				if (1 == get_shm(entry->key_shm, entry->size_shm, 0, &mem_entry[index].shm)) {
					/* garbage collect, they should have valid keys */
					print(LOG_ERR, "Unable to alloc shared mem\n");
					clear_proc_entry(index);
					return;
				}
				if (-1 == (mem_entry[index].rlock = create_lock(entry->key_rlock, 0))) {
					/* garbage collect, they should have valid keys */
					print(LOG_ERR, "Unable to create rlock\n");
					clear_proc_entry(index);
					return;
				}
				if (-1 == (mem_entry[index].wlock = create_lock(entry->key_wlock, 0))) {
					/* garbage collect, they should have valid keys */
					print(LOG_ERR, "Unable to create wlock\n");
					clear_proc_entry(index);
					return;
				}
			} else {
				print(LOG_DEBUG, "Index %d is active in shared mem but has no process\n", index);
				clear_proc_entry(index);
				/* garbage collect */
			}
			memcpy(mem_entry[index].proc_name, entry->proc_name, PROC_NAME_SIZE);
		}
	} else {
		print(LOG_DEBUG, "Index %d is not active\n", index);
	}
}

static int inc_sent(void) {
	proc_entry* entry;
	/* The locks might not be needed TODO */
	/* lock(lock_ctrl_sem); */
	entry = (proc_entry*)get_proc_at(my_index);
	if (LONG_MAX_VALUE == entry->minor_sent) {
		entry->major_sent++;
		entry->minor_sent = 0;
	}
	entry->minor_sent++;
	/* unlock(lock_ctrl_sem); */
	return 0;
}

static int inc_received(void) {
	proc_entry* entry;
	/* The locks might not be needed TODO */
	/* lock(lock_ctrl_sem); */
	entry = (proc_entry*)get_proc_at(my_index);
	if (LONG_MAX_VALUE == entry->minor_received) {
		entry->major_received++;
		entry->minor_received = 0;
	}
	entry->minor_received++;
	/* unlock(lock_ctrl_sem); */
	return 0;
}

static void set_active(int sem) {
	struct sembuf op[1];
	op[0].sem_num = 0;
	op[0].sem_op = 1;
	op[0].sem_flg = SEM_UNDO;
	semop(sem, op, 1);
}

/* Should be used when leaving gracefully */
/*
static void clear_active(int sem) {
	struct sembuf op[1];
	op[0].sem_num = 0;
	op[0].sem_op = -1;
	op[0].sem_flg = 0;
	semop(sem, op, 1);
}
*/

/* This function will serach for a free entry in the ctrl area        */
/* there it will fill all the keys that can be used to map shared mem */
static int add_proc(const char* proc_name, long size) {
	proc_entry* entry;
	int index;
	int key_base = 0;
	if (num_of_procs == (index = get_next_free_index())) {
		return 1;
	}
	print(LOG_DEBUG, "Adding proc %s to index %d\n", proc_name, index);
	entry = get_proc_at(index);
	key_base = sem_ctrl_key + index * 4;
	entry->key_shm = key_base + 1;
	entry->key_rlock = key_base + 2;
	entry->key_wlock = key_base + 3;
	entry->key_active = key_base + 4;
	entry->size_shm = size;
	entry->major_sent = 0;
	entry->minor_sent = 0;
	entry->major_received = 0;
	entry->minor_received = 0;
	my_index = index;
	print(LOG_DEBUG, "Allocating shared memory for key %d size %ld\n", entry->key_shm, entry->size_shm);
	/* Map up yourself in the local memory map with pointers instead of keys */
	if (1 == get_shm(entry->key_shm, entry->size_shm, 0, &mem_entry[index].shm)) {
		print(LOG_ERR, "Unable to alloc shared mem\n");
		clear_proc_entry(index);
		return 1;
	}
	print(LOG_DEBUG, "Shared memory allocated for key %d\n", entry->key_shm);
	if (-1 == (mem_entry[index].rlock = create_lock(entry->key_rlock, 0))) {
		print(LOG_ERR, "Unable to create rlock\n");
		clear_proc_entry(index);
		return 1;
	}
	if (-1 == (mem_entry[index].wlock = create_lock(entry->key_wlock, 1))) {
		print(LOG_ERR, "Unable to create wlock\n");
		clear_proc_entry(index);
		return 1;
	}
	if (-1 == (mem_entry[index].active = create_lock(entry->key_active, 0))) {
		print(LOG_ERR, "Unable to create active\n");
		clear_proc_entry(index);
		return 1;
	}
	set_active(mem_entry[index].active);
	if (try_lock1(mem_entry[index].active)) {
		print(LOG_DEBUG, "Proc %s is now active at index %d\n", proc_name, index);
	}
	strncpy(entry->proc_name, proc_name, PROC_NAME_SIZE);
	memcpy(mem_entry[index].proc_name, entry->proc_name, PROC_NAME_SIZE);
	/* signal the entry active to use */
	entry->active = 1;
	return 0;
}

static void* write_thread_func(void* arg) {
	header* hdr;
	char* msg;
	for (;;) {
		usleep(freq_microsecond);
		/* Add check for prio, TODO */
		print(LOG_DEBUG, "write thread going to lock\n");
		lock(mem_entry[my_index].rlock);
		print(LOG_DEBUG, "Entry inserted in shm for my process\n");
		hdr = (header*)mem_entry[my_index].shm;
		if (NULL == hdr) {
			continue;
		}
		/* check return of allocation TODO */
		msg = malloc(SIZEOF_HEADER + hdr->msg_len);
		memcpy(msg, mem_entry[my_index].shm, SIZEOF_HEADER + hdr->msg_len);
		if (queue_put(my_queue, (void*)msg)) {
			/* Failed to put in queue, msg lost */
			print(LOG_ERR, "Failed to put msg in queue, msg with seq %ld - %ld is lost!\n", hdr->major_seq, hdr->minor_seq);
			free(msg);
		} else {
			/* msg concidered received */
			inc_received();
		}
		unlock(mem_entry[my_index].wlock);
	}
	return (void*)0;
}

static void* read_thread_func(void* arg) {
	void* msg;
	header* hdr;
	for (;;) {
		msg = queue_get(my_queue);
		hdr = (header*)msg;
		if (NULL == hdr) {
			if (NULL != msg) {
				free(msg);
			}
			continue;
		}
		if (NULL != callbackmsg) {
			callbackmsg(hdr->proc_name, hdr->msg_type, hdr->msg_len, hdr->msg_len > 0 ? (msg + SIZEOF_HEADER) : NULL);
		} else {
			print(LOG_WARNING, "No callback\n");
		}
		free(msg);
	}
	return (void*)0;
}

static int start_listen_thread(void) {
	pthread_attr_t tattr;
	if (0 != pthread_attr_init(&tattr)) {
		print(LOG_ERR, "Unable to init thread attribute, errno %d\n", errno);
		return 1;
	}
	if (0 != pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED)) {
		print(LOG_ERR, "Unable to set detached state to thread, errno %d\n", errno);
		return 1;
	}
	if (0 != pthread_attr_setinheritsched(&tattr, PTHREAD_INHERIT_SCHED)) {
		print(LOG_ERR, "Unable to set inherit scheduling, errno %d\n", errno);
		return 1;
	}
	if (0 != pthread_create(&write_thread_t, &tattr, write_thread_func, (void*)NULL)) {
		print(LOG_ERR, "Unable to create worker thread for write, errno %d\n", errno);
		return 1;
	}
	if (0 != pthread_create(&read_thread_t, &tattr, read_thread_func, (void*)NULL)) {
		print(LOG_ERR, "Unable to create worker thread for read, errno %d\n", errno);
		return 1;
	}
	return 0;
}

/****************************** API ******************************/

int check_proc_entry(int index) {
	/* compare mem_entry with shared memory and se if process is active */
	proc_entry* entry;
	entry = get_proc_at(index);
	if (NULL == entry) {
		return 1;
	}
	populate_mem_proc_single(index);
	if (try_lock1(mem_entry[index].active) && (!memcmp(mem_entry[index].proc_name, entry->proc_name, PROC_NAME_SIZE)) && entry->active) {
		return 0;
	}
	return 2;
}

int get_proc_index(const char* proc_name) {
	int i;
	for (i = 0; i < num_of_procs; ++i) {
		if (0 == check_proc_entry(i)) {
			if (!strcmp(mem_entry[i].proc_name, proc_name)) {
				return i;
			}
		}
	}
	return -1;
}

void get_proc_info(int index, char** proc_name, long* data_size, long* major_send_count, long* minor_send_count, long* major_rec_count, long* minor_rec_count) {
	proc_entry* entry;
	/* this call will not check if the entry is active */
	entry = get_proc_at(index);
	if (NULL != entry) {
		*proc_name = entry->proc_name;
		*data_size = entry->size_shm;
		*major_send_count = entry->major_sent;
		*minor_send_count = entry->minor_sent;
		*major_rec_count = entry->major_received;
		*minor_rec_count = entry->minor_received;
	}
}

long get_long_max() {
	return LONG_MAX_VALUE;
}

int init_memshare(const char* proc_name, int proc_num, int shm_key, long shm_size, int queue_size, shm_callback_msg scbm, shm_callback_log scbl) {
	if (initialized) {
		return 1;
	}
	print(LOG_DEBUG, "Init_memshare start\n");
	/* a source proc name is a must */
	if (NULL == proc_name || 0 == strlen(proc_name)) {
		print(LOG_ERR, "proc name is NULL\n");
		return 2;
	}
	if (strlen(proc_name) > PROC_NAME_SIZE) {
		print(LOG_ERR, "proc name '%s' length > %d\n", proc_name, PROC_NAME_SIZE);
		return 2;
	}
	memcpy(my_proc_name, proc_name, PROC_NAME_SIZE);
	proc_num = proc_num >= 2 ? proc_num : 2;
	shm_ctrl_key = shm_key;
	sem_ctrl_key = shm_key + 1;
	my_queue = queue_create(queue_size, 1, 1);
	callbackmsg = scbm;
	callbacklog = scbl;
	/* clear the memory view */
	init_mem_proc(proc_num);
	/* start off by locking the ctrl lock */
	if (-1 == (lock_ctrl_sem = create_lock(sem_ctrl_key, 1))) {
		print(LOG_ERR, "Unable to create semaphore\n");
		return 1;
	}
	print(LOG_DEBUG, "Created ctrl lock\n");
	lock(lock_ctrl_sem);
	print(LOG_DEBUG, "Init_memshare ctrl\n");
	/* map up the ctrl area */
	if (1 == get_shm(shm_ctrl_key, SIZEOF_PROC_ENTRY * proc_num, 1, &shm_ctrl_ptr)) {
		print(LOG_ERR, "Unable to alloc shared mem\n");
		return 3;
	}
	print(LOG_DEBUG, "Init_memshare populate memproc\n");
	populate_mem_proc();
	add_proc(proc_name, shm_size);
	unlock(lock_ctrl_sem);
	print(LOG_DEBUG, "Init_memshare unlock ctrl\n");
	start_listen_thread();
	print(LOG_DEBUG, "Init_memshare done\n");
	initialized = 1;
	return 0;
}

int set_freq(float millisecond) {
	if (millisecond >= 0) {
		freq_microsecond = (int)(millisecond * 1000);
		return 0;
	}
	return 1;
}

int send_msg(const char* proc_name, int msg_type, long msg_len, const void* data) {
	int index;
	header hdr;
	proc_entry* entry;
	if (!initialized) {
		return 2;
	}
	if ((index = get_proc_index(proc_name)) < 0) {
		print(LOG_NOTICE, "No such process %s\n", proc_name);
		return 1;
	}
	entry = get_proc_at(index);
	if (SIZEOF_HEADER + msg_len > entry->size_shm) {
		print(LOG_NOTICE, "Data size %ld large shm size %ld\n", SIZEOF_HEADER + msg_len, entry->size_shm);
		return 3;
	}
	print(LOG_DEBUG, "Sending data to %s at index %d\n", proc_name, index);
	if (LONG_MAX_VALUE == minor_sequence) {
		major_sequence++;
		minor_sequence = 0;
	}
	minor_sequence++;
	lock(mem_entry[index].wlock);
	hdr.msg_type = msg_type;
	hdr.msg_len = msg_len;
	hdr.major_seq = major_sequence;
	hdr.minor_seq = minor_sequence;
	memcpy(hdr.proc_name, my_proc_name, PROC_NAME_SIZE);
	memcpy(mem_entry[index].shm, &hdr, SIZEOF_HEADER);
	if (msg_len > 0 && NULL != data) {
		memcpy(mem_entry[index].shm + SIZEOF_HEADER, data, msg_len);
	}
	inc_sent();
	unlock(mem_entry[index].rlock);
	return 0;
}

int get_datasize(const char* proc_name) {
	int index;
	proc_entry* entry;
	if ((index = get_proc_index(proc_name)) < 0) {
		print(LOG_NOTICE, "No such process %s\n", proc_name);
		return 0;
	}
	entry = get_proc_at(index);
	return entry->size_shm;
}

int set_print_level(int level) {
	if (level >= LOG_EMERG && level <= LOG_DEBUG) {
		current_level = level;
		return 0;
	}
	return 1;
}

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

#define QUEUE_EMPTY		0
#define QUEUE_NORMAL	1
#define QUEUE_FULL		2
#define PROC_NAME_SIZE	64

typedef struct queue_st {
	unsigned long capacity;
	unsigned long bottom;
	unsigned long top;
	int state;
	int loop;
	void** buf;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
} queue_st;

union semun {
	int val;
	struct semid_ds* buf;
	ushort* array;
};

/* struct that is passed along with the data/signal between procs */
typedef struct {
	char proc_name_send[PROC_NAME_SIZE];	/* send proc name */
	char proc_name_recv[PROC_NAME_SIZE];	/* recv proc name */
	int msg_type;
	long msg_len;
	long msg_seq;
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
	int active;			/* flag for the process to signal active */
	char proc_name[PROC_NAME_SIZE];	/* process name */
	long send_count;	/* counter for sent data/signals */
	long recv_count;	/* counter for received data/signals */
} proc_entry;
#define SIZEOF_PROC_ENTRY sizeof(proc_entry)

static pthread_attr_t thread_attr_t;
static pthread_t recv_thread_t;
static pthread_t read_thread_t;
static pthread_t send_thread_t;

/* global pointer to the ctrl area */
void* shm_ctrl_ptr = NULL;
/* global mutex protecting the ctrl area */
int lock_ctrl_sem = 0;
/* internal memory view of the ctrl area */
mem_proc_entry* mem_entry = NULL;
/* initialize flag */
int initialized = 0;
/* my own process name */
char my_proc_name[PROC_NAME_SIZE];
/* my own process index */
int my_proc_index;
/* msg receive queue */
queue_st* recv_queue = NULL;
/* msg send queue */
queue_st* send_queue = NULL;
/* msg sent sequence */
long msg_sequence = 0;
/* number of process */
int num_of_procs = 0;
/* key for shm ctrl */
int shm_ctrl_key;
/* key for sem ctrl */
int sem_ctrl_key;
/* msg handle callback */
shm_callback_msg callbackmsg = NULL;
/* log handle callback */
shm_callback_log callbacklog = NULL;
/* print functions either syslog, printf of user specific */
int current_level = LOG_ERR;

static int print(int level, const char* format, ...);

static void* getqueue(queue_st* q);
static queue_st* queue_create(unsigned long capacity, int loop);
static int queue_put(queue_st* q, void* data);
static void* queue_get(queue_st* q);

static int create_lock(int key, int value);
static void lock(int sem);
static void unlock(int sem);
static int try_lock1(int sem);

static void init_mem_proc(int num);
static void populate_mem_proc(void);
static int clear_shm(int key, long size, void** shm);
static int get_shm(int key, long size, int mode, void** shm);

static proc_entry* get_proc_at(int index);
static int clear_proc_entry(int index);
static int get_next_free_index(void);
static void populate_mem_proc_single(int index);
static int inc_send_count(void);
static int inc_recv_count(void);

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

/********** msg queue functions ***********/

static void* getqueue(queue_st* q) {
	void* retval;
	if (NULL == q || QUEUE_EMPTY == q->state) {
		return NULL;
	}
	retval = q->buf[q->bottom];
	q->buf[q->bottom] = NULL;
	++q->bottom;
	if (q->bottom == q->capacity) {
		q->bottom = 0;
	}
	if (q->bottom == q->top) {
		q->state = QUEUE_EMPTY;
	}
	return retval;
}

static queue_st* queue_create(unsigned long capacity, int loop) {
	queue_st* q;
	if (capacity <= 0) {
		return NULL;
	}
    q = (queue_st*)malloc(sizeof(*q));
	q->capacity = capacity;
    q->bottom = 0;
    q->top = 0;
    q->state = QUEUE_EMPTY;
	q->loop = loop;
    q->buf = malloc(capacity * sizeof(void*));
    pthread_mutex_init(&q->mutex, NULL);
	pthread_cond_init(&q->cond, NULL);
    return q;
}

static int queue_put(queue_st* q, void* data) {
	void* retval = NULL;
	if (NULL == q || NULL == data) {
		return 1;
	}
	pthread_mutex_lock(&q->mutex);
	int prev_state = q->state;
	if (QUEUE_FULL == prev_state) {
		if (!q->loop) {
			return 2;
		}
		retval = getqueue(q);
		if (NULL != retval) {
			free(retval);
		}
	}
	q->buf[q->top] = data;
	++q->top;
	q->state = QUEUE_NORMAL;
	if (q->top == q->capacity) {
		q->top = 0;
	}
	if (q->top == q->bottom) {
		q->state = QUEUE_FULL;
	}
	pthread_cond_signal(&q->cond);
	pthread_mutex_unlock(&q->mutex);
	return 0;
}

static void* queue_get(queue_st* q) {
	void* retval;
	if (NULL == q) {
		return NULL;
	}
	pthread_mutex_lock(&q->mutex);
	if (QUEUE_EMPTY == q->state) {
		pthread_cond_wait(&q->cond, &q->mutex);
	}
	retval = getqueue(q);
	pthread_mutex_unlock(&q->mutex);
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
	op[0].sem_flg = 0;
	semop(sem, op, 1);
}

static void unlock(int sem) {
	struct sembuf op[1];
	op[0].sem_num = 0;
	op[0].sem_op = 1;
	op[0].sem_flg = 0;
	semop(sem, op, 1);
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
	mem_entry = (mem_proc_entry*)malloc(num * SIZEOF_MEM_PROC_ENTRY);
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

/********** the shared memory functions ***********/
static int clear_shm(int key, long size, void** shm) {
	int shmid;
	if ((shmid = shmget(key, size, IPC_CREAT | 0666)) < 0) {
		print(LOG_ERR, "Unable get shared mem for key %d, errno %d\n", key, errno);
		return 1;
	}
	if (NULL != *shm) {
		shmdt(*shm);
		*shm = NULL;
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
static int get_shm(int key, long size, int mode, void** shm) {
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
	if (NULL != *shm) {
		shmdt(*shm);
		*shm = NULL;
	}
	if ((void*)-1 == (*shm = shmat(shmid, NULL, 0))) {
		*shm = NULL;
		print(LOG_ERR, "Unable to alloc shared mem for key %d, errno %d\n", key, errno);
		return 1;
	}
	return 0;
}

static proc_entry* get_proc_at(int index) {
	void* tmp_ptr;
	proc_entry* entry;
	if (index < 0 || index >= num_of_procs) {
		return NULL;
	}
	tmp_ptr = shm_ctrl_ptr + (SIZEOF_PROC_ENTRY * index);
	entry = (proc_entry*)tmp_ptr;
	return entry;
}

static int clear_proc_entry(int index) {
	proc_entry* entry;
	print(LOG_DEBUG, "Removes proc from entry[%d] and memlist\n", index);
	entry = (proc_entry*)get_proc_at(index);
	clear_shm(entry->key_shm, entry->size_shm, &mem_entry[index].shm);
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
		if (0 != check_proc_entry(i)) {
			return i;
		}
	}
	return num_of_procs;
}

static int inc_send_count(void) {
	proc_entry* entry;
	/* The locks might not be needed TODO */
	/* lock(lock_ctrl_sem); */
	entry = (proc_entry*)get_proc_at(my_proc_index);
	entry->send_count++;
	/* unlock(lock_ctrl_sem); */
	return 0;
}

static int inc_recv_count(void) {
	proc_entry* entry;
	/* The locks might not be needed TODO */
	/* lock(lock_ctrl_sem); */
	entry = (proc_entry*)get_proc_at(my_proc_index);
	entry->recv_count++;
	/* unlock(lock_ctrl_sem); */
	return 0;
}

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
	entry->send_count = 0;
	entry->recv_count = 0;
	my_proc_index = index;
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

static void* recv_thread_func(void* arg) {
	header* hdr;
	void* msg;
	for (;;) {
		lock(mem_entry[my_proc_index].rlock);
		hdr = (header*)mem_entry[my_proc_index].shm;
		if (NULL == hdr || (0 == strlen(hdr->proc_name_send) && 0 == hdr->msg_type && 0 == hdr->msg_len && 0 == hdr->msg_seq)) {
			continue;
		}
		msg = malloc(SIZEOF_HEADER + hdr->msg_len);
		memcpy(msg, mem_entry[my_proc_index].shm, SIZEOF_HEADER + hdr->msg_len);
		memset(mem_entry[my_proc_index].shm, 0, SIZEOF_HEADER + hdr->msg_len);
		if (queue_put(recv_queue, (void*)msg)) {
			/* Failed to put in queue, msg lost */
			print(LOG_ERR, "Failed to put msg in queue, msg with type %d, seq %ld is lost!\n", hdr->msg_type, hdr->msg_seq);
			free(msg);
		} else {
			/* msg concidered received */
			inc_recv_count();
		}
		unlock(mem_entry[my_proc_index].wlock);
	}
	return (void*)0;
}

static void* read_thread_func(void* arg) {
	void* msg;
	header* hdr;
	for (;;) {
		msg = queue_get(recv_queue);
		if (NULL == msg) {
			continue;
		}
		hdr = (header*)msg;
		if (NULL == hdr) {
			free(msg);
			continue;
		}
		if (NULL != callbackmsg) {
			callbackmsg(hdr->proc_name_send, hdr->msg_type, hdr->msg_len, hdr->msg_len > 0 ? (msg + SIZEOF_HEADER) : NULL);
		} else {
			print(LOG_WARNING, "No callback\n");
		}
		free(msg);
	}
	return (void*)0;
}

static void* send_thread_func(void* arg) {
	void* msg;
	header* hdr;
	for (;;) {
		msg = queue_get(send_queue);
		if (NULL == msg) {
			continue;
		}
		hdr = (header*)msg;
		if (NULL == hdr) {
			free(msg);
			continue;
		}
		shm_send(hdr->proc_name_recv, hdr->msg_type, hdr->msg_len, hdr->msg_len > 0 ? (msg + SIZEOF_HEADER) : NULL);
		free(msg);
	}
	return (void*)0;
}

static int start_listen_thread(void) {
	if (0 != pthread_attr_init(&thread_attr_t)) {
		print(LOG_ERR, "Unable to init thread attribute, errno %d\n", errno);
		return 1;
	}
	if (0 != pthread_attr_setdetachstate(&thread_attr_t, PTHREAD_CREATE_DETACHED)) {
		print(LOG_ERR, "Unable to set detached state to thread, errno %d\n", errno);
		return 1;
	}
	if (0 != pthread_attr_setinheritsched(&thread_attr_t, PTHREAD_INHERIT_SCHED)) {
		print(LOG_ERR, "Unable to set inherit scheduling, errno %d\n", errno);
		return 1;
	}
	if (0 != pthread_create(&recv_thread_t, &thread_attr_t, recv_thread_func, (void*)NULL)) {
		print(LOG_ERR, "Unable to create worker thread for recv, errno %d\n", errno);
		return 1;
	}
	if (0 != pthread_create(&read_thread_t, &thread_attr_t, read_thread_func, (void*)NULL)) {
		print(LOG_ERR, "Unable to create worker thread for read, errno %d\n", errno);
		return 1;
	}
	if (0 != pthread_create(&send_thread_t, &thread_attr_t, send_thread_func, (void*)NULL)) {
		print(LOG_ERR, "Unable to create worker thread for send, errno %d\n", errno);
		return 1;
	}
	return 0;
}

/****************************** API ******************************/

int set_print_level(int level) {
	if (level >= LOG_EMERG && level <= LOG_DEBUG) {
		current_level = level;
		return 0;
	}
	return 1;
}

int init_memshare(const char* proc_name, int proc_num, int shm_key, long shm_size, int queue_capacity, shm_callback_msg scbm, shm_callback_log scbl) {
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
	if (proc_num < 2) {
		print(LOG_ERR, "proc num '%d' must >= 2\n", proc_num);
		return 3;
	}
	if (shm_key < 0) {
		print(LOG_ERR, "shm key '%d' must >= 0\n", shm_key);
		return 4;
	}
	if (shm_size <= 0) {
		print(LOG_ERR, "shm size '%ld' must > 0\n", shm_size);
		return 5;
	}
	if (queue_capacity <= 0) {
		print(LOG_ERR, "queue capacity '%d' must > 0\n", queue_capacity);
		return 6;
	}
	memcpy(my_proc_name, proc_name, PROC_NAME_SIZE);
	shm_ctrl_key = shm_key;
	sem_ctrl_key = shm_key + 1;
	recv_queue = queue_create(queue_capacity, 1);
	send_queue = queue_create(queue_capacity, 1);
	callbackmsg = scbm;
	callbacklog = scbl;
	/* clear the memory view */
	init_mem_proc(proc_num);
	/* start off by locking the ctrl lock */
	if (-1 == (lock_ctrl_sem = create_lock(sem_ctrl_key, 1))) {
		print(LOG_ERR, "Unable to create semaphore\n");
		return 7;
	}
	print(LOG_DEBUG, "Created ctrl lock\n");
	lock(lock_ctrl_sem);
	print(LOG_DEBUG, "Init_memshare ctrl\n");
	/* map up the ctrl area */
	if (1 == get_shm(shm_ctrl_key, SIZEOF_PROC_ENTRY * proc_num, 1, &shm_ctrl_ptr)) {
		print(LOG_ERR, "Unable to alloc shared mem\n");
		return 8;
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

int shm_send(const char* proc_name_recv, int msg_type, long msg_len, const void* data) {
	int index;
	header hdr;
	proc_entry* entry;
	if (NULL == proc_name_recv || 0 == strlen(proc_name_recv)) {
		print(LOG_ERR, "Recv proc name is NULL\n");
		return 1;
	}
	if (strlen(proc_name_recv) > PROC_NAME_SIZE) {
		print(LOG_ERR, "Recv proc name '%s' length > %d\n", proc_name_recv, PROC_NAME_SIZE);
		return 1;
	}
	if ((index = get_proc_index(proc_name_recv)) < 0) {
		print(LOG_NOTICE, "No such process %s\n", proc_name_recv);
		return 2;
	}
	msg_len = msg_len >= 0 ? msg_len : 0;
	entry = get_proc_at(index);
	if (SIZEOF_HEADER + msg_len > entry->size_shm) {
		print(LOG_NOTICE, "Data size %ld large shm size %ld\n", SIZEOF_HEADER + msg_len, entry->size_shm);
		return 3;
	}
	print(LOG_DEBUG, "Sending data to %s at index %d\n", proc_name_recv, index);
	lock(mem_entry[index].wlock);
	memcpy(hdr.proc_name_send, my_proc_name, PROC_NAME_SIZE);
	memcpy(hdr.proc_name_recv, proc_name_recv, PROC_NAME_SIZE);
	hdr.msg_type = msg_type;
	hdr.msg_len = msg_len;
	hdr.msg_seq = msg_sequence++;
	memcpy(mem_entry[index].shm, &hdr, SIZEOF_HEADER);
	if (msg_len > 0 && NULL != data) {
		memcpy(mem_entry[index].shm + SIZEOF_HEADER, data, msg_len);
	}
	inc_send_count();
	unlock(mem_entry[index].rlock);
	return 0;
}

void shm_send_nio(const char* proc_name_recv, int msg_type, long msg_len, const void* data) {
	header hdr;
	void* msg;
	if (NULL == proc_name_recv || 0 == strlen(proc_name_recv)) {
		print(LOG_ERR, "Recv proc name is NULL\n");
		return;
	}
	if (strlen(proc_name_recv) > PROC_NAME_SIZE) {
		print(LOG_ERR, "Recv proc name '%s' length > %d\n", proc_name_recv, PROC_NAME_SIZE);
		return;
	}
	msg_len = msg_len >= 0 ? msg_len : 0;
	memcpy(hdr.proc_name_recv, proc_name_recv, PROC_NAME_SIZE);
	hdr.msg_type = msg_type;
	hdr.msg_len = msg_len;
	msg = malloc(SIZEOF_HEADER + hdr.msg_len);
	memcpy(msg, &hdr, SIZEOF_HEADER);
	if (msg_len > 0 && NULL != data) {
		memcpy(msg + SIZEOF_HEADER, data, msg_len);
	}
	queue_put(send_queue, msg);
}

int check_proc_entry(int index) {
	/* compare mem_entry with shared memory and see if process is active */
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
	if (NULL == proc_name || 0 == strlen(proc_name)) {
		return -1;
	}
	for (i = 0; i < num_of_procs; ++i) {
		if (0 == check_proc_entry(i)) {
			if (!strcmp(mem_entry[i].proc_name, proc_name)) {
				return i;
			}
		}
	}
	return -1;
}

int get_proc_info(int index, char** proc_name, long* data_size, long* send_count, long* recv_count) {
	proc_entry* entry;
	/* this call will not check if the entry is active */
	entry = get_proc_at(index);
	if (NULL == entry) {
		return 1;
	}
	*proc_name = entry->proc_name;
	*data_size = entry->size_shm;
	*send_count = entry->send_count;
	*recv_count = entry->recv_count;
	return 0;
}

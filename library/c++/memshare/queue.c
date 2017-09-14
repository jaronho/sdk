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

#include "queue.h"
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <pthread.h>
#include <semaphore.h>

#ifndef NULL
#define NULL	0
#endif

#define EMPTY	0
#define NORMAL	1
#define FULL	2

int queues_num;

sem_t* qlock;
pthread_mutex_t* condition_mutex;
pthread_cond_t* condition_cond;
int* low_bottom;
int* low_top;
int* low_totalsize;
int* low_state;
int* hi_bottom;
int* hi_top;
int* hi_totalsize;
int* hi_state;
int* seize_queue_state;
int* seize_queue_key;
void*** low_queuebase;
void*** hi_queuebase;

int queue_state = 0;

static int lo_qinit(int index, int size);
static int hi_qinit(int index, int size);
static void* getloq(int index);
static void* gethiq(int index);

/*****************************************************************************/
/* Description        : This function initializes a queue and sets           */
/*                      the depth(size), called from seize_queus             */
/* Input(s)           : index and size                                       */
/* Return Value(s)    : 0 ok                                                 */
/*                      1 memory allocation failed                           */
/*****************************************************************************/
static int lo_qinit(int index, int size) {
	if (NULL != (low_queuebase[index] = malloc(size * sizeof(void*)))) {
		/* initalize all stuff */
		low_bottom[index] = 0;
		low_top[index] = 0;
		low_totalsize[index] = size;
		low_state[index] = EMPTY;
		sem_init(&qlock[index], 0, 1);
		return 0;
	}
	return 1;
}

/*****************************************************************************/
/* Description        : This function initializes a queue and sets           */
/*                      the depth(size), called from seize_queus             */
/* Input(s)           : index and size                                       */
/* Return Value(s)    : 0 ok                                                 */
/*                      1 memory allocation failed                           */
/*****************************************************************************/
static int hi_qinit(int index, int size) {
	if (NULL != (hi_queuebase[index] = malloc(size * sizeof(void*)))) {
		hi_bottom[index] = 0;
		hi_top[index] = 0;
		hi_totalsize[index] = size;
		hi_state[index] = EMPTY;
		sem_init(&qlock[index], 0, 1);
		return 0;
	}
	return 1;
}

/*****************************************************************************/
/* Description        : Internal function                                    */
/* Input(s)           : index                                                */
/* Return Value(s)    :                                                      */
/*****************************************************************************/
static void* getloq(int index) {
	void* retval;
	retval = low_queuebase[index][low_bottom[index]];
	low_queuebase[index][low_bottom[index]] = NULL;
	++low_bottom[index];
	if (low_bottom[index] == low_totalsize[index]) {
		low_bottom[index] = 0;
	}
	if (low_bottom[index] == low_top[index]) {
		low_state[index] = EMPTY;
	}
	return retval;
}

/*****************************************************************************/
/* Description        : Internal function                                    */
/* Input(s)           : index                                                */
/* Return Value(s)    :                                                      */
/*****************************************************************************/
static void* gethiq(int index) {
	void* retval;
	retval = hi_queuebase[index][hi_bottom[index]];
	hi_queuebase[index][hi_bottom[index]] = NULL;
	++hi_bottom[index];
	if (hi_bottom[index] == hi_totalsize[index]) {
		hi_bottom[index] = 0;
	}
	if (hi_bottom[index] == hi_top[index]) {
		hi_state[index] = EMPTY;
	}
	return retval;
}

/*****************************************************************************/
/* Description        : This function initialises the queue seize env        */
/* Input(s)           : num of queues                                        */
/* Output(s)          : None.                                                */
/* Return Value(s)    : None.                                                */
/*****************************************************************************/
void init_queues(int num) {
	int i;
	num = num > 0 ? num : 1;
	queues_num = num;
	qlock = (sem_t*)malloc(num * sizeof(sem_t));
	condition_mutex = (pthread_mutex_t*)malloc(num * sizeof(pthread_mutex_t));
	condition_cond = (pthread_cond_t*)malloc(num * sizeof(pthread_cond_t));
	low_bottom = (int*)malloc(num * sizeof(int));
	low_top = (int*)malloc(num * sizeof(int));
	low_totalsize = (int*)malloc(num * sizeof(int));
	low_state = (int*)malloc(num * sizeof(int));
	hi_bottom = (int*)malloc(num * sizeof(int));
	hi_top = (int*)malloc(num * sizeof(int));
	hi_totalsize = (int*)malloc(num * sizeof(int));
	hi_state = (int*)malloc(num * sizeof(int));
	seize_queue_state = (int*)malloc(num * sizeof(int));
	seize_queue_key = (int*)malloc(num * sizeof(int));
	low_queuebase = (void***)malloc(num * sizeof(void**));
	hi_queuebase = (void***)malloc(num * sizeof(void**));
	for (i = 0; i < num; ++i) {
		pthread_mutex_init(&condition_mutex[i], NULL);
		pthread_cond_init(&condition_cond[i], NULL);
		seize_queue_state[i] = 0;
		seize_queue_key[i] = 0;
	}
	queue_state = 1;
}

/*****************************************************************************/
/* Description        : This function reserves a queue by a key and sets     */
/*                      the depth(size)                                      */
/* Input(s)           : key for identification(> 0), size of queue           */
/* Output(s)          : index to be used for quick reference                 */
/* Return Value(s)    : 0 ok (trust the index)                               */
/*                      1 key already seized,size ignored (trust the index)  */
/*                  (for rest of the return codes the index can't be trusted */
/*                      2 queue not initialized                              */
/*                      3 no queues avaliable                                */
/*                      4 sanity check of parameters failed                  */
/*****************************************************************************/
int seize_queue(int* index, int key, int size) {
	int i;
	if (0 == queue_state) {
		return 2;
	}
	if (0 == index || 0 == key || 0 == size) {
		return 4;
	}
	/* search for already seized */
	for (i = 0; i < queues_num; ++i) {
		if (1 == seize_queue_state[i] && key == seize_queue_key[i]) {
			*index = i;
			return 1;
		}
	}
	/* search for a free entry */
	for (i = 0; i < queues_num; ++i) {
		if (0 == seize_queue_state[i]) {
			seize_queue_state[i] = 1;
			/* Found an index initalize the queues for that index */
			lo_qinit(i, size);
			hi_qinit(i, size);
			seize_queue_key[i] = key;
			*index = i;
			return 0;
		}
	}
	return 3;
}

/*****************************************************************************/
/* Description        : This function adds a pointer to a low prio queue     */
/* Input(s)           : index and a pointer to a pointer                     */
/* Return Value(s)    : 0 ok                                                 */
/*                      1 entry has to be a pointer                          */
/*****************************************************************************/
int lo_qadd(int index, void** entry) {
	void* retval = NULL;
	if (NULL == entry) {
		return 1;
	}
	sem_wait(&qlock[index]);
	int prev_state = low_state[index];
	if (FULL == low_state[index]) {
		retval = getloq(index);
		if (NULL != retval) {
			free(retval);
		}
	}
	low_queuebase[index][low_top[index]] = (void*)*entry;
	++low_top[index];
	low_state[index] = NORMAL;
	if (low_top[index] == low_totalsize[index]) {
		low_top[index] = 0;
	}
	if (low_top[index] == low_bottom[index]) {
		low_state[index] = FULL;
	}
	pthread_mutex_lock(&condition_mutex[index]);
	if (EMPTY == prev_state && EMPTY == hi_state[index]) {
		pthread_cond_signal(&condition_cond[index]);
	}
	pthread_mutex_unlock(&condition_mutex[index]);
	sem_post(&qlock[index]);
	return 0;
}

/*****************************************************************************/
/* Description        : This function adds a pointer to a high prio queue    */
/* Input(s)           : index and a pointer to a pointer                     */
/* Return Value(s)    : 0 ok                                                 */
/*                      1 entry has to be a pointer                          */
/*****************************************************************************/
int hi_qadd(int index, void** entry) {
	void* retval = NULL;
	if (NULL == entry) {
		return 1;
	}
	sem_wait(&qlock[index]);
	int prev_state = hi_state[index];
	if (FULL == hi_state[index]) {
		retval = gethiq(index);
		if (NULL != retval) {
			free(retval);
		}
	}
	hi_queuebase[index][hi_top[index]] = (void*)*entry;
	++hi_top[index];
	hi_state[index] = NORMAL;
	if (hi_top[index] == hi_totalsize[index]) {
		hi_top[index] = 0;
	}
	if (hi_top[index] == hi_bottom[index]) {
		hi_state[index] = FULL;
	}
	pthread_mutex_lock(&condition_mutex[index]);
	if (EMPTY == prev_state && EMPTY == low_state[index]) {
		pthread_cond_signal(&condition_cond[index]);
	}
	pthread_mutex_unlock(&condition_mutex[index]);
	sem_post(&qlock[index]);
	return 0;
}

/*****************************************************************************/
/* Description        : This function picks the next entry in the indexed    */
/*                      queue, preferably the hi_queue.                      */
/*                      The call will block until an entry is available      */
/* Input(s)           : index                                                */
/* Return Value(s)    : A pointer to a buffer                                */
/*****************************************************************************/
void* qget(int index) {
	void* retval = NULL;
	static int count;
	pthread_mutex_lock(&condition_mutex[index]);
	if (EMPTY == low_state[index] && EMPTY == hi_state[index]) {
		pthread_cond_wait(&condition_cond[index], &condition_mutex[index]);
	}
	pthread_mutex_unlock(&condition_mutex[index]);
	sem_wait(&qlock[index]);
	/* pick every 10th packet first from loq */
	++count;
	if (10 == count) {
		if (EMPTY != low_state[index]) {
			retval = getloq(index);
		} else if (EMPTY != hi_state[index]) {
			retval = gethiq(index);
		}
		count = 0;
	} else {
		if (EMPTY != hi_state[index]) {
			retval = gethiq(index);
		} else if (EMPTY != low_state[index]) {
			retval = getloq(index);
		}
	}
	sem_post(&qlock[index]);
	return retval;
}

/*****************************************************************************/
/* Description        : This function peeks for the next entry in the        */
/*                      indexed queue, preferably the hi_queue.              */
/* Input(s)           : index                                                */
/* Return Value(s)    : 0 no entry available in any (hi or lo) queue         */
/*                      1 An entry returned in buff                          */
/*****************************************************************************/
int qpeek(int index, void** buff) {
	pthread_mutex_lock(&condition_mutex[index]);
	if (EMPTY != hi_state[index]) {
		*buff = hi_queuebase[index][hi_bottom[index]];
		pthread_mutex_unlock(&condition_mutex[index]);
		return 1;
	} else if (EMPTY != low_state[index]) {
		*buff = low_queuebase[index][low_bottom[index]];
		pthread_mutex_unlock(&condition_mutex[index]);
		return 1;
	}
	pthread_mutex_unlock(&condition_mutex[index]);
	return 0;
}

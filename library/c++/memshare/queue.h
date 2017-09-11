/*
 * Memshare, quick and easy IPC.
 * Copyright (C) 2012  Tommy Wiklund
 */

#ifndef _QUEUE_H_
#define _QUEUE_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
/* Description        : This function initialises the queue seize env        */
/* Input(s)           : num of queues                                        */
/* Output(s)          : None.                                                */
/* Return Value(s)    : None.                                                */
/*****************************************************************************/
extern void init_queues(int num);

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
extern int seize_queue(int* index, int key, int size);

/*****************************************************************************/
/* Description        : This function adds a pointer to a low prio queue     */
/* Input(s)           : index and a pointer to a pointer                     */
/* Return Value(s)    : 0 ok                                                 */
/*                      1 entry has to be a pointer                          */
/*                      2 queue full, the enty is dropped                    */
/*****************************************************************************/
extern int lo_qadd(int index, void** entry);

/*****************************************************************************/
/* Description        : This function adds a pointer to a high prio queue    */
/* Input(s)           : index and a pointer to a pointer                     */
/* Return Value(s)    : 0 ok                                                 */
/*                      1 entry has to be a pointer                          */
/*                      2 queue full, the enty is dropped                    */
/*****************************************************************************/
extern int hi_qadd(int index, void** entry);

/*****************************************************************************/
/* Description        : This function picks the next entry in the indexed    */
/*                      queue, preferably the hi_queue.                      */
/*                      The call will block until an entry is available      */
/* Input(s)           : index                                                */
/* Return Value(s)    : A pointer to a buffer                                */
/*****************************************************************************/
extern void* qget(int index);

/*****************************************************************************/
/* Description        : This function peeks for the next entry in the        */
/*                      indexed queue, preferably the hi_queue.              */
/* Input(s)           : index                                                */
/* Return Value(s)    : 0 no entry available in any (hi or lo) queue         */
/*                      1 An entry returned in buff                          */
/*****************************************************************************/
extern int qpeek(int index, void** buff);

#ifdef __cplusplus
}
#endif

#endif	// _QUEUE_H_

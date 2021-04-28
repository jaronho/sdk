/* ipqmpd - ip queuing multiplex daemon
 *
 *  (C) 2000 by Harald Welte <laforge@sunbeam.franken.de>
 *
 *  This code is released under the terms of GNU GPL
 *
 *  ipq.h,v 1.1 2000/08/19 17:17:57 laforge Exp
 */

#ifndef _IPQ_H
#define _IPQ_H

#include <libipq/libipq.h>
#include "queue.h"

struct ipq_handle *qh;
queue_t *kernelq;

int ipq_inp(void);
int ipq_outp(void);
int ipqmpd_recalc_mode(void);

#endif /* _IPQ_H */

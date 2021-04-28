/* ipqmpd - ip queuing multiplex daemon
 *
 *  (C) 2000 by Harald Welte <laforge@sunbeam.franken.de>
 *
 *  This code is released under the terms of GNU GPL
 *
 *  ctlfd.h,v 1.1 2000/08/19 17:17:57 laforge Exp
 */

#ifndef _CTLFD_H
#define _CTLFD_H

int ctlfd;

/* register master control socket, bind it, listen on it */
int ctlfd_register(void);

/* handle input from ctlfd */
int ctlfd_inp(void);

#endif /* _CTLFD_H */

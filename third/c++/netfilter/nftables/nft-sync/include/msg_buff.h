#ifndef _MSG_BUFF_H_
#define _MSG_BUFF_H_

#include <stdint.h>

struct msg_buff;

struct msg_buff *msgb_alloc(uint32_t size);
void msgb_free(struct msg_buff *msgb);

uint32_t msgb_len(struct msg_buff *msgb);
uint32_t msgb_size(struct msg_buff *msgb);

unsigned char *msgb_data(struct msg_buff *msgb);
unsigned char *msgb_tail(struct msg_buff *msgb);

void *msgb_put(struct msg_buff *msgb, uint32_t len);
void *msgb_pull(struct msg_buff *msgb, uint32_t len);
void msgb_burp(struct msg_buff *msgb);

#endif

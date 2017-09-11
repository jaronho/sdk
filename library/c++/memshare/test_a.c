#include "memshare.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

typedef struct {
	int a;
	long b;
	float c;
	double d;
	char s[128];
} Info;

void msg_callback(const char* proc, int msg_type, int msg_len, const void* data) {
	Info* info;
	int retvalue = 0;
	printf("receive [%s], msg_type => %d, msg_len => %d \n", proc, msg_type, msg_len);
	if (1 == msg_type) {
		info = (Info*)data;
		printf("receive data => %d, %ld, %f, %f, %s \n", info->a, info->b, info->c, info->d, info->s);
		retvalue = send_msg(proc, 7, msg_len, data);
		if (retvalue) {
			printf("replay:data failed with %d\n", retvalue);
		}
	}
}

int main(int argc, char *argv[]) {
	set_print_level(0);
	printf("test_a started\n");

	if (init_memshare("test_a", DEF_PROC_NUM, DEF_SHM_KEY, DEF_SHM_SIZE, DEF_QUEUE_SIZE)) {
		printf("Failed to init memshare\n");
		exit(1);
	}
	
	register_msg(msg_callback);

	while (1) {
		sleep(1);
		send_msg("test_b", 3, 0, NULL);
		send_msg("test_c", 4, 0, NULL);
	}
}

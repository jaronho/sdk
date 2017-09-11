#include "memshare.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

typedef struct {
	int a;
	long b;
	float c;
	double d;
	char s[64];
} Info;

void msg_callback(const char* proc, int msg_type, long msg_len, const void* data) {
	Info* info;
	printf("receive [%s], msg_type => %d, msg_len => %ld \n", proc, msg_type, msg_len);
	if (7 == msg_type) {
		info = (Info*)data;
		printf("receive data => %d, %ld, %f, %f, %s \n", info->a, info->b, info->c, info->d, info->s);
	}
}

int main(int argc, char *argv[]) {
	set_print_level(0);
	printf("test_b started\n");

	if (init_memshare("test_b", DEF_PROC_NUM, DEF_SHM_KEY, DEF_SHM_SIZE, DEF_QUEUE_SIZE)) {
		printf("Failed to init memshare\n");
		exit(1);
	}
	
	register_msg(msg_callback);
	
	char m[] = "hello, world.";
	while (1) {
		sleep(1);
		Info* i = (Info*)malloc(sizeof(Info));
		i->a = 1;
		i->b = 1000;
		i->c = 0.6;
		i->d = 1.0001;
		memcpy(i->s, m, strlen(m) + 1);
		send_msg("test_a", 1, sizeof(*i), (void*)i);
		free(i);
		send_msg("test_c", 2, 0L, NULL);
	}

	return 0;
}

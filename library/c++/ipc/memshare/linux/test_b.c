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
	char s[64];
} Info;

void msg_callback(const char* proc_name, int msg_type, long msg_len, const void* data) {
	Info* info;
	//printf("receive [%s], msg_type => %d, msg_len => %ld \n", proc_name, msg_type, msg_len);
	if (7 == msg_type) {
		info = (Info*)data;
		//printf("receive data => %d, %ld, %f, %f, %s \n", info->a, info->b, info->c, info->d, info->s);
	}
}

void log_callback(int level, const char* format, va_list ap) {
    if (level <= LOG_INFO) {
        vprintf(format, ap);
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
	set_print_level(0);
	printf("test_b started\n");

	if (init_memshare("test_b", 3, DEF_SHM_KEY, DEF_SHM_SIZE, DEF_QUEUE_SIZE, msg_callback, log_callback)) {
		printf("Failed to init memshare\n");
		exit(1);
	}
	
	char m[] = "hello, world.";
	while (1) {
		usleep(100 * 1000);
		Info* i = (Info*)malloc(sizeof(Info));
		i->a = 1;
		i->b = 1000;
		i->c = 0.6;
		i->d = 1.0001;
		memcpy(i->s, m, strlen(m) + 1);
		shm_send("test_a", 1, sizeof(*i), (void*)i);
		free(i);
		shm_send("test_c", 2, 0L, NULL);
	}

	return 0;
}

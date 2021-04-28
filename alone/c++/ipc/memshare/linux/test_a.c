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
	int retvalue = 0;
	//printf("receive [%s], msg_type => %d, msg_len => %ld \n", proc_name, msg_type, msg_len);
	if (1 == msg_type) {
		info = (Info*)data;
		//printf("receive data => %d, %ld, %f, %f, %s \n", info->a, info->b, info->c, info->d, info->s);
		retvalue = shm_send(proc_name, 7, msg_len, data);
		if (retvalue) {
			//printf("replay:data failed with %d\n", retvalue);
		}
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
	printf("test_a started\n");

	if (init_memshare("test_a", 3, DEF_SHM_KEY, DEF_SHM_SIZE, DEF_QUEUE_SIZE, msg_callback, log_callback)) {
		printf("Failed to init memshare\n");
		exit(1);
	}

	while (1) {
		usleep(100 * 1000);
		shm_send("test_b", 3, 0L, NULL);
		shm_send("test_c", 4, 0L, NULL);
	}
}

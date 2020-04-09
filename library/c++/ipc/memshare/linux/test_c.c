#include "memshare.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

void msg_callback(const char* proc_name, int msg_type, long msg_len, const void* data) {
	//printf("receive [%s], msg_type => %d, msg_len => %ld \n", proc_name, msg_type, msg_len);
}

void log_callback(int level, const char* format, va_list ap) {
    if (level <= LOG_INFO) {
        vprintf(format, ap);
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
	set_print_level(0);
	printf("test_c started\n");

	if (init_memshare("test_c", 3, DEF_SHM_KEY, DEF_SHM_SIZE, DEF_QUEUE_SIZE, msg_callback, log_callback)) {
		printf("Failed to init memshare\n");
		exit(1);
	}
	
	while (1) {
		usleep(100 * 1000);
		shm_send("test_a", 5, 0L, NULL);
		shm_send("test_b", 6, 0L, NULL);
	}

	return 0;
}

#include "memshare.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

void msg_callback(const char* proc, int msg_type, long msg_len, const void* data) {
	printf("receive [%s], msg_type => %d, msg_len => %ld \n", proc, msg_type, msg_len);
}

int main(int argc, char *argv[]) {
	set_print_level(0);
	printf("test_c started\n");

	if (init_memshare("test_c", DEF_PROC_NUM, DEF_SHM_KEY, DEF_SHM_SIZE, DEF_QUEUE_SIZE)) {
		printf("Failed to init memshare\n");
		exit(1);
	}
	
	register_msg(msg_callback);
	
	while (1) {
		sleep(1);
		send_msg("test_a", 5, 0L, NULL);
		send_msg("test_b", 6, 0L, NULL);
	}

	return 0;
}

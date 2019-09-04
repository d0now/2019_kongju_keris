
/*
 * Stealien @ 314ckC47
 * oneshot.c - kongju keris pwnable task.
 * $ gcc -o oneshot oneshot.c -no-pie
*/

#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>

bool read_ptr(void);
bool write_ptr(void);

int main(void) {

	void *ptr = NULL;
	
	if (read_ptr() || write_ptr())
		goto err;

	return 0;
err:
	return -1;
}

bool read_ptr(void) {

	void *ptr = NULL;

	if (read(0, &ptr, sizeof(uint64_t)) != sizeof(uint64_t))
		goto err;

	write(1, ptr, sizeof(uint64_t));

	return false;
err:
	return true;
}

bool write_ptr(void) {

	void *ptr = NULL;

	if (read(0, &ptr, sizeof(uint64_t)) != sizeof(uint64_t))
		goto err;
	usleep(1);
	read(0, ptr, sizeof(uint64_t));

	return false;
err:
	return true;
}

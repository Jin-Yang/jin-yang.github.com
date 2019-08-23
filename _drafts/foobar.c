#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE (8 * 1024 * 1024)
#define LOOPS    16

char arr[BUF_SIZE] __attribute__((__aligned__((64)), __section__(".data.cacheline_aligned"))) ;

int main(int argc, char **argv)
{
	(void) argc;
	int i, j, step;

	step = atoi(argv[1]);

	for (i = 0; i < LOOPS; i++)
		for (j = 0; j < BUF_SIZE; j += step)
			arr[j] = 3;
	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#define NUMS  (64 * 1024 * 1024)

int main(void)
{
	int *arr, i, j, rc;
	struct timeval beg, end, diff;

	//void * memalign (size_t boundary, size_t size)

	rc = posix_memalign((void **)&arr, 4096, NUMS * sizeof(int));
	if (arr == NULL)
		return -1;
printf("xxxxxxxxxxxx %p\n", arr);

	for (j = 1; j < 2; j++) {
j = 1;
		for (i = 0; i < NUMS; i++)
			arr[i] = i;

		gettimeofday(&beg, NULL);
		for (i = 0; i < NUMS; i += j)
			arr[i]++;
		gettimeofday(&end, NULL);
		timersub(&end, &beg, &diff);
		fprintf(stderr, "%02d round time consume %ld.%06lds.\n",
				j, diff.tv_sec, diff.tv_usec);
	}

	return 0;
}

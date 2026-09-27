#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>

/* gigs start in 2007 */
int counts[20];


int main(int argc, char *argv[])
{
	int rc = 0;
	FILE *fp = fopen("gigs", "r");
	if (!fp) {
		perror("gigs");
		exit(1);
	}

	char line[256];
	int nline = 0;
	while (fgets(line, sizeof(line), fp)) {
		++nline;

		char *dstr = strtok(line, "/");
		char *ystr = strtok(NULL, " ");
		if (!dstr || !ystr) {
			printf("PROBLEMS\n");
			rc = 1;
			continue;
		}

		int year = strtol(ystr, NULL, 10);
		if (year < 7 || year > 27) {
			printf("BAD YEAR %d (%s) %d\n", year, ystr, nline);
			rc = 1;
			continue;
		}

		counts[year - 7]++;
	}

	fclose(fp);

	int i, total = 0;
	for (i = 0; i < 20 && counts[i]; ++i) {
		total += counts[i];
		printf("%d: %2d\n", i + 2007, counts[i]);
	}

	printf("Average: %.0f\n", (double)total / (double)i);
	return rc;
}

/*
 * Local Variables:
 * compile-command: "gcc -O3 -Wall count-gigs.c -o count-gigs"
 * End:
 */

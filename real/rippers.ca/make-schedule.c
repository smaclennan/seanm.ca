#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>

#define NROWS 12
//#define ADD_YEARS

struct gig {
	int year;
	int month;
	int day;
	int is_year;
	int upcoming;
	char *perf;
	struct gig *next;
} *gigs, *gig_tail;
int n_gigs;

static const char *months[] = { "Jan", "Feb", "March", "April", "May", "June",
							  "July", "Aug", "Sep", "Oct", "Nov", "Dec" };

static int upcoming(struct gig *gig)
{
	static int now_year = -1, now_month, now_day;
	if (now_year == -1) {
		time_t now = time(NULL);
		struct tm *tm = localtime(&now);
		if (!tm) {
			perror("localtime");
			exit(1);
		}
		now_year = tm->tm_year % 100;
		now_month = tm->tm_mon;
		now_day = tm->tm_mday;
	}

	if (gig->is_year)
		return 0;

	if (gig->year > now_year)
		return 1;

	if (gig->year == now_year) {
		if (gig->month > now_month)
			return 1;
		if (gig->month == now_month)
			if (gig->day >= now_day)
				return 1;
	}

	return 0;
}

static void parse_date(const char *date, struct gig *gig)
{
	char *p = (char *)date;
	int i, n = 0, err = 1;

	if ((i = strtol(date, NULL, 10)) > 0) {
		gig->year = i;
		gig->is_year = 1;
		return;
	}

	while (isalpha(*p)) {
		++p;
		++n;
	}

	for (i = 0; i < 12; ++i)
		if (strncmp(date, months[i], n) == 0) {
			gig->month = i;
			break;
		}
	if (i == 12)
		goto bad_date;

	++err;
	gig->day = strtol(p, &p, 10);
	if (gig->day <= 0 || gig->day > 31 || *p != '/')
		goto bad_date;

	++err;
	gig->year = strtol(p + 1, NULL, 10);
	if (gig->year < 07 || gig->year > 99)
		goto bad_date;

	gig->upcoming = upcoming(gig);

	return;

bad_date:
	printf("Bad date %d: '%s'\n", err, date);
	exit(2);
}

static struct gig *add_gig(char *date, char *perf)
{
	struct gig *new = calloc(1, sizeof(struct gig));
	if (!new) {
		puts("Out of memory");
		exit(1);
	}
	parse_date(date, new);
	if (perf) {
		new->perf = strdup(perf);
		if (!new->perf) {
			puts("Out of memory");
			exit(1);
		}
	} else
		new->is_year = 1;
	if (!gigs)
		gigs = new;
	else
		gig_tail->next = new;
	gig_tail = new;
	++n_gigs;

	return new;
}

static void out_td(FILE *out, struct gig *gig)
{
	char date[80];

#ifdef ADD_YEARS
	sprintf(date, "%s&nbsp;%d", months[gig->month], gig->day);
#else
	sprintf(date, "%s&nbsp;%02u/%u", months[gig->month], gig->day, gig->year);
#endif

	if (gig->is_year)
		fprintf(out, "<td class=year colspan=2>%u\n", gig->year);
	else if (gig->upcoming) {
		if (strncmp(gig->perf, "<a ", 3) == 0)
			fprintf(out, "  <td class=update>%s\n  <td class=upperf><a class=\"up\" %s\n",
					date, gig->perf + 3);
		else
			fprintf(out, "  <td class=update>%s\n  <td class=upperf>%s\n",
					date, gig->perf);
	} else
		fprintf(out, "  <td class=date>%s\n  <td class=perf>%s\n",
				date, gig->perf);
}

static void out_table(FILE *in, FILE *out, struct tm *tm, int nrows, int drop)
{
	struct gig *col1, *col2;
	char line[1024];
	int i;

	while (fgets(line, sizeof(line), in)) {
		fputs(line, out);
		if (strncmp(line, "<!-- Schedule table -->", 23) == 0) {
			if (drop) {
				for (col1 = gigs; drop-- > 0; col1 = col1->next) ;
				i = nrows;
				for (col2 = col1; i-- > 0; col2 = col2->next);
			} else {
				col1 = gigs;
				col2 = NULL;
			}

			for (i = 0; i < nrows; ++i) {
				fprintf(out, "<tr>\n");
				out_td(out, col1);
				col1 = col1->next;
				if (col2) {
					out_td(out, col2);
					col2 = col2->next;
				}
			}
		} else if (strstr(line, "<!-- copyright -->"))
			fprintf(out, "\tCopyright Rippers 2008-%d. All rights reserved.<br>\n",
					tm->tm_year + 1900);
	}
}

static void out_gigs(struct tm *tm)
{
	FILE *in = fopen("all-template.htm", "r");
	if (!in) {
		perror("all-template.htm");
		exit(1);
	}
	FILE *out = fopen("all-gigs.htm", "w");
	if (!out) {
		perror("all-gigs.htm");
		exit(1);
	}

	out_table(in, out, tm, n_gigs, 0);

	fclose(in);
	fclose(out);
}

int main(int argc, char *argv[])
{
	int c, upload = 0;

	while ((c = getopt(argc, argv, "u")) != EOF)
		if (c == 'u') ++upload;

	struct tm *tm;
	time_t now = time(NULL);
	tm = localtime(&now);

	FILE *fp = fopen("gigs", "r");
	if (!fp) {
		perror("gigs");
		exit(1);
	}

	char line[1024];
	while (fgets(line, sizeof(line), fp)) {
		char *date = strtok(line, " ");
		char *perf = strtok(NULL, "\n");
		if (date && perf)
			add_gig(date, perf);
#ifdef ADD_YEARS
		else if (date && strtol(date, NULL, 10) > 0)
			add_gig(date, NULL);
#endif
	}

	fclose(fp);

	out_gigs(tm);

	int drop = n_gigs - (NROWS * 2);
	printf("ngigs %d drop %d\n", n_gigs, drop);

	FILE *in = fopen("sched-template.htm", "r");
	if (!in) {
		perror("sched-template.htm");
		exit(1);
	}
	FILE *out = fopen("schedule.htm", "w");
	if (!out) {
		perror("shedule.htm");
		exit(1);
	}

	out_table(in, out, tm, NROWS, drop);

	fclose(in);
	fclose(out);

	if (upload) {
		system("aws s3 cp schedule.htm s3://rippers.ca");
		system("aws s3 cp all-gigs.htm s3://rippers.ca");
	}

	return 0;
}

/*
 * Local Variables:
 * compile-command: "gcc -O3 -Wall make-schedule.c -o make-schedule"
 * End:
 */

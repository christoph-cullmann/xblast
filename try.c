#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MSG 256

int
main ()
{
	FILE *fp;
	char s[MAX_MSG];
	char q[MAX_MSG];
	int l[16], i, j;
	char *c, *d;

	if (NULL != (fp = fopen ("test", "r"))) {
		c = s;
		i = 0;
		l[i] = 0;
		while (1) {
			d = fgets (c, MAX_MSG, fp);
			if (d == NULL)
				break;
			i++;
			l[i] = strlen (s) - 1;
			c = s + strlen (s) - 1;
		}
		l[i + 1] = strlen (s);
		printf ("%s\n", s);
		for (j = 0; j < i; j++) {
			strncpy (q, (s + l[j]), (l[j + 1] - l[j]));
			q[l[j + 1] - l[j]] = 0;
			printf ("--%s--\n", q);
		}
	}

	sprintf (s, "%s", "dude");
	printf ("--%s--\n", s);
	strcpy (q, s);
	printf ("--%s--\n", q);
	strcpy ((q + strlen (q)), "play");
	printf ("--%s--\n", q);

	return 0;
}

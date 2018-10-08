/*
 * includes
 */

#include "bad_words.h"
#include "xblast.h"
#include "chat.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>



/*
 * global data
 */

char **bad_words=NULL;
int bad_words_count=0;
static char buffer[256];



/*
 * local functions
 */

static void
strtolower(char *s)
{
    while (*s) {
        *s=tolower(*s);
        s++;
    }
}



/*
 * functions
 */

void
init_bad_words(void)
{
    // load from data file
    FILE *fp=fopen(GAME_DATADIR "/bad_words.txt", "r");
    if (!fp)
        return;

    // read line by line
    while (!feof(fp)) {
        *buffer='\0';
        fscanf(fp, "%s", buffer);
        if (*buffer) {
            bad_words_count++;

            if (!bad_words)
                bad_words=(char **)malloc(sizeof(char *));
            else
                bad_words=(char **)realloc(bad_words, bad_words_count*sizeof(char *));

            bad_words[bad_words_count-1]=strdup(buffer);
        }
    }
}



size_t
remove_bad_words(char *text)
{
    char *ptr;
    int idx;

/* Dbg_Chat("initial: text=%s.\n", text); */
/* Dbg_Chat("initial: strlen(text)=%d.\n", strlen(text)); */

    if (!bad_words)
        return strlen(text);

    for (idx=0; idx<bad_words_count; idx++) {
        strcpy(buffer, text);
        strtolower(buffer);

        while (NULL != (ptr=strstr(buffer, bad_words[idx]))) {

/*			Dbg_Chat("iteration\n"); */

			if ( !strncmp(ptr,"<3",2) ) {
				/* Special case for '<3' message parts */
/*				Dbg_Chat("<3 case\n"); */
				/* Store the suffix after the '<3' */
				char *suffix = strdup(ptr+2);
/*				Dbg_Chat("suffix=%s.\n", suffix); */
				/* Transform '<3' to '</3' */
				text[ptr-buffer]   = '<';
				text[ptr-buffer+1] = '/';
				text[ptr-buffer+2] = '3';
				ptr[0]             = '<';
				ptr[1]             = '/';
				ptr[2]             = '3';
				/* Append the suffix */
				strcpy(text+(ptr-buffer)+3,suffix);
				strcpy(buffer+(ptr-buffer)+3,suffix);
/*				Dbg_Chat("vorher: text=%s.\n", text); */
/*				Dbg_Chat("vorher: strlen(text)=%d.\n", strlen(text)); */
				/* Check whether the input string is now too long */
				if ( strlen(text) > CHAT_LINE_SIZE-1 ) {
					text[CHAT_LINE_SIZE-2] = '\0';
					buffer[CHAT_LINE_SIZE-2] = '\0';
				}
/*				Dbg_Chat("nachher: text=%s.\n", text); */
/*				Dbg_Chat("nachher: strlen(text)=%d.\n", strlen(text)); */
			} else {
				/* 'Normal' *-padding */
				memset(text+(ptr-buffer), '*', strlen(bad_words[idx]));
				memset(ptr, '*', strlen(bad_words[idx]));
			}

        }
    }

	return strlen(text);
}

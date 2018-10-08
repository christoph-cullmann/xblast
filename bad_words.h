#ifndef BAD_WORDS_H
#define BAD_WORDS_H

#include <ctype.h>
#include <stdlib.h>
#include <stddef.h>

extern char **bad_words;

void init_bad_words(void);
size_t remove_bad_words(char *text);

#endif /* #ifndef BAD_WORDS_H */

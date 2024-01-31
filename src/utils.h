// Various *unsafe* utility functions from K&R The C Programming Language + Stackoverflow

#pragma once

#include <string.h>

void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s) - 1; i < j; i++, j--)
    {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

/* itoa:  convert n to characters in s */
void itoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0) /* record sign */
        n = -n;         /* make n positive */
    i = 0;
    do
    {                          /* generate digits in reverse order */
        s[i++] = n % 10 + '0'; /* get next digit */
    } while ((n /= 10) > 0);   /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

char* new_n_str(const char* src, size_t src_len) {
    char* dest = (char*) calloc(src_len, sizeof(char));
    memcpy(dest, src, src_len);

    if (dest[src_len] != '\0') {
        fprintf(stderr, "FATAL: Copied non null-terminated string\n");
        return NULL;
    }

    return dest;
}

char* new_str(const char* src) {
    return new_n_str(src, strlen(src) + 1);
}

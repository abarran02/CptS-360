/*
 * async-signal-safe functions to write to screen
 * adapted from CSAPP by Bryant and O'Hallaron
*/

/* get string length */
size_t sio_strlen(char *s) {
    int i = 0;

    while (s[i] != '\0') {
        i++;
    }

    return i;
}

/* reverse string */
void sio_reverse(char *s)
{
    int c, i, j;

    for (i = 0, j = sio_strlen(s)-1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

/* convert long int to string */
void sio_ltoa(long v, char *s, int b) {
    int c, i = 0;
    int neg = v < 0;

    if (neg) {
        v = -v;
    }

    do {
        s[i++] = ((c = (v % b)) < 10)  ?  c + '0' : c - 10 + 'a';
    } while ((v /= b) > 0);

    if (neg) {
        s[i++] = '-';
    }

    s[i] = '\0';
    sio_reverse(s);
}

/* print string to screen */
ssize_t sio_puts(char *s) {
    return write(STDOUT_FILENO, s, sio_strlen(s));
}

/* print long int to screen */
ssize_t sio_putl(long v) {
    char s[16];

    sio_ltoa(v, s, 10);
    return sio_puts(s);
}

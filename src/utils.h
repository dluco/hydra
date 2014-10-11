#ifndef UTILS_H
#define UTILS_H

int mod(int x, int m);
void die(const char *errstr, ...);
void print_err(const char *errstr, ...);
char *str_copy(char **dest, const char *src);

#endif /* UTILS_H */

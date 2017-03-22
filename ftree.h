#ifndef _FTREE_H_
#define _FTREE_H_
#include <stdbool.h>

#define BUFFSIZE    512

/* Function for copying a file tree rooted at src to dest
 * Returns < 0 on error. The magnitude of the return value
 * is the number of processes involved in the copy and is
 * at least 1.
 */
int copy_ftree(const char *src, const char *dest);
int copy_file(const char *src, const char *dest);
bool is_directory(const char *path);
char * get_fullname(const char *dir, const char *name);
bool same_size(const char *src, const char *dest);
int update_permission(const char *src, const char *dest);

#endif // _FTREE_H_

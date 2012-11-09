#ifndef _DIRECTORY_H
#define _DIRECTORY_H

#include "unixfilesystem.h"

#include "direntv6.h"

#define MAX_FILE_NAME_LENGTH 14

int directory_findname(struct unixfilesystem *fs, const char *name,
                       int dirinumber, struct direntv6 *dirEnt, int *info1, int *info2);

#endif /* _DIECTORY_H */

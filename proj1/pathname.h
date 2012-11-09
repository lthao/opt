#ifndef _PATHNAME_H
#define _PATHNAME_H

#include "unixfilesystem.h"

int pathname_lookup(struct unixfilesystem *fs, const char *pathname, int *info1, int *info2);


#endif /* pathname.h */

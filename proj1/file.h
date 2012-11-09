#ifndef _FILE_H
#define _FILE_H

#include "unixfilesystem.h"


int file_getblock(struct unixfilesystem *fs, int inumber, int blockNo, void *buf); 
int directoryFile_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf, struct inode *in);


#endif /* _FILE_H */

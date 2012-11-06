#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include "unixfilesystem.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/*
 * Lookup the specified name (name) in the directory (dirinumber). If found, return the 
 * directory entry in dirEnt. Return 0 on success and something negative on failure. 
 */
int
directory_findname(struct unixfilesystem *fs, const char *name,
                   int dirinumber, struct direntv6 *dirEnt)
{
  	struct inode in;
	int getResult = inode_iget(fs, dirinumber, &in);
	if (getResult == -1) return -1;
	if ((in.i_mode & IFMT) != IFDIR) return -1;
	int size = inode_getsize(&in);
	int numbBlocks = ((size / BYTES_PER_BLOCK) + 1);
	for (int i = 0; i < numbBlocks; i++) {
		struct direntv6 buff[(BYTES_PER_BLOCK / sizeof(struct direntv6))];
		int validBytes = file_getblock(fs, dirinumber, i, &buff);
		if (validBytes == -1) return -1;
		for (int j = 0; j < (validBytes / (int)sizeof(struct direntv6)); j++) {
			struct direntv6 currentDirent = buff[j];
			if (strcmp(currentDirent.d_name, name) == 0) {
				*dirEnt = currentDirent;
				return 0;
			}
		}
	}
	return -1;
}

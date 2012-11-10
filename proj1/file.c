#include <stdio.h>
#include <assert.h>

#include "../cachemem.h"

#include "file.h"
#include "inode.h"
#include "diskimg.h"
#include "unixfilesystem.h"

/*
 * Fetch the specified file block from the specified inode.
 * Return the number of valid bytes in the block, -1 on error.
 */
int
file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf)
{
	//fprintf(stderr, "in file_getblock, size %d, inum: %d\n", cacheMemSizeInKB, inumber);
	struct inode in;
	int getResult = inode_iget(fs, inumber, &in);
	int sectorNum = inode_indexlookup(fs, &in, blockNum);
	int bytesRead = diskimg_readsector(fs->dfd, sectorNum, buf);
	if ((getResult == -1) || (bytesRead == -1) || (sectorNum == -1)) return -1;
	int size = inode_getsize(&in);
	if ((size % BYTES_PER_BLOCK) == 0) {
		return BYTES_PER_BLOCK;
	} else {
		int numblocks = (size / BYTES_PER_BLOCK);
		if (blockNum == numblocks) {
			return size % BYTES_PER_BLOCK;
		} else {
			return BYTES_PER_BLOCK;
		}
	}
}

int
directoryFile_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf, struct inode *in)
{
	//struct inode in;
	//int getResult = inode_iget(fs, inumber, &in);
	int sectorNum = inode_indexlookup(fs, in, blockNum);
	//fprintf(stderr, "in dirGetBlock, size %d, inum: %d\n", cacheMemSizeInKB, inumber);
	//fprintf(stderr, "in dirGetBlock, inum: %d blockNum: %d secNum: %d\n", inumber, blockNum, sectorNum);
	int bytesRead = diskimg_readsector(fs->dfd, sectorNum, buf);
	if ((bytesRead == -1) || (sectorNum == -1)) return -1;
	int size = inode_getsize(in);
	if ((size % BYTES_PER_BLOCK) == 0) {
		return BYTES_PER_BLOCK;
	} else {
		int numblocks = (size / BYTES_PER_BLOCK);
		if (blockNum == numblocks) {
			return size % BYTES_PER_BLOCK;
		} else {
			return BYTES_PER_BLOCK;
		}
	}
}


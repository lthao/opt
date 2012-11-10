/*
 * fileops.c  -  This module provides an Unix like file absraction
 * on the prog1 file system access code
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>

#include "fileops.h"
#include "proj1/pathname.h"
#include "proj1/unixfilesystem.h"
#include "diskimg.h"
#include "proj1/inode.h"
#include "proj1/file.h"
#include "proj1/chksumfile.h"

#define MAX_FILES 64

static uint64_t numopens = 0;
static uint64_t numreads = 0;
static uint64_t numgetchars = 0;
static uint64_t numisfiles = 0;

/*
 * Table of open files.
 */
static struct {
	char *pathname;    // absolute pathname NULL if slot is not used.
	int  cursor;       // Current position in the file
	//put the inumber in here
	//store the inode
	int inumber;
	int blockNo;
	struct inode in;
	int size;
	int flag;
	unsigned char buf[DISKIMG_SECTOR_SIZE];
} openFileTable[MAX_FILES];


static struct unixfilesystem *unixfs;

/*
 * Initialize the fileops module for the specified disk.
 */
void *
Fileops_init(char *diskpath)
{
	memset(openFileTable, 0, sizeof(openFileTable));
	
	int fd = diskimg_open(diskpath, 1);
	if (fd < 0) {
		fprintf(stderr, "Can't open diskimagePath %s\n", diskpath);
		return NULL;
	}
	unixfs = unixfilesystem_init(fd);
	if (unixfs == NULL) {
		diskimg_close(fd);
		return NULL;
	}
	return unixfs;
}

/*
 * Open the specified absolute pathname for reading. Returns -1 on error;
 */
int
Fileops_open(char *pathname)
{
	int fd;
	int inumber;
	
	numopens++;
	
	for (fd = 0; fd < MAX_FILES; fd++) {
		if (openFileTable[fd].pathname == NULL) break;
	}
	if (fd >= MAX_FILES) {
		return -1;  // No open file slots
	}
	
	//fprintf(stderr, "fd: %d\n", fd);
	
	inumber = pathname_lookup(unixfs, pathname, &(openFileTable[fd].size), &(openFileTable[fd].flag));
	if (inumber < 0) {
		return -1; // File not found
	}
	fprintf(stderr, "file_open path: %s\n", pathname);
	fprintf(stderr, "file_open in: %d\n", inumber);
	
	//fprintf(stderr, "size: %d\n", openFileTable[fd].size);
	
	openFileTable[fd].pathname = strdup(pathname); // Save our own copy
	openFileTable[fd].cursor = 0;
	openFileTable[fd].inumber = inumber;
	openFileTable[fd].blockNo = -1;
	
	int err = inode_iget(unixfs, inumber, &(openFileTable[fd].in));
	if (err < 0) {
		return err;
	}
	if (!(openFileTable[fd].in.i_mode & IALLOC)) {
		return -1;
	}
	
	return fd;
}

int
Fileops_open2(char *pathname, int argInumber)
{
	int fd;
	//int inumber;
	
	numopens++;
	
	for (fd = 0; fd < MAX_FILES; fd++) {
		if (openFileTable[fd].pathname == NULL) break;
	}
	if (fd >= MAX_FILES) {
		return -1;  // No open file slots
	}
	
	//fprintf(stderr, "fd: %d\n", fd);
	
	//inumber = pathname_lookup(unixfs, pathname, &(openFileTable[fd].size), &(openFileTable[fd].flag));
	if (argInumber < 0) {
		return -1; // File not found
	}
	
	//fprintf(stderr, "size: %d\n", openFileTable[fd].size);
	
	openFileTable[fd].pathname = strdup(pathname); // Save our own copy
	openFileTable[fd].cursor = 0;
	openFileTable[fd].inumber = argInumber;
	openFileTable[fd].blockNo = -1;
	
	int err = inode_iget(unixfs, argInumber, &(openFileTable[fd].in));
	if (err < 0) {
		return err;
	}
	if (!(openFileTable[fd].in.i_mode & IALLOC)) {
		return -1;
	}
	
	return fd;
}


/*
 * Fetch the next character from the file. Return -1 if at end of file.
 */
int
Fileops_getchar(int fd)
{
	int inumber;
	int bytesMoved;
	int size;
	int blockOffset;
	
	numgetchars++;
	
	if (openFileTable[fd].pathname == NULL)
		return -1;  // fd not opened.
	
	inumber = openFileTable[fd].inumber;
	size = inode_getsize(&(openFileTable[fd].in));
	
	//use size to figure out the number of blocks
	
	if (openFileTable[fd].cursor >= size) return -1; // Finished with file
	
	int newblockNo = openFileTable[fd].cursor / DISKIMG_SECTOR_SIZE;
	blockOffset =  openFileTable[fd].cursor % DISKIMG_SECTOR_SIZE;
	if (openFileTable[fd].blockNo != newblockNo) {
		openFileTable[fd].blockNo = newblockNo;
		bytesMoved = directoryFile_getblock(unixfs, inumber, openFileTable[fd].blockNo, openFileTable[fd].buf, &(openFileTable[fd].in));
		if (bytesMoved < 0) {
			return -1;
		}
		assert(bytesMoved > blockOffset);
	}
	
	openFileTable[fd].cursor += 1;
	
	return (int)(openFileTable[fd].buf[blockOffset]);
}

/*
 * Implement the Unix read system call. Number of bytes returned.  Return -1 on
 * err.
 */
int
Fileops_read(int fd, char *buffer, int length)
{
	int i;
	int ch;
	
	numreads++;
	
	for (i = 0; i < length; i++) {
		ch = Fileops_getchar(fd);
		if (ch == -1) break;
		buffer[i] = ch;
	}
	return i;
}

/*
 * Return the current position in the file.
 */
int
Fileops_tell(int fd)
{
	if (openFileTable[fd].pathname == NULL)
		return -1;  // fd not opened.
	
	return openFileTable[fd].cursor;
}


/*
 * Close the files - return the resources
 */

int
Fileops_close(int fd)
{
	if (openFileTable[fd].pathname == NULL)
		return -1;  // fd not opened.
	
	free(openFileTable[fd].pathname);
	openFileTable[fd].pathname = NULL;
	return 0;
}

/*
 * Return true if specified pathname is a regular file.
 */
int
Fileops_isfile(char *pathname)
{
	numisfiles++;
	
	int inumber = pathname_lookup(unixfs, pathname, NULL, NULL);
	if (inumber < 0) {
		return 0;
	}
	//fprintf(stderr, "file_isfile path: %s\n", pathname);
	//fprintf(stderr, "file_isfile in: %d\n", inumber);
	struct inode in;
	int err = inode_iget(unixfs, inumber, &in);
	if (err < 0) return 0;
	
	if (!(in.i_mode & IALLOC) || ((in.i_mode & IFMT) != 0)) {
		/* Not allocated or not a file */
		return 0;
	}
	return 1; /* Must be a file */
}

int
Fileops_isfile2(char *pathname, int *inumber)
{
	numisfiles++;
	
	int resultInumber = pathname_lookup(unixfs, pathname, NULL, NULL);
	if (resultInumber < 0) {
		return 0;
	}
	
	*inumber = resultInumber;
	
	struct inode in;
	int err = inode_iget(unixfs, resultInumber, &in);
	if (err < 0) return 0;
	
	if (!(in.i_mode & IALLOC) || ((in.i_mode & IFMT) != 0)) {
		/* Not allocated or not a file */
		return 0;
	}
	return 1; /* Must be a file */
}

void
Fileops_dumpstats(FILE *file)
{
	fprintf(file,
			"Fileops: %"PRIu64" opens, %"PRIu64" reads, "
			"%"PRIu64" getchars, %"PRIu64 " isfiles\n",
			numopens, numreads, numgetchars, numisfiles);
}


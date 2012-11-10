/*
 * cachemem.c  -  This module allocates the memory for caches. 
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h> // for PRIu64

#include <sys/mman.h>

#include "cachemem.h"

int cacheMemSizeInKB;
void *cacheMemPtr;

//indoe_iget
int igetOccupied = 0;
int currentInum = -1;

//index
int indexFlag = 0;
int indexSectorNum = -1;
int indexSectorNum2 = -1;

typedef struct block {
	int secNum;
	int count;
} CacheBlock;

CacheBlock *arrayBlock;
int numEntries;
int fullFlag;
int numAccess;
int maxNumEntries;

/*
 * Allocate memory of the specified size for the data cache optimizations
 * Return -1 on error, 0 on success. 
 */
int
CacheMem_Init(int sizeInKB)
{
  /* Size needs to be not negative or too big and 
   * multiple of the 4KB page size 
   */
    if ((sizeInKB < 0) || (sizeInKB > (CACHEMEM_MAX_SIZE/1024))
  	  || (sizeInKB % 4)) {
	  fprintf(stderr, "Bad cache size %d\n", sizeInKB);
	  return -1;
    }
    void *memPtr = mmap(NULL, sizeInKB*1024, PROT_READ|PROT_WRITE, 
			  MAP_PRIVATE|MAP_ANON, -1, 0);
    if (memPtr == MAP_FAILED) {
  	  perror("mmap");
	  return -1;
    }
	
	maxNumEntries = (sizeInKB * 1024) / 512;
	
	arrayBlock = calloc(maxNumEntries, sizeof(CacheBlock));
	
	for (int i = 0; i < maxNumEntries; i++) {
		arrayBlock[i].secNum = -1;
		arrayBlock[i].count = -1;
	}
	
    cacheMemSizeInKB = sizeInKB;
    cacheMemPtr = memPtr;
	numAccess = 0;
	numEntries = 0;
	fullFlag = 0;
    return 0;
}

int secNumInCache(int num) {
	for (int i = 0; i < numEntries; i++) {
		if (arrayBlock[i].secNum == num) return i;
	}
	return -1;
}

int leastUsedIndex() {
	int min = arrayBlock[0].count;
	int index = 0;
	for (int i = 1; i < numEntries; i++) {
		if (arrayBlock[i].count < min) {
			min = arrayBlock[i].count;
			index = i;
		}
	}
	return index;
}

void writeToCache(void *source, int num) {
	numAccess++;
	if (!fullFlag) {
		//this is 0 based, but the numEntries is not so we increment it after
		memcpy(((char *)cacheMemPtr + (512 * numEntries)), source, 512);
		arrayBlock[numEntries].count = numAccess;
		arrayBlock[numEntries].secNum = num;
		numEntries++;
		if (numEntries == maxNumEntries) {
			fullFlag = 1;
		}
	} else {
		int indexToRemove = leastUsedIndex();//this is 0 based
		memcpy(((char *)cacheMemPtr + (512 * indexToRemove)), source, 512);
		arrayBlock[indexToRemove].count = numAccess;
		arrayBlock[indexToRemove].secNum = num;
	}
}

void getFromCache(void *destination, int index) {
	numAccess++;
	arrayBlock[index].count = numAccess;
	memcpy(destination, ((char *)cacheMemPtr + (512 * index)), 512);
}

//for indexlookup

int indexFlagIsSet() {
	return indexFlag;
}

int getCurrentIndexSectorNum() {
	return indexSectorNum;
}

void setCurrentIndexSectorNum(int secNum) {
	indexSectorNum = secNum;
}

void setIndexFlag(int flag) {
	indexFlag = flag;
}

int getCurrentIndexSectorNum2() {
	return indexSectorNum2;
}

void setCurrentIndexSectorNum2(int secNum) {
	indexSectorNum2 = secNum;
}

//for inode_iget

int iGetFlagIsSet() {
	return igetOccupied;
}

int getCurrentInum() {
	return currentInum;
}

void setCurrentInum(int inum) {
	currentInum = inum;
}

void setIGetFlag(int flag) {
	igetOccupied = flag;	
}












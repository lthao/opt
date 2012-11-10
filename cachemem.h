#ifndef _CACHEMEM_H
#define _CACHEMEM_H


/*
 * The main export of the cachemem module is the memory for the cache
 * pointed to by the following global variables:
 * cacheMemSizeInKB - The size of the cache memory in kiloytes. 
 * cacheMemPtr      - Starting address of the cache memory. 
 */ 
extern int cacheMemSizeInKB;
extern void *cacheMemPtr;

#define CACHEMEM_MAX_SIZE (64*1024*1024)

int CacheMem_Init(int sizeInKB);
int secNumInCache(int num);
void writeToCache(void *source, int num);
void getFromCache(void *destination, int index);

int iGetFlagIsSet();
int getCurrentInum();
void setCurrentInum(int inum);
void setIGetFlag(int flag);

int indexFlagIsSet();
int getCurrentIndexSectorNum();
void setCurrentIndexSectorNum(int secNum);
void setIndexFlag(int flag);

int getCurrentIndexSectorNum2();
void setCurrentIndexSectorNum2(int secNum);

#endif /* _CACHEMEM_H */

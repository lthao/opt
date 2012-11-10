#include <stdio.h>
#include <assert.h>
#include "ino.h"
#include "inode.h"
#include "diskimg.h"
#include "unixfilesystem.h"

#define INODES_PER_SEC 16
#define I_ADDR_LAST_INDEX 7

/*
 * Fetch the specified inode from the filesystem. 
 * Return 0 on success, -1 on error.  
 */
int
inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp)
{	
	int sectorNum = ((inumber - 1) / INODES_PER_SEC) + 2; //+2 accounts for boot and super block
	int sectorIndex = (inumber - 1) % INODES_PER_SEC;
	struct inode buff[INODES_PER_SEC];
	int bytesRead = diskimg_readsector(fs->dfd, sectorNum, &buff);
	if (bytesRead != DISKIMG_SECTOR_SIZE) return -1;
	*inp = buff[sectorIndex];
	return 0;
}

/*
 * Get the location of the specified file block of the specified inode.
 * Return the disk block number on success, -1 on error.  
 */
int
inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum)
{
	if (((inp->i_mode) & ILARG) == 0) {
		if ((blockNum < 0) || (blockNum > I_ADDR_LAST_INDEX)) return -1;
		return inp->i_addr[blockNum];
	} else {
		int i_addrIndex = blockNum / UINT16_ENTRIES_PER_BLOCK;
		int sectorIndex = blockNum % UINT16_ENTRIES_PER_BLOCK;		
		if (i_addrIndex <= (I_ADDR_LAST_INDEX - 1)) {
			int sectorNum = inp->i_addr[i_addrIndex];
			uint16_t buff[UINT16_ENTRIES_PER_BLOCK];
			
			int cacheArrayIndex = secNumInCache(sectorNum);
			if (cacheArrayIndex != -1) {
				//secnum is in cache
				getFromCache(&buff, cacheArrayIndex);
				return (int) buff[sectorIndex];
			} else {
				//secnum is not in cache
				int bytesRead = diskimg_readsector(fs->dfd, sectorNum, &buff);
				if (bytesRead == -1) return -1;
				writeToCache(&buff, sectorNum);
				return (int) buff[sectorIndex];
			}
		} else {
			int block1SectorNum = inp->i_addr[I_ADDR_LAST_INDEX];
			uint16_t block1Buff[UINT16_ENTRIES_PER_BLOCK];
			int block1Index = ((blockNum - (I_ADDR_LAST_INDEX * UINT16_ENTRIES_PER_BLOCK)) / UINT16_ENTRIES_PER_BLOCK);
			uint16_t block2Buff[UINT16_ENTRIES_PER_BLOCK];
			
			int cacheArrayIndex1 = secNumInCache(block1SectorNum);
			if (cacheArrayIndex1 != -1) {
				//block1SectorNum is in cache
				getFromCache(&block1Buff, cacheArrayIndex1);
				int block2SectorNum = block1Buff[block1Index];
				int cacheArrayIndex2 = secNumInCache(block2SectorNum);
				if (cacheArrayIndex2 != -1) {
					//block2SectorNum is in cache
					getFromCache(&block1Buff, cacheArrayIndex2);
					int index = blockNum % UINT16_ENTRIES_PER_BLOCK;
					return (int) block2Buff[index];
				} else {
					//block2SectorNum is not in cache
					int block2BytesRead = diskimg_readsector(fs->dfd, block2SectorNum, &block2Buff);
					if (block2BytesRead == -1) return -1;
					writeToCache(&block2Buff, block2SectorNum);
					int index = blockNum % UINT16_ENTRIES_PER_BLOCK;
					return (int) block2Buff[index];
				}
			} else {
				//block1SectorNum is not in cache
				int bytesRead = diskimg_readsector(fs->dfd, block1SectorNum, &block1Buff);
				if (bytesRead == -1) return -1;
				writeToCache(&block1Buff, block1SectorNum);
				int block2SectorNum = block1Buff[block1Index];
				int block2BytesRead = diskimg_readsector(fs->dfd, block2SectorNum, &block2Buff);
				if (block2BytesRead == -1) return -1;
				writeToCache(&block2Buff, block2SectorNum);
				int index = blockNum % UINT16_ENTRIES_PER_BLOCK;
				return (int) block2Buff[index];
			}
		}
	}
	return -1;
}

/* 
 * Compute the size of an inode from its size0 and size1 fields.
 */
int
inode_getsize(struct inode *inp) 
{
	return ((inp->i_size0 << 16) | inp->i_size1); 
}

#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "unixfilesystem.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

/*
 * Return the inumber associated with the specified pathname. This need only
 * handle absolute paths. Return a negative number if an error is encountered.
 */

int
pathname_lookup(struct unixfilesystem *fs, const char *pathname, int *info1, int *info2)
{
	char *path = strdup(pathname);
	int inumber = ROOT_INUMBER;
	char *index;
	path += 1;
	while(strchr(path, '/') != NULL) {
		char *nextSlash = strchr(path, '/');
		char name[MAX_FILE_NAME_LENGTH];
		if (nextSlash != NULL) {
			int numChars = nextSlash - path;
			strncpy(name, path, numChars);
			name[numChars] = '\0';
		} else {
			int length = strlen(path);
			strncpy(name, path, length);
			name[length] = '\0';
		}
		struct direntv6 dirEnt;
		int result = directory_findname(fs, name, inumber, &dirEnt, info1, info2);
		if (result < 0) return -1;
		inumber = dirEnt.d_inumber;
		index = strchr(path, '/');
		path = index + 1; //the plus one reflects the existence of a slash
	}
	if (strcmp(path, "") == 0) return inumber; //accounts for pathnames that end in a slash
	struct direntv6 dirEnt;
	int result = directory_findname(fs, path, inumber, &dirEnt, info1, info2);
	if (result != 0) return -1;
	inumber = (int)dirEnt.d_inumber;
	if (strchr(path, '/') == NULL) {
		return inumber;
	}
	free(path);
	return -1;
}






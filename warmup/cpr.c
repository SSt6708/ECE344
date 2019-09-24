#include "common.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <errno.h>

/* make sure to use syserror() when a system call fails. see common.h */

void
usage()
{
	fprintf(stderr, "Usage: cpr srcdir dstdir\n");
	exit(1);
}

void singleFileCopy(char*source, char* dest);
char* makePathName(char* source, char*fileName);
bool ifRegularFile(char* path);
void copyDirectory(char* source, char* dest);

bool validDirName(char *name);

int
main(int argc, char *argv[])
{

	if (argc != 3) {
		usage();
		
	}

	/*source = "./source";
	dest = "./distination";*/
	char *source, *dest;
	source = argv[1];
	dest = argv[2];

	copyDirectory(source, dest);
		
	
	//TBD();
	
	return 0;
}

void copyDirectory(char* source, char* dest){
	

	DIR *dirEntry;
	struct dirent *pdrent;

	 // permission of the file

	//trying new things
	
	


	int success;
	success = mkdir(dest, S_IRWXU); // make a destination folder

	if(success < 0){
		syserror(mkdir, dest);
	}

	
	
	dirEntry = opendir(source);
	
	printf("%s\n", source);
	

	if(dirEntry == NULL){
		syserror(opendir, source);
	}

	if(readdir(dirEntry) == NULL){
		syserror(readdir, source);
	}


	struct stat filestat;
	stat(source, &filestat); // try to extract the permission

	if(stat(source, &filestat) < 0){
		syserror(stat, source);
	}
	mode_t permission = filestat.st_mode;

	while((pdrent = readdir(dirEntry)) != NULL){
		
		char* fileName = pdrent->d_name;
		
		if(!validDirName(fileName)){
			
		}else{

			
			char*sourceName = makePathName(source, fileName);
			printf("Source name is: %s\n", sourceName);
			char *destName = makePathName(dest, fileName);
			printf("Destinasion name is: %s\n", destName);
		
			if(ifRegularFile(sourceName)){
				singleFileCopy(sourceName, destName);
			}else{
				copyDirectory(sourceName, destName);
			}
		
		}

	
		
	}	

	int closeDir;
	closeDir = closedir(dirEntry);

	if(closeDir < 0){
		syserror(closeDir, source);
	}

	int setPerm;
	setPerm = chmod(dest, permission);

	if(setPerm < 0){
		syserror(chmod, dest);
	}
	
}


bool validDirName(char *name){
	
	if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0){
		return false;
	}else{
		return true;
	}
	


}

bool ifRegularFile(char* path){
	struct stat filestat;

	if(stat(path, &filestat) < 0){
		syserror(stat, path);
	}

	stat(path, &filestat);


	
		if(S_ISREG(filestat.st_mode)){
			return true;
		}else{
			return false;
		}
	
		
	

}



char* makePathName(char* source, char*fileName){
	char *sourceName = (char*)malloc((strlen(fileName) +2+ strlen(source))* sizeof(char));
		int i =0;
		int j = 0;
		while (source[i] != '\0'){
			sourceName[i] = source[i];
			i++;
		}

		sourceName[i] = '/';
		i++;

		while(fileName[j] != '\0'){
			sourceName[i+j] = fileName[j];
			j++;
		}
		sourceName[i+j] = '\0';
		return sourceName;
}


void singleFileCopy(char* sourceDir, char* destDir){
	int fd, flags, ret, cret, wrt, setPerm;
	char *buf;

	struct stat filestat;

	if(stat(sourceDir, &filestat) < 0){
		syserror(stat, sourceDir);
	}
	stat(sourceDir, &filestat); // try to extract the permission
	mode_t permission = filestat.st_mode; // permission of the file

	long fileSize = filestat.st_size;
	buf = (char*)malloc(fileSize*sizeof(char)); // buffer that reads in a file, first 4096 bits
	
//try to copy single file 
	flags = O_RDONLY;
	fd = open(sourceDir, flags);
	
	if(fd < 0){
		syserror(open,sourceDir);
		
	}
//
	cret = creat(destDir, S_IRWXU);
	if(cret < 0){
		syserror(creat, destDir);
	}
	
	
	
		ret= read(fd, buf, fileSize); 

	if(ret < 0){
		syserror(read, sourceDir);
	}

		wrt = write(cret, buf, ret);

	if(wrt < 0){
		syserror(write, destDir);
	}
		
	
	

	setPerm = chmod(destDir, permission);

	if(setPerm < 0){
		syserror(chmod, destDir);
	}
	

	


	int closeSrc = close(fd);
	int closeDst = close(cret);

	if(closeSrc < 0){
		syserror(close, sourceDir);
	}

	if(closeDst < 0){
		syserror(close, destDir);
	}

	
	
	

}

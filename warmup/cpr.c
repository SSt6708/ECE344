#include "common.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

/* make sure to use syserror() when a system call fails. see common.h */

void
usage()
{
	fprintf(stderr, "Usage: cpr srcdir dstdir\n");
	exit(1);
}

void singleFileCopy(char*source, char* dest);
char* makePathName(char* source, char*fileName);

void copyDirectory(char* source, char* dest);

int
main(int argc, char *argv[])
{
	char *source, *dest;
	source = "./source";
	dest = "./destination";

	

	
	
		copyDirectory(source, dest);
		
		
			
			
		
	
	

	
	
	/*if (argc != 3) {
		usage();\
		
	}*/
	//TBD();
	
	return 0;
}

void copyDirectory(char* source, char* dest){
	

	DIR *dirEntry;
	struct dirent *pdrent;
	struct stat filestat;
	
	int success;
	success = mkdir(dest, S_IRWXU); // make a destination folder
	
	if(success == -1){
		fprintf(stderr, "Usage: cpr srcdir dstdir\n");
	}

	dirEntry = opendir(source);

	//singleFileCopy("./source/test", "./destination/test ");
	

	while((pdrent = readdir(dirEntry)) != NULL){
		
		char* fileName = pdrent->d_name;
		
		if(fileName[0] == '.'){
			continue;
		}

		char*sourceName = makePathName(source, fileName);
		printf("Source name is: %s\n", sourceName);
		



		char *destName = makePathName(dest, fileName);
		printf("Destinasion name is: %s\n", destName);
		
		if(stat(sourceName, &filestat) < 0){
			printf("read stat failed\n");
			return;
			
		}

		//1 is regular file, 2 is directory, 3 is other 
		int fileMode;

		if(S_ISREG(filestat.st_mode)){
			fileMode = 1;
		}else if(S_ISDIR(filestat.st_mode)){
			fileMode = 2;
		}else{
			fileMode = 3;
		}

		if(fileMode == 1){
			singleFileCopy(sourceName, destName);
		}else if(fileMode == 2){

			
			copyDirectory(sourceName, destName); // recursively
			
		}
		
		
		

		//printf("Source name: %s\nDestination name: %s\n", sourceName, destName);

		
	}	
	
}






char* makePathName(char* source, char*fileName){
	char *sourceName = (char*)malloc((strlen(fileName) + strlen(source))* sizeof(char));
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
		return sourceName;
}


void singleFileCopy(char* sourceDir, char* destDir){
	int fd, flags, ret, cret;
	char *buf;

	buf = (char*)malloc(4096*sizeof(char)); // buffer that reads in a file
	
//try to copy single file 
	flags = O_RDWR;
	fd = open(sourceDir, flags);
	
	if(fd < 0){
		fprintf(stderr, "Failed to open file\n");
		exit(1);
	}
//
	cret = creat(destDir, S_IRWXU);
	if(cret < 0){
		fprintf(stderr, "Failed to create file\n");
		exit(1);
	}
	
	

	ret= read(fd, buf, 4096); 
	write(cret, buf, ret);

	int closeSrc = close(fd);
	int closeDst = close(cret);

	if(closeSrc == -1 || closeDst == -1){
		fprintf(stderr, "Failed to close file\n");
		exit(1);
	}

	
	
	

}

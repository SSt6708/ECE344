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

int
main(int argc, char *argv[])
{

/*	int fd, flags, new, ret, cret;
	char *buf;

	buf = (char*)malloc(4096*sizeof(char)); 
	
//try to copy single file 
	flags = O_RDWR;
	fd = open("./source/test", flags);
	cret = creat("./source/newFile", S_IRWXU);
	
	flags = O_RDWR;
	fd = open("./source/test", flags);
	new = open("./source/test1", flags);

	ret= read(fd, buf, 4096); 

	printf("The test document says: %s", buf);

	write(new, buf, ret);
	write(cret, buf, ret);
*/

	int success ;
	success = mkdir("./destination", S_IRWXU);
	
	if(success == -1){
		fprintf(stderr, "Usage: cpr srcdir dstdir\n");
	}
	DIR *dirEntry;
	

	dirEntry = opendir("./source");

	char* buf = (char*) malloc(256*sizeof(char));
	buf = readdir(dirEntry)->d_name;

	printf("The first file name is: %s\n", buf);
	
	/*if (argc != 3) {
		usage();
	}*/
	//TBD();
	return 0;
}

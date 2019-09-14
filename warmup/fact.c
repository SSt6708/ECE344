#include "common.h"
#include <stdbool.h>

bool digit(char* argv);

int fact(int num);



int
main(int argc, char* argv[])
{

	if(argc != 2){
		printf("Huh?\n");
	}else{

		if(!digit(argv[1])){
			printf("Huh?\n");
		}else{
			int input = atoi(argv[1]);
			if(input > 12){
				printf("Overflow\n");
			}else if( input < 1){
				printf("Huh?\n");
			}else{
				int answer = fact(input);
				printf("%d\n", answer);	

			}
		}


	}


	
	return 0;
}


bool digit(char* argv){
	int i = 0;

	while(argv[i] != '\0'){

		if(argv[i] < 48 || argv[i] > 57){
			return false;
		}
		i++;
	}
	return true;
}

int fact(int num){

	if (num == 1 || num == 0){
		return 1;
	}else{
		return num* fact(num - 1);
	}


}


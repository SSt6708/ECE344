#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "wc.h"
#include <string.h>
#include <ctype.h>



typedef struct node{
	int count; // number of words occur
	char* word;
	struct node *next; //linkedlist in hashtable 
}entry;



struct wc {
	entry ** table;
	unsigned long tableSize;

};

//helper functions
void addWord(struct wc *wc, char *word, int index);
void fillTable(struct wc* wc, char* userInput);
int hashKey(char *word, unsigned long tableSize); // generates hashFunctions
void printEntry( entry* t);
void deleteEntry( entry* t);

struct wc *
wc_init(char *word_array, long size)
{
	struct wc *wc;

	wc = (struct wc *)malloc(sizeof(struct wc));
	assert(wc); 

	//init size of table 
	wc->table = (entry**) malloc(size * sizeof(entry *));
	assert(wc->table);
	wc->tableSize = size;

	fillTable(wc, word_array);

	return wc;
}

int hashKey(char*words, unsigned long tableSize){

	int hash = 5381;
	int c = 0;

	while((c = *words++) != 0){
		hash = ((hash << 5) + hash) + c;
	}

	if (hash < 0){
		hash *= -1;
	}

	hash = hash % tableSize; //make sure its not out of index

	return hash;

}

void addWord(struct wc *wc, char *word, int index){

	if (wc->table[index] == NULL){ //if nothings there, place it
		
		wc->table[index] = (entry*)malloc(sizeof(entry));
		wc->table[index]->count = 1;
		wc->table[index]->word = word;
		wc->table[index]->next = NULL;
		return;
	}

	//pointer to the first of the list
	entry* curr = wc->table[index];
	entry* prev = NULL;
	

	while(curr != NULL){

		if(strcmp(word, curr->word) == 0){
			curr->count = curr->count + 1;
			free(word);
			return;
		}else{
			
			prev = curr;
			curr = curr->next;
		}

	}

	// havent found the word so add it to the list
	prev->next = (entry *)malloc(sizeof(entry)); // initialize a new entry at the end of the list
	prev->next->count = 1;
	prev->next->word = word;
	prev->next->next = NULL;
	return;
}


void fillTable(struct wc* wc, char* userInput){

	char *lastHold = NULL; //last word being parsed

	int i = 0;
	
	int wordSize = 0;
	int lastPosition = 0;

	
	
	
	while(userInput[i] != '\0'){ //while not the end of line
			if(!isspace(userInput[i])){
				wordSize ++;
				
			}else{
				if(wordSize != 0){ //there is a word before space
					lastHold = (char*)malloc((wordSize + 1) * sizeof(char)); // allocate the size of a new word

					int j;

					for(j = 0; j < wordSize; j++){
						lastHold[j] = userInput[lastPosition + j];
					}
						lastHold[j] = '\0';
						int h = hashKey(lastHold, wc->tableSize);
						addWord(wc, lastHold, h);
						wordSize = 0;
						
				}
				
				lastPosition = i +1;
			}


		i++;

	}
	
	
	
	



}

void printEntry( entry* t){


	if(t->next != NULL){
		printEntry(t->next);
	}
	printf("%s:%d\n", t->word, t->count);
	return;
}
void deleteEntry(entry* t){

	if(t->next != NULL){
		deleteEntry(t->next);
	}
	
	free(t->word);
	free(t);
	return;
}
void
wc_output(struct wc *wc)
{
	int i;
	for(i = 0; i < wc->tableSize; i++){
		if(wc->table[i] != NULL){
			printEntry(wc->table[i]);
		}
	}
}

void
wc_destroy(struct wc *wc)
{
	int i;
	for(i = 0; i < wc->tableSize ; i ++){
		if(wc->table[i] != NULL){
			deleteEntry(wc->table[i]);


		}

	}
	free(wc->table);
	free(wc);
}

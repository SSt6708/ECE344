#include "request.h"
#include "server_thread.h"
#include "common.h"
#include <pthread.h>




typedef struct node{

	int count;
	struct file_data *data;
	struct node* next; //a linked list;
}entry;


typedef struct h_table
{
	int size;
	int cache_size;
	int available_cache;
	pthread_mutex_t *table_lock;
	entry* LRU_Cache;
	entry** table; //array of entries
}hashTable;



unsigned long hashKey(char*words){ //hash key

	int hash = 5381;
	int c = 0;

	while((c = *words++) != 0){
		hash = ((hash << 5) + hash) + c;
	}

	if (hash < 0){
		hash *= -1;
	}

	hash = hash % 10000000; //make sure its not out of index

	return hash;

}





hashTable *ht;





struct server {
	int nr_threads;
	int max_requests;
	int max_cache_size;
	int exiting;
    pthread_cond_t *cv_full;
	pthread_cond_t *cv_empty;
	pthread_t *worker_threads;
	pthread_mutex_t *lock;
	int *buf;
	int in;
	int out;

	/* add any other parameters you need */
};

/* static functions */

/* initialize file data */
static struct file_data *
file_data_init(void)
{
	struct file_data *data;

	data = Malloc(sizeof(struct file_data));
	data->file_name = NULL;
	data->file_buf = NULL;
	data->file_size = 0;
	return data;
}

/* free all file data */
static void
file_data_free(struct file_data *data)
{
	free(data->file_name);
	free(data->file_buf);
	free(data);
}


void update_Cache(struct file_data * data_in_file){

	//if nothing is in cache
	if(ht->LRU_Cache == NULL){

		entry* new = malloc(sizeof(entry));
		new->data = data_in_file;
		new->count = 0;
		new->next = NULL;
		ht->LRU_Cache = new;
	}else if(ht->LRU_Cache->next == NULL){

		if(ht->LRU_Cache->data != data_in_file){
			entry* new = malloc(sizeof(entry));
			new->data = data_in_file;
			new->count = 0;
			new->next = NULL;
			ht->LRU_Cache->next = new;
		}
		

	}else{ //more than 1
		entry* curr = ht->LRU_Cache;
		entry* prev = ht->LRU_Cache;

		while (curr)
		{
			if(curr->data == data_in_file) break; //find the same file
			prev = curr;
			curr = curr->next;
		}
		
		if(curr){  //found
		
			if(curr->data == ht->LRU_Cache->data){ //if it is the first one
				ht->LRU_Cache = ht->LRU_Cache->next;
				curr->next = NULL;
				entry* last = ht->LRU_Cache;
				while (last)
				{
					prev = last;
					last = last->next;
				}
				prev->next = curr;
			
			
			
			}else{
				prev->next = curr->next;
				curr->next = NULL;
				while (prev->next)
				{
					prev = prev->next;
				}
				prev->next = curr;

			}
			//insert it to the back, most recent use is at the back of the linked list
			
		}else{
			entry* new = malloc(sizeof(entry));
			new->data = data_in_file;
			new->count = 0;
			new->next = NULL;
			prev->next = new;
		}
	
	}

}

struct file_data* cache_lookup(struct file_data* data_in_file, int flag){

	unsigned long key = hashKey(data_in_file->file_name);

	if(ht->table[key] == NULL){ //not find
		return NULL;
	}

	entry* temp = ht->table[key];

	while(temp){

		if(strcmp(temp->data->file_name, data_in_file->file_name) == 0){
			
			if(!flag){
				update_Cache(temp->data);
			}
			

			return temp->data;
		}

		temp = temp->next;


	}

	return NULL; //cant find


}


void cache_insert_table(struct file_data* data_in_file){

	unsigned long key = hashKey(data_in_file->file_name);
	entry* new = malloc(sizeof(entry));
	new->count = 0;
	new->data = data_in_file;
	new->next = NULL;

	if(ht->table[key] == NULL){
		ht->table[key] = new;
		ht->available_cache = ht->available_cache - data_in_file->file_size;
	}else{
		entry * temp = ht->table[key];

		while(temp->next){
			temp = temp->next;
		}

		temp->next = new;
		ht->available_cache = ht->available_cache - data_in_file->file_size;
	}

	update_Cache(data_in_file); 



}

void delete_entry(struct file_data* to_delete){
	unsigned long key = hashKey(to_delete->file_name);

	entry* curr = ht->table[key];
	entry* prev = ht->table[key];

	if(curr->next == NULL){
		free(curr);
		ht->table[key] = NULL;
		return;
	}
	
	
	while (curr->data != to_delete)
	{
		prev = curr;
		curr = curr->next;
	}
	
	prev->next = curr->next;
	
	free(curr);
}

void cache_evict(int amount_to_evict){

	while(ht->available_cache < amount_to_evict){
		
		entry* to_be_evicted = ht->LRU_Cache;
		ht->LRU_Cache = ht->LRU_Cache->next;
		ht->available_cache += to_be_evicted->data->file_size;
		delete_entry(to_be_evicted->data);
		free(to_be_evicted);
	}
}


void cache_insert(struct file_data* data_to_insert){

	if(data_to_insert->file_size > 0.5*ht->cache_size){
		return;
	}else if(data_to_insert->file_size > ht->available_cache){
		cache_evict(data_to_insert->file_size);
		cache_insert_table(data_to_insert);

	}else{
		cache_insert_table(data_to_insert);
	}

}






static void
do_server_request(struct server *sv, int connfd)
{
	//int ret;
	struct request *rq;
	struct file_data *data;

	data = file_data_init();

	/* fill data->file_name with name of the file being requested */
	rq = request_init(connfd, data);
	if (!rq) {
		file_data_free(data);
		return;
	}
	/* read file, 
	 * fills data->file_buf with the file contents,
	 * data->file_size with file size. */
	// ret = request_readfile(rq);
	// if (ret == 0) { /* couldn't read file */
	// 	goto out;
	// }
	
	
	struct file_data* cache_file = NULL;

	if(sv->max_cache_size >0){
		pthread_mutex_lock(ht->table_lock);
		cache_file = cache_lookup(data, 0);
		pthread_mutex_unlock(ht->table_lock);
	}
	if(cache_file){
		request_set_data(rq, cache_file);
		request_sendfile(rq);
	}else{
		request_readfile(rq);
		request_sendfile(rq);

		if(sv->max_cache_size >0){
			pthread_mutex_lock(ht->table_lock);
			
			struct file_data* returned_data = cache_lookup(data, 1);
			if(!returned_data){
				cache_insert(data);
			}


			pthread_mutex_unlock(ht->table_lock);
		}



	}

	request_destroy(rq);
}

/* entry point functions */

void *worker_thread(void* sv){

struct server* request_sv = (struct server *)sv;

while(1){
	pthread_mutex_lock(request_sv->lock);

	while(request_sv->in == request_sv->out){
		pthread_cond_wait(request_sv->cv_empty, request_sv->lock);
		if(request_sv->exiting == 1){
		pthread_mutex_unlock(request_sv->lock);
		pthread_exit(0);
		}
	}
	int msg = request_sv->buf[request_sv->out];

	if((request_sv->in - request_sv->out + request_sv->max_requests + 1)%(request_sv->max_requests + 1) == request_sv->max_requests){
		pthread_cond_broadcast(request_sv->cv_full);
		
	}

	

	request_sv->out = (request_sv->out + 1)%(request_sv->max_requests + 1);
	pthread_mutex_unlock(request_sv->lock);
	do_server_request(request_sv, msg);

	
}
	

	return NULL;



}








struct server *
server_init(int nr_threads, int max_requests, int max_cache_size)
{
	struct server *sv;

	sv = Malloc(sizeof(struct server));
	sv->nr_threads = nr_threads;
	sv->max_requests = max_requests;
	sv->max_cache_size = max_cache_size;
	sv->exiting = 0;
	
	if (nr_threads > 0 || max_requests > 0 || max_cache_size > 0) {

		if(max_cache_size > 0){
			ht = malloc(sizeof(hashTable));
			ht->size = 10000000;
			ht->table = malloc(sizeof(entry*)*10000000);
			ht->LRU_Cache = NULL;
			ht->cache_size = max_cache_size;
			ht->available_cache = max_cache_size;
			ht->table_lock = malloc(sizeof(pthread_mutex_t));
			pthread_mutex_init(ht->table_lock, NULL);
			for(int i = 0; i < ht->size; i++){
				ht->table[i] = NULL;
			}
		}



		
		sv->cv_full = malloc(sizeof (pthread_cond_t));
		sv->cv_empty = malloc(sizeof(pthread_cond_t));
		sv->lock = malloc(sizeof(pthread_mutex_t));
		pthread_cond_init(sv->cv_full, NULL);
		pthread_cond_init(sv->cv_empty, NULL);
		pthread_mutex_init(sv->lock, NULL);
		sv->buf = malloc((max_requests +1)*sizeof(int));
		sv->in = 0;
		sv->out = 0;

		sv->worker_threads = malloc(nr_threads *sizeof(pthread_t));
		for(int i = 0; i < nr_threads; i++){
			pthread_create(&sv->worker_threads[i], NULL, worker_thread, (void*)sv);
		}



	}

	/* Lab 4: create queue of max_request size when max_requests > 0 */

	/* Lab 5: init server cache and limit its size to max_cache_size */

	/* Lab 4: create worker threads when nr_threads > 0 */

	return sv;
}

void
server_request(struct server *sv, int connfd)
{
	if (sv->nr_threads == 0) { /* no worker threads */
		do_server_request(sv, connfd);
	} else {
		/*  Save the relevant info in a buffer and have one of the
		 *  worker threads do the work. */
		pthread_mutex_lock(sv->lock);
		while((sv->in - sv->out + sv->max_requests + 1)%(sv->max_requests+1) == sv->max_requests){
			pthread_cond_wait(sv->cv_full, sv->lock);
		}
		sv->buf[sv->in] = connfd;
		if(sv->in == sv->out){
			pthread_cond_broadcast(sv->cv_empty);
		}

		sv->in = (sv->in + 1)%(sv->max_requests + 1);
		pthread_mutex_unlock(sv->lock);
	}
}

void
server_exit(struct server *sv)
{
	/* when using one or more worker threads, use sv->exiting to indicate to
	 * these threads that the server is exiting. make sure to call
	 * pthread_join in this function so that the main server thread waits
	 * for all the worker threads to exit before exiting. */
	

	//wake up all threads
	sv->exiting = 1;
	
	pthread_cond_broadcast(sv->cv_empty);
	
	
	
	//join all threads 
	for(int i = 0; i < sv->nr_threads; i++){
		pthread_join(sv->worker_threads[i], NULL);
	}

	free(sv->buf);
	free(sv->cv_empty);
	free(sv->cv_full);
	free(sv->lock);
	free(sv->worker_threads);

	/* make sure to free any allocated resources */
	free(sv);
}
#include <assert.h>
#include <stdlib.h>
#include <ucontext.h>
#include "thread.h"
#include "interrupt.h"
#include <stdbool.h>








typedef struct thread {
	Tid threadID;
	int state; //0 is 
	ucontext_t context;
	void* stack_address;
	
	int exit; //if 1, thread should exit 
    struct thread *next;

}Thread;

struct wait_queue {
	Thread *wait_head;
	int size;
	
};



Thread *ready_q = NULL;
Thread *exit_q = NULL;
Thread *curr_thread = NULL; //current thread thats running 
int tidArray[THREAD_MAX_THREADS]; //set it to 1 if its taken 
struct wait_queue* wq[THREAD_MAX_THREADS];




Thread first_thread; //first thread create static globally 


//helper functions 
void printQueue(Thread*ready_q);
void insert_to_queue(Thread* newThread);
void insert_to_exitQueue(Thread* newThread);
Thread* dequeue_thread(Tid want_tid);
void delete_exit_queue();
void insert_to_wait(Thread* newThread, struct wait_queue *wq);

void printQueue(Thread*ready_q){
	
	Thread*q = ready_q;
	//printf("hi\n");
	while(q != NULL){
		//printf("hi from printQ with currently running thread %d\n", curr_thread->threadID);
		printf("%i ->", (int)q->threadID);
		q = q->next;
	}
	printf("\n");
}


void insert_to_queue(Thread* newThread){

	if(ready_q == NULL){
		ready_q = newThread;
	}else{
		Thread* tmp = ready_q;

		while (tmp->next != NULL)
		{
			tmp = tmp->next;
		}
		tmp->next = newThread;
		
	}
}
void insert_to_exitQueue(Thread* newThread){

	if(exit_q == NULL){
		exit_q = newThread;
	}else{
		Thread* tmp = exit_q;

		while (tmp->next != NULL)
		{
			tmp = tmp->next;
		}
		tmp->next = newThread;
		
	}
}

Thread* dequeue_thread(Tid want_tid){

		Thread * temp = ready_q;
		Thread * prev = ready_q;

		while(temp->threadID != want_tid){
			prev = temp;
			temp = temp->next;
		}
		if(ready_q->threadID == want_tid){
			ready_q = ready_q->next;
		}else{
			prev->next = temp->next;
		}

		temp->next = NULL;

	return temp;


}



void delete_exit_queue(){ // delete exit queue 

	Thread *delete = exit_q;

	while(delete != NULL){
		Thread *tmp = delete;
		delete = delete->next;

		if(tmp->threadID != 0){ 

			//printf("Deleting thread id %d\n", tmp->threadID);
			free(tmp->stack_address);
			tmp->stack_address = NULL;
		}
		free(tmp);
		tmp = NULL;

	}

	exit_q = NULL;

}



void thread_stub(void (*thread_main)(void *), void *arg){

	interrupts_set(1);
	
	thread_main(arg);
	thread_exit();
}




void
thread_init(void){

	
	getcontext(&(first_thread.context));

	   //new state = 0 recover state = 1 
	first_thread.next = NULL;
	first_thread.stack_address = NULL;
	first_thread.exit = 0;
	first_thread.threadID = 0;
	first_thread.state = 0;

	int i;
	for(i = 0; i < THREAD_MAX_THREADS; i++){
		tidArray[i] = 0;
		wq[i] = NULL;
	}

	

	tidArray[0] = 1;
	curr_thread = &first_thread;

}


Tid
thread_id()
{
	
	return curr_thread->threadID;
}


Tid
thread_create(void (*fn) (void *), void *parg)
{
	
	Tid ret;
	int enable = interrupts_set(0);
	int i = 0;
	while(i < THREAD_MAX_THREADS && tidArray[i] == 1){
		i++;
	}
	
	
	
	if(i == THREAD_MAX_THREADS){
		//fprintf(stderr, "No more threads can be created");
		return THREAD_NOMORE;
	}else{ //there's still space
		ret = i; // new thread id
		tidArray[ret] = 1;
	}

	Thread *new_thread = (Thread*)malloc(sizeof(Thread));


	if(new_thread == NULL){

		//fprintf(stderr, "Threads have no memory\n");
		return THREAD_NOMEMORY; //cant create new threads 
	}

	getcontext(&(new_thread->context));
	new_thread->threadID = ret;
	new_thread->exit = 0;
	new_thread->next = NULL;
	new_thread->state = 0;

	void * stack_ptr =  malloc((THREAD_MIN_STACK + 16)/ 16 *16);
	
	if(stack_ptr == NULL){

		return THREAD_NOMEMORY;
	
	}
	new_thread->stack_address = stack_ptr;

	new_thread->context.uc_mcontext.gregs[REG_RSP] = (long long int)(stack_ptr + THREAD_MIN_STACK -8);
	new_thread->context.uc_mcontext.gregs[REG_RBP] = (long long int)stack_ptr;
	new_thread->context.uc_mcontext.gregs[REG_RIP] = (long long int)thread_stub;
	//initialize the argument pointer to the new thread 
	new_thread->context.uc_mcontext.gregs[REG_RDI] = (long long int)fn;
	new_thread->context.uc_mcontext.gregs[REG_RSI] = (long long int)parg;


	insert_to_queue(new_thread);
	
	
	interrupts_set(enable);
	return new_thread->threadID;





}

Tid
thread_yield(Tid want_tid)
{
	
	int enable = interrupts_set(0);
	if(want_tid >= -2 && want_tid < THREAD_MAX_THREADS){ // make sure the id is within the range 
		
		if(want_tid == THREAD_ANY && ready_q == NULL){
			interrupts_set(enable);
			return THREAD_NONE;
		}	

		if(want_tid > 0 && tidArray[want_tid] == 0){
			interrupts_set(enable);
			return THREAD_INVALID;
		}
	}else{
		interrupts_set(enable);
		return THREAD_INVALID;
		}
	

	
	if(want_tid == THREAD_ANY){

		//printQueue(ready_q);


		Thread *temp = ready_q;
		ready_q = ready_q->next;
		temp->next = NULL;

		Tid ret = temp->threadID;

		insert_to_queue(curr_thread);
		curr_thread->state = 0;

		getcontext(&(curr_thread->context));

		if(curr_thread->exit == 1){
			thread_exit();
		}

		if(exit_q != NULL){
			//printf("Exit que not empty\n");
			delete_exit_queue();
		}

		if(!tidArray[ret]){
			
			interrupts_set(enable);
			return ret;
		}

		if(temp->state == 0 && curr_thread->state != 1){

			temp->state = 1;
			curr_thread = temp;
			setcontext(&(temp->context));
		}
		
		interrupts_set(enable);
		return ret;
	}else if(want_tid == THREAD_SELF || want_tid == curr_thread->threadID){

		curr_thread->state = 0;
		getcontext(&(curr_thread->context));

		if(curr_thread->state == 0){
			curr_thread->state = 1;
			setcontext(&(curr_thread->context));
		}
		
		interrupts_set(enable);
		return curr_thread->threadID;
	}else{

		Thread *temp = dequeue_thread(want_tid);
		
		Tid ret = temp->threadID;
		insert_to_queue(curr_thread);
		curr_thread->state = 0;
		getcontext(&(curr_thread->context));
	
		if(curr_thread->exit == 1){
			thread_exit();
		}

		if(exit_q != NULL){
			delete_exit_queue();
			//printf("Exit que not empty\n");
		}

		if(!tidArray[ret]){
			
			interrupts_set(enable);
			return ret;
		}

		if(temp->state == 0 && curr_thread->state != 1){
			temp->state = 1;
			curr_thread = temp;
			setcontext(&(temp->context));
		}
		
		interrupts_set(enable);
		return ret;


	}


	return THREAD_FAILED;

}

void
thread_exit()
{ 
	interrupts_enabled(0);
	thread_wakeup(wq[curr_thread->threadID], 1); //wake up all threads waiting
	
	wait_queue_destroy(wq[curr_thread->threadID]);
	//printf("after destroy\n");
	wq[curr_thread->threadID] = NULL;
	if(ready_q == NULL){
		exit(0);
	}
	
	
	Thread *curr = curr_thread;
	Thread *new = ready_q;
	ready_q = ready_q->next;
	curr->next = NULL;
	tidArray[curr->threadID] = 0;
	
	
	
	
	insert_to_exitQueue(curr);
	new->next = NULL;
	new->state = 1;
	curr_thread = new;
	setcontext(&(new->context));
	
	

}

Tid
thread_kill(Tid tid)
{
	
	int enable = interrupts_set(0);
	if(tid == THREAD_SELF || tid == curr_thread->threadID || tid < -2 || tid > THREAD_MAX_THREADS -1){
		
		return THREAD_INVALID;
	}

	if(tid > 0 && tidArray[tid] == 0){
		
		return THREAD_INVALID;
	}


	if(tid == THREAD_ANY){
		if(ready_q == NULL){
			
			return THREAD_INVALID;
		}

		Thread* temp = ready_q;
		Tid ret = temp->threadID;
		temp->exit = 1;
		interrupts_set(enable);
		return ret;

	}else{
		

		int i;
			for(i = 0; i < THREAD_MAX_THREADS; i ++){ // searches the wait q

				if(wq[i] != NULL){
					Thread *thread_in_wait = wq[i]->wait_head;
					while(thread_in_wait->threadID != tid && thread_in_wait != NULL){
						thread_in_wait = thread_in_wait->next;
					}
					if(thread_in_wait != NULL){
						Tid ret = thread_in_wait->threadID;
						thread_in_wait->exit = 1;
						interrupts_set(enable);
						return ret;
					} //found
				}
				

			}

		Thread* temp = ready_q;
		
		while(temp->threadID != tid && temp != NULL){ //search the ready q first
			
			temp = temp->next;
		}

		
		Tid ret = temp->threadID;
		temp->exit = 1;
		interrupts_set(enable);
		return ret;




	}
	
interrupts_set(enable);
 return THREAD_FAILED;
}

/*******************************************************************
 * Important: The rest of the code should be implemented in Lab 3. *
 *******************************************************************/

/* make sure to fill the wait_queue structure defined above */
struct wait_queue *
wait_queue_create()
{
	struct wait_queue *wq;

	wq = malloc(sizeof(struct wait_queue));
	assert(wq);
	wq->wait_head = NULL;
	wq->size = 0;

	return wq;
}

void
wait_queue_destroy(struct wait_queue *wq)
{
	if(wq == NULL){ //nothing to delete
		return;
	}

	Thread* temp = wq->wait_head;

	while(temp!= NULL){
		Thread* delete = temp;
		temp = temp->next;

		if(delete->threadID != 0){
			free(delete->stack_address);
			delete->stack_address = NULL;
		}
		free(delete);
		delete = NULL;
	}


	free(wq);
}

void insert_to_wait(Thread* newThread, struct wait_queue *wq){

	Thread* head = wq->wait_head;
	
	
	if(head == NULL){
		wq->wait_head = newThread;
		wq->size++;
	}else{
		Thread* tmp = wq->wait_head;

		while (tmp->next != NULL)
		{
			tmp = tmp->next;
		}
		tmp->next = newThread;
		tmp->next->next = NULL;
		wq->size++;
		
	}
}



Tid
thread_sleep(struct wait_queue *queue)
{
	
	
	int enable = interrupts_set(0);

	if(queue == NULL){

		interrupts_set(enable);
		return THREAD_INVALID;
	}else if (ready_q == NULL){
		interrupts_set(enable);
		return THREAD_NONE;
	}else{

		Thread *temp = ready_q;
		ready_q = ready_q->next;
		temp->next = NULL;
		
		Tid ret = temp->threadID;

		insert_to_wait(curr_thread, queue);
		curr_thread->state = 0; //set it to new 
		getcontext(&(curr_thread->context));

		if(curr_thread->exit == 1){
			thread_exit();
		}

		if(exit_q != NULL){
			//printf("Exit que not empty\n");
			delete_exit_queue();
		}

		if(!tidArray[ret]){
			
			interrupts_set(enable);
			return ret;
		}

		if(temp->state == 0 && curr_thread->state != 1){

			temp->state = 1;
			curr_thread = temp;
			setcontext(&(temp->context));
		}
		
		interrupts_set(enable);
		return ret;
	}
	interrupts_set(enable);
	return THREAD_FAILED;
}

/* when the 'all' parameter is 1, wakeup all threads waiting in the queue.
 * returns whether a thread was woken up on not. */
int
thread_wakeup(struct wait_queue *queue, int all)
{
	int enable = interrupts_set(0);

	if(queue == NULL){
		interrupts_set(enable);
		return 0;
	}else if(queue->wait_head == NULL){
		interrupts_set(enable);
		return 0;
	}else{

		if(all == 0){ //wake up one thread
			if(queue->size == 1){
				insert_to_queue(queue->wait_head);
				queue->wait_head = NULL;
			}else{
				Thread* temp = queue->wait_head;
				queue->wait_head = queue->wait_head->next;
				queue->size --;
				temp->next = NULL;
				insert_to_queue(temp);
			}
			interrupts_set(enable);
			return 1;
		}else if(all == 1){ // wake up all in wait queue
			
			int size = queue->size;
			Thread* temp = queue->wait_head;
			insert_to_queue(temp);
			queue->wait_head = NULL;
			queue->size = 0;
			interrupts_set(enable);
			return size;
		}



	}
	interrupts_set(enable);
	return 0;
}

/* suspend current thread until Thread tid exits */
Tid
thread_wait(Tid tid)
{
	int enable = interrupts_set(0);

	// int exit_found = 0;
	// Thread* exit = exit_q;
	// while(exit != NULL){
	// 	if(exit->threadID == tid){
	// 		exit_found = 1;
	// 		break;
	// 	}else{
	// 		exit = exit->next;
	// 	}
	// }
	// int ready_found = 0;
	// int wait_found = 0;

	// Thread* ready = ready_q;
	// while(ready->threadID != tid && ready != NULL){
	// 	if(ready->threadID == tid){
	// 		ready_found = 1;
	// 		break;
	// 	}else{
	// 		ready = ready->next;
	// 	}
	// }

	
	// for(int j = 0; j < THREAD_MAX_THREADS; j++){
		
	// 	if(wq[j]!= NULL){
	// 		Thread* thread_in_wait = wq[j]->wait_head;
	// 		while(thread_in_wait->threadID != tid && thread_in_wait != NULL){
	// 			if(thread_in_wait->threadID == tid){
	// 				wait_found = 1;
	// 				break;
	// 			}
	// 			thread_in_wait = thread_in_wait->next;
	// 		}
	// 	}
		
	// }


	if(tidArray[tid] == 0 || tidArray[tid] == THREAD_SELF || tid == curr_thread->threadID || tid < 0 ){
		interrupts_set(enable);
		return THREAD_INVALID;
	}

	

	if(wq[tid] == NULL){

		wq[tid] = wait_queue_create();
		thread_sleep(wq[tid]);
	}else{
		thread_sleep(wq[tid]);
	}

	
	

	//printQueue(wq[tid]->wait_head);

	interrupts_set(enable);
	return tid;
}

struct lock {
	struct wait_queue *wq;
	int lock_state;
};

struct lock *
lock_create()
{
	struct lock *lock;

	lock = malloc(sizeof(struct lock));
	assert(lock);
	lock->wq = wait_queue_create();
	lock->lock_state = 0;
	

	return lock;
}

void
lock_destroy(struct lock *lock)
{
	assert(lock != NULL);

	wait_queue_destroy(lock->wq);

	free(lock);
}

void
lock_acquire(struct lock *lock)
{
	assert(lock != NULL);
	int enable = interrupts_set(0);
	while(lock->lock_state == 1){
		thread_sleep(lock->wq);
	}
	lock->lock_state = 1;
	interrupts_set(enable);
	
}

void
lock_release(struct lock *lock)
{
	assert(lock != NULL);
	int enable = interrupts_set(0);
	lock->lock_state = 0;
	thread_wakeup(lock->wq, 1);
	interrupts_set(enable);
	
}

struct cv {
	struct wait_queue *wq;
};

struct cv *
cv_create()
{
	struct cv *cv;

	cv = malloc(sizeof(struct cv));
	assert(cv);
	cv->wq = wait_queue_create();

	

	return cv;
}

void
cv_destroy(struct cv *cv)
{
	assert(cv != NULL);
	wait_queue_destroy(cv->wq);
	

	free(cv);
}

void
cv_wait(struct cv *cv, struct lock *lock)
{
	assert(cv != NULL);
	assert(lock != NULL);
	lock_release(lock);
	thread_sleep(cv->wq);
	lock_acquire(lock);
	
}

void
cv_signal(struct cv *cv, struct lock *lock)
{
	assert(cv != NULL);
	assert(lock != NULL);
	thread_wakeup(cv->wq, 0);
	
}

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
	assert(cv != NULL);
	assert(lock != NULL);
	thread_wakeup(cv->wq, 1);
	
}

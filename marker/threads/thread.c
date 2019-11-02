#include <assert.h>
#include <stdlib.h>
#include <ucontext.h>
#include "thread.h"
#include "interrupt.h"
#include <stdbool.h>

struct wait_queue {
	
};





typedef struct thread {
	Tid threadID;
	int state; //0 is 
	ucontext_t context;
	void* stack_address;
	
	int exit; //if 1, thread should exit 
    struct thread *next;

}Thread;


Thread *ready_q = NULL;
Thread *exit_q = NULL;
Thread *curr_thread = NULL; //current thread thats running 
int tidArray[THREAD_MAX_THREADS]; //set it to 1 if its taken 

Thread first_thread; //first thread create static globally 


//helper functions 
void printQueue(Thread*ready_q);
void insert_to_queue(Thread* newThread);
void insert_to_exitQueue(Thread* newThread);
Thread* dequeue_thread(Tid want_tid);
void delete_exit_queue();


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
	
	return new_thread->threadID;





}

Tid
thread_yield(Tid want_tid)
{
	
	if(want_tid >= -2 && want_tid < THREAD_MAX_THREADS){ // make sure the id is within the range 
		
		if(want_tid == THREAD_ANY && ready_q == NULL){
			return THREAD_NONE;
		}	

		if(want_tid > 0 && tidArray[want_tid] == 0){
			return THREAD_INVALID;
		}
	}else{
		return THREAD_INVALID;
		}
	

	
	if(want_tid == THREAD_ANY){

		

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
			return ret;
		}

		if(temp->state == 0 && curr_thread->state != 1){

			temp->state = 1;
			curr_thread = temp;
			setcontext(&(temp->context));
		}

		return ret;
	}else if(want_tid == THREAD_SELF || want_tid == curr_thread->threadID){

		curr_thread->state = 0;
		getcontext(&(curr_thread->context));

		if(curr_thread->state == 0){
			curr_thread->state = 1;
			setcontext(&(curr_thread->context));
		}
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
			return ret;
		}

		if(temp->state == 0 && curr_thread->state != 1){
			temp->state = 1;
			curr_thread = temp;
			setcontext(&(temp->context));
		}
		return ret;


	}


	return THREAD_FAILED;

}

void
thread_exit()
{ 
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
		return ret;

	}else{
		if(ready_q == NULL){
			return THREAD_INVALID;
		}

		Thread* temp = ready_q;

		while(temp->threadID != tid){
			temp = temp->next;
		}
		Tid ret = temp->threadID;
		temp->exit = 1;
		return ret;




	}
	

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

	TBD();

	return wq;
}

void
wait_queue_destroy(struct wait_queue *wq)
{
	TBD();
	free(wq);
}

Tid
thread_sleep(struct wait_queue *queue)
{
	TBD();
	return THREAD_FAILED;
}

/* when the 'all' parameter is 1, wakeup all threads waiting in the queue.
 * returns whether a thread was woken up on not. */
int
thread_wakeup(struct wait_queue *queue, int all)
{
	TBD();
	return 0;
}

/* suspend current thread until Thread tid exits */
Tid
thread_wait(Tid tid)
{
	TBD();
	return 0;
}

struct lock {
	/* ... Fill this in ... */
};

struct lock *
lock_create()
{
	struct lock *lock;

	lock = malloc(sizeof(struct lock));
	assert(lock);

	TBD();

	return lock;
}

void
lock_destroy(struct lock *lock)
{
	assert(lock != NULL);

	TBD();

	free(lock);
}

void
lock_acquire(struct lock *lock)
{
	assert(lock != NULL);

	TBD();
}

void
lock_release(struct lock *lock)
{
	assert(lock != NULL);

	TBD();
}

struct cv {
	/* ... Fill this in ... */
};

struct cv *
cv_create()
{
	struct cv *cv;

	cv = malloc(sizeof(struct cv));
	assert(cv);

	TBD();

	return cv;
}

void
cv_destroy(struct cv *cv)
{
	assert(cv != NULL);

	TBD();

	free(cv);
}

void
cv_wait(struct cv *cv, struct lock *lock)
{
	assert(cv != NULL);
	assert(lock != NULL);

	TBD();
}

void
cv_signal(struct cv *cv, struct lock *lock)
{
	assert(cv != NULL);
	assert(lock != NULL);

	TBD();
}

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
	assert(cv != NULL);
	assert(lock != NULL);

	TBD();
}

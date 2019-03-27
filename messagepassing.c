#include <stdio.h>
#include <pthread.h>
#include <sys/prctl.h>

#ifdef DEBUG_ME
#define	TRACE	printf("%s(%d)\n",__func__,__LINE__)
#else
#define TRACE
#endif


#define MAX_MESSAGES	5

typedef enum _message_type 
{
	BUTTON_BEGIN, BUTTON_LEFT,BUTTON_RIGHT, BUTTON_UP, BUTTON_DOWN, BUTTON_ENTER, BUTTON_UNKNOWN
}message_type;


pthread_mutex_t message_mutex;
pthread_cond_t message_condition;

typedef struct _message
{
	message_type	emsg;
	void (*completed) (void);
}message;

static int total_messages = 0;
static int message_read_location = 0;


message _message_queue[MAX_MESSAGES] = {0};

int post_msg(message *pmsg)
{
	int return_val;
	TRACE;
	return_val = -1;
	do
	{
		if(NULL == pmsg)	break;
		
		if(total_messages >= MAX_MESSAGES) break;
		
		pthread_mutex_lock(&message_mutex);
		_message_queue[total_messages].emsg = pmsg->emsg;
		_message_queue[total_messages].completed = pmsg->completed;
		
		// printf("callback : %p\n",_message_queue[total_messages].completed);
		total_messages++;
		pthread_cond_signal(&message_condition);
        pthread_mutex_unlock(&message_mutex);
				
		return_val = 0;
		
	}while(0);
	
	return return_val;
}

/*
int post_msg(message *pmsg)
{
	int return_val;
	TRACE;
	return_val = -1;
	do
	{
		if(NULL == pmsg)	break;
		
		if(total_messages >= MAX_MESSAGES) break;
		
		pthread_mutex_lock(&message_mutex);
		_message_queue[total_messages].emsg = pmsg->emsg;
		_message_queue[total_messages].completed = pmsg->completed;
		
		// printf("callback : %p\n",_message_queue[total_messages].completed);
		total_messages++;
		pthread_cond_signal(&message_condition);
        pthread_mutex_unlock(&message_mutex);
				
		return_val = 0;
		
	}while(0);
	
	return return_val;
}
*/
/*
int recv_msg(message *pmsg)
{

	int return_val;
	TRACE;
	return_val = -1;
	do
	{
		if(NULL == pmsg)	break;
		
		if(0 == total_messages) 
		{
				// No messages, wait for message
                pthread_mutex_lock(&message_mutex);
                pthread_cond_wait(&message_condition, &message_mutex);
				pthread_mutex_unlock(&message_mutex);
		}else
		{

			pthread_mutex_lock(&message_mutex);
			// Read current message
			pmsg->emsg = _message_queue[message_read_location].emsg;
			pmsg->completed = _message_queue[message_read_location].completed;
			total_messages--;
			pthread_mutex_unlock(&message_mutex);
			return_val = 0;
			// Update count index for next message
			message_read_location++;
			
			if(message_read_location >= MAX_MESSAGES)
			{
				message_read_location = 0;
			}
		}
	}while(0);

	return return_val;
}

*/
// FIXME: message queue not working.., currently its message stack...
int recv_msg(message *pmsg)
{

	int return_val;
	TRACE;
	return_val = -1;
	do
	{
		if(NULL == pmsg)	break;
		
		// if(0 == total_messages) break;
		
		// No messages, wait for message
		pthread_mutex_lock(&message_mutex);
		pthread_cond_wait(&message_condition, &message_mutex);
		pthread_mutex_unlock(&message_mutex);
		

		pthread_mutex_lock(&message_mutex);
		// Read current message
		total_messages--;
		pmsg->emsg = _message_queue[total_messages].emsg;
		pmsg->completed = _message_queue[total_messages].completed;

		pthread_mutex_unlock(&message_mutex);
		return_val = 0;

	}while(0);

	return return_val;
}


static int processing_done = 0;

void processed(void)
{
	TRACE;
	processing_done = 1;
	// printf("\t\t %p\n",processed);
	// printf("%s\n",__func__);
}

void * consumer(void *p)
{
	message msg;
    prctl(PR_SET_NAME,"consumer",0,0,0);
	TRACE;
	while(1)
	{
		recv_msg(&msg);

		printf("message : %d\n",msg.emsg);
		switch(msg.emsg)
		{
			case BUTTON_UP:
			{
				printf("BUTTON_UP\n");
				msg.completed();
				sleep(1);
			}break;
			
			case BUTTON_DOWN:
			{
				printf("BUTTON_DOWN\n");
				msg.completed();
				sleep(2);
				
			}break;
			
			case BUTTON_LEFT:
			{
				printf("BUTTON_LEFT\n");
				msg.completed();
				sleep(3);
				// printf("\t\t %p\n",msg.completed);

			}break;
			
			case BUTTON_RIGHT:
			{
				printf("BUTTON_RIGHT\n");
				msg.completed();
				sleep(4);

			}break;
			
			case BUTTON_ENTER:
			{
				printf("BUTTON_ENTER\n");
				msg.completed();
				sleep(3);

			}break;
		}
		
	}
	return NULL;
}



void *producer(void *p)
{
	message_type	m;
	message msg;
	prctl(PR_SET_NAME,"producer",0,0,0);
	TRACE;
	m = BUTTON_RIGHT;
	
	while(1)
	{
		sleep(2);
		// m++;
		// if(m >= BUTTON_UNKNOWN) m = 1;
		
		msg.emsg = BUTTON_RIGHT;
		msg.completed = processed;
		post_msg(&msg);

		// sleep(1);
		msg.emsg = BUTTON_LEFT;
		post_msg(&msg);

		/// sleep(1);
		msg.emsg = BUTTON_UP;
		post_msg(&msg);
		// sleep(1);
/*		sleep(2);
		msg.emsg = BUTTON_DOWN;
		post_msg(&msg);

		sleep(1);
		// sleep(1);
		msg.emsg = BUTTON_ENTER;
		post_msg(&msg);
*/


		// Wait for message processing...,
		/*while(!processing_done)
		{
			sleep(2);
		}
		processing_done = 0;
		*/
	}
	
	retrun NULL;
}


#define NUM_THREADS  2
pthread_t threads[NUM_THREADS];

int main (int argc, char *argv[])
{
  int i, rc;

  pthread_attr_t attr;

  /* Initialize mutex and condition variable objects */
  pthread_mutex_init(&message_mutex, NULL);
  pthread_cond_init (&message_condition, NULL);

  /* For portability, explicitly create threads in a joinable state */
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_create(&threads[0], &attr, producer, NULL);
  pthread_create(&threads[1], &attr, consumer, NULL);

  prctl(PR_SET_NAME,"main",0,0,0);
  TRACE;
  pthread_join(threads[0], NULL);
  pthread_join(threads[1], NULL); 

  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&message_mutex);
  pthread_cond_destroy(&message_condition);
  pthread_exit(NULL);
  
 }
  

#include <stdio.h>
#include <pthread.h>
#include <sys/prctl.h>

#ifdef DEBUG_ME
#define	TRACE	printf("%s(%d)\n",__func__,__LINE__)
#else
#define TRACE
#endif

pthread_mutex_t m_SuspendMutex;
pthread_cond_t m_ResumeCond;
int suspended = 0;
enum {false,true};

void suspendMe() {
	pthread_mutex_lock(&m_SuspendMutex);
	suspended = true;
	do {
		pthread_cond_wait(&m_ResumeCond, &m_SuspendMutex);
	} while (suspended);
	pthread_mutex_unlock(&m_SuspendMutex);
}

void resume() {
	/* The shared state 'suspended' must be updated with the mutex held. */
	pthread_mutex_lock(&m_SuspendMutex);
	suspended = false;
	pthread_cond_signal(&m_ResumeCond);
	pthread_mutex_unlock(&m_SuspendMutex);
}
	

void * consumer(void *p)
{
    prctl(PR_SET_NAME,"consumer",0,0,0);
	TRACE;
	while(1)
	{
		suspendMe();
		
		printf("Can start life now as planet is in supportive environmental condition\n");
		
	}
	return NULL;
}



void *producer(void *p)
{

	prctl(PR_SET_NAME,"producer",0,0,0);
	TRACE;
	while(1)
	{
		sleep(3);
		resume();
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
  pthread_mutex_init(&m_SuspendMutex, NULL);
  pthread_cond_init (&m_ResumeCond, NULL);

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
  pthread_mutex_destroy(&m_SuspendMutex);
  pthread_cond_destroy(&m_ResumeCond);
  pthread_exit(NULL);
  
 }
  

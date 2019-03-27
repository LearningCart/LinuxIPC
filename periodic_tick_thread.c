#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS  3
#define TCOUNT 12
#define COUNT_LIMIT 12

#define DEFAULT_NTD	3

pthread_t threads[NUM_THREADS];

int SetTemp = 0;

int Result_Val = 0;
int SetFilt = 0;

int cycle = 0;

pthread_mutex_t count_mutex;
pthread_cond_t count_threshold_cv;


void *toggleresult(void *p)
{
	int count = 0;
	while(1)
	{
		count ++;
		if(count >  8)
		{
			Result_Val = 1;
			// dont' reset count count = 0;
		} else
		{
			Result_Val = 0;
		}

		if(count > 20) count = 0;
		printf("Result : %d, Output: %d\n",Result_Val,SetFilt);
		sleep(1);
	}

	pthread_exit(NULL);
}

void *post_tick(void *t) 
{
	while(1)
	{
		sleep(1);
    		pthread_mutex_lock(&count_mutex);
		 // printf("."); fflush(stdout);
     		pthread_cond_signal(&count_threshold_cv);
   		pthread_mutex_unlock(&count_mutex);
	}

    /* Do some "work" so threads can alternate on mutex lock */
  pthread_exit(NULL);
}

void *watch_count(void *t) 
{
	while(1)
	{
		pthread_mutex_lock(&count_mutex);
		pthread_cond_wait(&count_threshold_cv, &count_mutex);

		// Tick occured..,
		//
		// check NTD
		//
		if(Result_Val == 0)
		{
			SetTemp = 0;
			SetFilt      = 0;
		} else 
		{
			// Now result is true we must count 
			if(0 == SetFilt)
			{
				//  If not set already...,
				//
				if(SetTemp > 1)
				{
					SetTemp--;
				}else if (SetTemp == 1)
				{				
					SetFilt = 1;
				} else
				{
					// It is not set,, lets load default delay for which vari should be ON
					SetTemp = DEFAULT_NTD; 
				}
			}
		}

		pthread_mutex_unlock(&count_mutex);

	}
	pthread_exit(NULL);
}

int main (int argc, char *argv[])
{
  int i, rc;

  pthread_attr_t attr;

  /* Initialize mutex and condition variable objects */
  pthread_mutex_init(&count_mutex, NULL);
  pthread_cond_init (&count_threshold_cv, NULL);

  /* For portability, explicitly create threads in a joinable state */
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_create(&threads[0], &attr, watch_count, NULL);
  pthread_create(&threads[1], &attr, post_tick, NULL);
  pthread_create(&threads[2], &attr, toggleresult, NULL);
  /// pthread_create(&threads[2], &attr, inc_count, (void *)t3);

  /* Wait for all threads to complete */
  for (i=0; i<NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }
  printf ("Main(): Waited on %d  threads. Done.\n", NUM_THREADS);

  /* Clean up and exit */
  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&count_mutex);
  pthread_cond_destroy(&count_threshold_cv);
  pthread_exit(NULL);

}



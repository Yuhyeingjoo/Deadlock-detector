  
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include<pthread.h>
#include <unistd.h>
#define MN 10
pthread_mutex_t m[MN];
void *th(void* n ){
	int n_mutex = *(int*)n;
	int randn = rand()%MN;
	printf("%ld\n",(long)pthread_self());
	pthread_mutex_lock(&m[n_mutex]);

	int randn2 = rand()%MN;
	int randn3 = rand()%MN;
	pthread_mutex_lock(&m[randn2]);
	pthread_mutex_lock(&m[randn]);
	pthread_mutex_lock(&m[randn3]);

	sleep(0.8);
	pthread_mutex_unlock(&m[randn3]);
	pthread_mutex_unlock(&m[randn]);
	pthread_mutex_unlock(&m[randn2]);

	pthread_mutex_unlock(&m[n_mutex]);

	}

void *th2(){
	pthread_mutex_lock(&m[0]);
	pthread_mutex_lock(&m[1]);
	sleep(1);
	pthread_mutex_lock(&m[2]);
	sleep(1);
	pthread_mutex_unlock(&m[2]);
	pthread_mutex_unlock(&m[1]);
	pthread_mutex_unlock(&m[0]);

}

void *th3(){
	pthread_mutex_lock(&m[2]);
	sleep(1);
	pthread_mutex_lock(&m[0]);
	sleep(1);
	pthread_mutex_unlock(&m[0]);
	pthread_mutex_unlock(&m[2]);

}
int 
main ()
{
	
	srand(time(0x0));
	printf("test1 start\n");

	pthread_t threadd[7];
	
	for(int i=0; i<7; i++){
		if(pthread_create(&threadd[i], 0x0, th, (void*)&i)<0)
			printf("error\n");
		
	}

	for(int i=0; i<7; i++){
		pthread_join(threadd[i],0x0);
	}

	}

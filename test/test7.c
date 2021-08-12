#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>


pthread_mutex_t m[5];

void *th(){
	pthread_mutex_lock(&m[0]);
	pthread_mutex_lock(&m[1]);

	pthread_mutex_unlock(&m[1]);
	pthread_mutex_lock(&m[2]);
	
	pthread_mutex_lock(&m[3]);
	pthread_mutex_unlock(&m[3]);

	pthread_mutex_lock(&m[4]);
	pthread_mutex_unlock(&m[4]);
	
	pthread_mutex_unlock(&m[2]);
	pthread_mutex_unlock(&m[0]);

	
}

int main(){
	pthread_t t[3];
	pthread_create(&t[0], 0x0,th, 0x0);
	pthread_join(t[0],0x0);



}

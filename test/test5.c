#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m3 = PTHREAD_MUTEX_INITIALIZER;
void* mythr(){
    pthread_mutex_lock(&m1);
    pthread_mutex_lock(&m2);
    pthread_mutex_lock(&m3);
    printf("freeing m2\n");
    pthread_mutex_unlock(&m2);
    printf("locking m1\n");
    pthread_mutex_lock(&m1);
    pthread_mutex_unlock(&m1);
    pthread_mutex_unlock(&m3);
}
void main(){
    pthread_t thread1, thread2;
    pthread_create(&thread1, NULL, mythr, NULL);
    pthread_join(thread1, NULL);
    printf("end\n");
    exit(0);
}

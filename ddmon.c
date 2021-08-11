#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>
#include<execinfo.h>


int pthread_mutex_lock(pthread_mutex_t *mutex ){
	int fd, protocol=1, size;
	int line_n, str_size;
	void *buf[100];
	char **line;
        int (*mutex_p)(pthread_mutex_t *m);

	line_n = backtrace(buf, 100);
	line =backtrace_symbols(buf, line_n);
	if(line ==NULL){
		perror("backtrace");
		exit(EXIT_FAILURE);
	}

	char *ptr = strtok(line[1], ")");
	str_size = strlen(ptr);
	if (mkfifo("channel", 0666)) {
		if (errno != EEXIST) {
			perror("fail to make fifo: ") ;
			exit(1) ;
		}
	}

	if((fd = open("channel", O_WRONLY | O_SYNC))<0){
		printf("fail to open fifo");
		exit(1);
	}


        
	mutex_p = dlsym(RTLD_NEXT,"pthread_mutex_lock");
        long lock_n = (long)mutex, thread_id =(long)pthread_self();
	protocol = 1;
	size =sizeof(int); 
	printf("ddmon lock: %d %ld %ld\n",protocol, thread_id, lock_n);
	flock(fd,LOCK_EX);
	for(int i=0; i<4; )
		i+= write(fd,((char*)&protocol)+i,size-i);
	size = sizeof(long);
	for(int i=0; i<8; )
		i+= write(fd,((char*)&thread_id)+i,size-i);
	size = sizeof(long);
	for(int i=0; i<8; )	
		i+= write(fd,((char*)&lock_n)+i,size-i);
	size = sizeof(int);
	
	for(int i=0; i<4;)
		i+= write(fd, ((char*)&str_size)+i, size-i);
	for(int i=0; i<str_size; )
		i+= write(fd, ptr+i, str_size-i);
	
	flock(fd, LOCK_UN);
	close(fd) ;
        int ret = mutex_p(mutex);
	return ret;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex){
	int protocol = 0, str_size = 0;

	if (mkfifo("channel", 0666)) {
		if (errno != EEXIST) {
			perror("fail to open fifo: ") ;
			exit(1) ;
		}
	}

	int fd = open("channel", O_WRONLY | O_SYNC) ;


	int (*mutex_p_un)(pthread_mutex_t *m);
	char *error;
        mutex_p_un = dlsym(RTLD_NEXT,"pthread_mutex_unlock");
	long lock_n = (long)mutex , thread_id = (long)pthread_self();
	
	printf("ddmon unlock: %d %ld %ld\n",protocol, thread_id, lock_n);
	
	flock(fd,LOCK_EX);
	int size =sizeof(int); 
	for(int i=0; i<4; ){
		i+= write(fd,((char*)&protocol)+i,size-i);
	}
	size = sizeof(long);
	for(int i=0; i<8; ){	
		i+= write(fd,((char*)&thread_id)+i,size-i);
	}
	size = sizeof(long);
	for(int i=0; i<8; ){	
		i+= write(fd,((char*)&lock_n)+i,size-i);
	}
	for(int i=0; i<4;)
		i+= write(fd, ((char*)&str_size)+i, 4-i);	
	flock(fd,LOCK_UN);
	close(fd);
	return mutex_p_un(mutex);
}

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
static pthread_mutex_t fifo_m= PTHREAD_MUTEX_INITIALIZER;
int relative_addr(){
	int line_n;
	char **line;	
	void *buf[100];
	line_n = backtrace(buf, 128);
	line =backtrace_symbols(buf, line_n);
	if(line ==NULL){
		perror("backtrace");
		exit(EXIT_FAILURE);
	}
	char *ptr = strtok(line[2], "(");
	ptr = strtok(NULL, ")");
	return (int)strtol(ptr,NULL,16);
}
 
int ret_fd(){
	int fd;
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
	return fd;
}
int send_message(int fd, int protocol, long thread_id,long lock_m, int addr){
	int size =sizeof(int); 
	for(int i=0; i<4; )
		i+= write(fd,((char*)&protocol)+i,size-i);
	size = sizeof(long);
	for(int i=0; i<8; )
		i+= write(fd,((char*)&thread_id)+i,size-i);
	size = sizeof(long);
	for(int i=0; i<8; )	
		i+= write(fd,((char*)&lock_m)+i,size-i);
	size = sizeof(int);
	
	for(int i=0; i<4;)
		i+= write(fd, ((char*)&addr)+i, size-i);

}

int pthread_mutex_lock(pthread_mutex_t *mutex ){
	int fd, protocol=1;
	int str_size, addr ;
        int (*mutex_p)(pthread_mutex_t *m);
	int (*mutex_p_un) (pthread_mutex_t *m);
	char *error;
	addr = relative_addr();
	
        fd = ret_fd();
	
	mutex_p = dlsym(RTLD_NEXT,"pthread_mutex_lock");
	mutex_p_un = dlsym(RTLD_NEXT,"pthread_mutex_unlock");
	if ((error = dlerror()) != 0x0) 
		exit(1) ;
        
	long lock_m = (long)mutex, thread_id =(long)pthread_self();
	protocol = 1;
	
	printf("ddmon lock: %d %ld %ld %X\n",protocol, thread_id, lock_m, addr);
	
	if(mutex_p(&fifo_m)){
		printf("Lock error\n");
		exit(1);
	}
	send_message(fd, protocol, thread_id, lock_m, addr);
	if(mutex_p_un(&fifo_m)){
		printf("Unlock error\n");
		exit(1);
	}	
	
	close(fd) ;
        int ret = mutex_p(mutex);
	return ret;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex){
	int protocol = 0, addr = 0, fd;
	int (*mutex_p)(pthread_mutex_t *m);
	int (*mutex_p_un) (pthread_mutex_t *m);
	char *error;
	fd = ret_fd();
	
	mutex_p = dlsym(RTLD_NEXT,"pthread_mutex_lock");
	mutex_p_un = dlsym(RTLD_NEXT,"pthread_mutex_unlock");
	if ((error = dlerror()) != 0x0) 
		exit(1) ;	
	
	long lock_m = (long)mutex , thread_id = (long)pthread_self();
	
	printf("ddmon unlock: %d %ld %ld\n",protocol, thread_id, lock_m);
	
	if(mutex_p(&fifo_m)){
		printf("Lock error\n");
		exit(1);
	}
	send_message(fd, protocol, thread_id, lock_m, addr);
	if(mutex_p_un(&fifo_m)){
		printf("Unlock error\n");
		exit(1);
	}
	close(fd);
	return mutex_p_un(mutex);
}

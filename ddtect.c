#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <stdbool.h>
struct node{
        long mutex;
        struct node *next;
	int visit;
};
struct thread{
	long thread_id;
	int list_n;
	struct node* list[256];
};
static struct thread *plist[10];
static int plist_n =0;
static struct node *mlist[256];
static int mlist_n =0;
char *exefile_name =0x0 ;
struct node* new_node(long given_m){
	struct node* new_p;
	new_p = (struct node*)malloc(sizeof(struct node));
	new_p->mutex = given_m;
	new_p->visit = 0;
	new_p->next = 0x0;
	return new_p;
}
struct thread* new_thread(long given_m, long given_id){
	struct thread* new_t;
	new_t = (struct thread*)malloc(sizeof(struct thread));
	new_t->thread_id=given_id;
	new_t->list_n = 0;
}
struct node* is_in_mlist(long given_lock){
        for(int i=0; i<mlist_n; i++){
                if(mlist[i]!=0x0 && given_lock == mlist[i]->mutex)
                        return mlist[i];
        }
        return 0x0;
}
struct thread* is_in_plist(long given_id){
        for(int i=0; i<plist_n; i++){
                if(given_id==plist[i]->thread_id)
                        return plist[i];
        }
        return 0x0;
}

void graph_make(long given_id, long given_m){
	struct thread* tmp1;
	struct node* tmp2;
	if((tmp1 =is_in_plist(given_id)) == 0x0){
		tmp1 = new_thread(given_m, given_id);
		plist[plist_n++] = tmp1;
	}
	if((tmp2 = is_in_mlist(given_m))==0x0){
		tmp2 = new_node(given_m);
		mlist[mlist_n++] = tmp2;
	}
	tmp1->list[tmp1->list_n] = tmp2;
	if(tmp1->list_n>0 && tmp1->list[tmp1->list_n-1]!=0x0){
		int from = tmp1->list_n-1;
		int to = tmp1->list_n;
		tmp1->list[from]->next = tmp1->list[to];
	} 
	tmp1->list_n++;
}
void unlock_graph(long given_id, long given_m){
	struct thread* ttmp = is_in_plist(given_id);
	struct node* ntmp = is_in_mlist(given_m);
	for(int i=0; i<ttmp->list_n; i++){
		if(ttmp->list[i]!=0x0 && ttmp->list[i]->mutex == given_m){
			if(i-1>=0&&ttmp->list[i-1]!=0x0){
				ttmp->list[i-1]->next = ttmp->list[i]->next;
			}
			ttmp->list[i] = 0x0;
			ntmp->next = 0x0;
		}
	}
}
bool deadlock_detect(struct node* Node){
//        printf("visit Mutex %ld   %d\n",Node->mutex, Node->visit);
        bool is_deadlock = false;
        if(Node->visit ){
                printf("\nMut : %ld \n",Node->mutex);
                return true;
        }
        Node->visit = 1;
        if(Node->next!=0x0){
                is_deadlock = deadlock_detect(Node->next);
        }
        Node->visit = 0;
        return is_deadlock;
}

void deadlock_exception(char* exename, char *addr_str){
        for(int i=0; i<mlist_n; i++){
                if(deadlock_detect(mlist[i])){
			FILE *fp = 0x0;
			char line[64] = "addr2line -e";
			strcat(line,exename);
			strcat(line," ");
			strcat(line,addr_str);
			if((fp = popen(line, "r")) == NULL) {
                		return ;
        		}
			while(fgets(line, 64, fp) != NULL) {
                		printf("%s", line);
       		 	}

                        printf("*********Detect a Deadlock**********\n");
                        exit(1);
                }
        }
}

void mucheck(){
	printf("\nMutex check");

	for(int i=0; i<mlist_n; i++){
		printf("\nMutex: %ld ",mlist[i]->mutex);
		if(mlist[i]->next!=0x0){
			printf("Next : %ld",mlist[i]->next->mutex);
		}
	}
	printf("\n\n");
}
void check(){
	
	printf("\ncheck");
	for(int i=0; i<plist_n; i++){
		printf("\nID: %ld\nList ", plist[i]->thread_id) ;
		for(int j =0; j<plist[i]->list_n; j++){
			if(plist[i]->list[j] !=0x0)
				printf("%ld ",plist[i]->list[j]->mutex);
		}
	}
}
void reset(){
	exefile_name = 0x0;
	for(int i=0; i<mlist_n; i++){
		free(mlist[i]);
		mlist[i] = 0x0;

	}
	mlist_n = 0;
	for(int i=0; i<plist_n; i++){
		free(plist[i]);
		plist[i] = 0x0;
	}
	plist_n = 0;
	check();
}
void reset_or_not(char * exename){
	if(exefile_name ==0x0){
		exefile_name = (char*)malloc(64);
		strcpy(exefile_name , exename);

		printf("-----------------%s start------------\n",exefile_name);
	}	
	else if(!strcmp(exefile_name, exename)){
	
		printf("--------------on going!!! %s %s------------\n",exefile_name, exename);
	}
	else if(strcmp(exefile_name, exename)){
		printf("-----------------new program!!!------------\n");
		reset();
	}
}

void read_message(int fd, int* protocol, long* recv_pid, long* recv_m, char* addr_str  ){
	int read_size, addr;	
	read_size = sizeof(int);
	for(int i=0; i<read_size;){
		i+=read(fd,((char*)protocol)+i,read_size-i);
	}
	read_size = sizeof(long);
	for(int i=0; i<read_size;){
		i+=read(fd, ((char*)recv_pid)+i, read_size -i);
	}
	read_size = sizeof(long);
	for(int i=0; i<read_size;){
		i+=read(fd,((char*) recv_m)+i, read_size -i);
	}
	read_size = sizeof(int);
	for(int i=0; i<read_size;)
		i+=read(fd, ((char*)&addr)+i,read_size-i);

	sprintf(addr_str,"%X", addr);
}

int main( int argc, char* argv[]){
	long recv_m, recv_pid;
	int protocol, read_size= 0, fd, addr;
	char *exename = (char*)malloc(32), trace_str[64], *addr_str =(char*)malloc(32) ;
	strcpy(exename,argv[1]);
	fd = open("channel", O_RDONLY | O_SYNC) ;
	while(1){
		read_message(fd, &protocol, &recv_pid, &recv_m, addr_str);
		printf("\n\nReceive\n%d %ld %ld %s\n ",protocol, recv_pid, recv_m, argv[1]);
		if(protocol ==1){
//			printf("Exe %s  addr %s\n", argv[1], addr_str);
			graph_make(recv_pid, recv_m);
			check();
			mucheck();
			deadlock_exception(exename, addr_str );
			printf("\n");
		}
		else{
			unlock_graph(recv_pid, recv_m);
			check();
			mucheck();
//			printf("exefile name : %s\n",exefile_name);
		}	
		
	}	
	close(fd);
		
}

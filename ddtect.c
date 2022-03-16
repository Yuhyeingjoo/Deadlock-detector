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
#include<signal.h>
struct node{
        long mutex;
        struct node *next;
	struct edge *Edge;
	int checked;
	int visit;
};
struct edge{
        struct node *elist[32];
        long thread_id[32];
        long mut[32][32];
        int edge_n;
	int relative_addr[32];
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

int exit_sig =0;
char *exename =0x0 ;
char* addr_str;


void sighandler(int sig){
	exit(0);
}
struct node* new_node(long given_m){
        struct node* new_p;
        new_p = (struct node*)malloc(sizeof(struct node));
        new_p->mutex = given_m;
        new_p->visit = 0;
	new_p->checked = 0;
        new_p->Edge = (struct edge*)malloc(sizeof(struct edge));
        new_p->Edge->edge_n =0;
	for(int i=0; i<32; i++){
		for(int j=0; j<32; j++){
			new_p->Edge->mut[i][j]=0;}
	}
        return new_p;
}
struct thread* new_thread(long given_m, long given_id){
	struct thread* new_t;
	new_t = (struct thread*)malloc(sizeof(struct thread));
	new_t->thread_id=given_id;
	new_t->list_n = 0;
	return new_t;
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
struct node* last_node(struct thread* thr){
        struct node* ntmp =0x0;
        for(int i=0; i<thr->list_n; i++){
                if(thr->list[i]!=0x0)
                        ntmp = thr->list[i];
        }
        return ntmp;
}
void give_holding_m(struct thread* thr, struct edge* E){
	for(int i=0; i<thr->list_n; i++){
		if(thr->list[i]!=0x0)
			E->mut[E->edge_n][i] = thr->list[i]->mutex;
		else
			E->mut[E->edge_n][i] = 0x0;
	}
}
struct node* mlist_make(long given_id, long given_m){
	struct node* mut;
	mut = is_in_mlist(given_m);
	if(mut ==0x0){
		mut = new_node(given_m);
		mlist[mlist_n++] = mut;
	}
	return mut;
}
struct thread* plist_make(long given_id, long given_m){
	struct thread* thr_tmp1;
	thr_tmp1 =is_in_plist(given_id);
	if(thr_tmp1== 0x0){
		thr_tmp1 = new_thread(given_m, given_id);
		plist[plist_n++] = thr_tmp1;
	}
	return thr_tmp1;
}
void graph_make(long given_id, long given_m, int addr){
	struct thread* thr_tmp1;
        struct node* mut_tmp2;
	thr_tmp1 = plist_make(given_id, given_m);
	mut_tmp2 = mlist_make(given_id, given_m);
	thr_tmp1->list[thr_tmp1->list_n] = mut_tmp2;
        struct node * last_m = last_node(thr_tmp1);

        if(thr_tmp1->list_n>0 && last_m!=0x0){
		last_m->next = mut_tmp2;
                struct edge* E = last_m -> Edge;
		E->elist[E->edge_n] = mut_tmp2;
		E->thread_id[E->edge_n] = given_id;
		E->relative_addr[E->edge_n] = addr;
		give_holding_m(thr_tmp1,E);
		E->edge_n++;
        }
        thr_tmp1->list_n++;
}

void unlock_graph(long given_id, long given_m){
	struct thread* thr = is_in_plist(given_id);
	struct node* mut = is_in_mlist(given_m);
	for(int i=0; i<thr->list_n; i++){
		if(thr->list[i]!= 0x0 && thr->list[i]->mutex == given_m){
			if( i-1>=0 && thr->list[i-1]!=0x0 ){
				thr->list[i-1] -> next = thr->list[i]->next;
			}
			thr->list[i] = 0x0;
			mut->next = 0x0;
		}
	}

}
char * addr2line_ret(char * exename, char * addr_str){
	FILE *fp = 0x0;
	char *line = (char*)malloc(sizeof(char)*64);
	strcpy(line,"addr2line -e");
	strcat(line,exename);
	strcat(line," ");
	strcat(line,addr_str);
	if((fp = popen(line, "r")) == NULL) {
            return 0x0;
      	}
	while(fgets(line, 64, fp) != NULL) {
	
   	}
	return line;
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

bool is_single_cycle(struct node* Node, long cur_id){
	bool is_single_ret = false;
	if(Node->visit ==2){
		Node->visit =1;
		return true;
	}
	Node->visit = 2;
	struct edge * E = Node->Edge;
	for(int i = 0; i<E->edge_n; i++){
		if(E->elist[i]!=0x0 && E->thread_id[i] == cur_id){
			is_single_ret = is_single_cycle(E->elist[i], cur_id);
		}
	}
	
	Node->visit=1;
	return is_single_ret;
}


void deadlock_exception(char* exename, char *addr_str){
        for(int i=0; i<mlist_n; i++){
                if(deadlock_detect(mlist[i])){
                        printf("*********Detect a Deadlock**********\n");
			printf("%s\n",addr2line_ret(exename, addr_str));			
                        exit(1);
                }
        }
}

void mucheck(struct node* mlist[]){
	printf("\nMutex check");
        for(int i=0; i<mlist_n; i++){
		int e_n = mlist[i]->Edge->edge_n;
                printf("\nMutex: %ld ",mlist[i]->mutex);
                for(int j =0; j < e_n; j++){
                        if(mlist[i]->Edge->elist[j]!=0x0){
                                printf("\nNext : %ld ",mlist[i]->Edge->elist[j]->mutex);
				for(int k=0; k<32; k++){
					if(mlist[i]->Edge->mut[j][k]!=0x0)
						printf("\nHold %ld",mlist[i]->Edge->mut[j][k]);
				}
			}
                }
		printf("\n");

        }
        printf("\n\n");
}
void check(struct thread* plist[]){
	printf("\ncheck");
	for(int i=0; i<plist_n; i++){
		printf("\nID: %ld\nList ", plist[i]->thread_id) ;
		for(int j =0; j<plist[i]->list_n; j++){
			if(plist[i]->list[j] !=0x0)
				printf("%ld ",plist[i]->list[j]->mutex);
		}
	}
}

int read_message(int fd, int* protocol, long* recv_pid, long* recv_m, char* addr_str, int * addr  ){
	int read_size;	
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
		i+=read(fd, ((char*)addr)+i,read_size-i);
	sprintf(addr_str,"%X", *addr);
	return read_size;
}

int main( int argc, char* argv[]){
	signal(SIGINT, sighandler);
	long recv_m, recv_pid;
	int protocol, read_size= 0, fd, addr;
	exename = (char*)malloc(sizeof(char)*32);
	addr_str =(char*)malloc(sizeof(char)*32) ;
	strcpy(exename,argv[1]);
	fd = open("channel", O_RDONLY | O_SYNC) ;
	while(1){
		read_size = read_message(fd, &protocol, &recv_pid, &recv_m, addr_str, &addr);
		printf("\n\nReceive\n%d %ld %ld %s\n ",protocol, recv_pid, recv_m, argv[1]);
		if(protocol ==1){
			graph_make(recv_pid, recv_m, addr);
			check(plist);
			mucheck(mlist);
			deadlock_exception(exename, addr_str );
			printf("\n");
		}
		else if (protocol ==0){
			unlock_graph(recv_pid, recv_m);
			check(plist);
			mucheck(mlist);
		}
	}	
	close(fd);
		
}

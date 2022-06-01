#define _GNU_SOURCE
#include <pthread.h>
#include <assert.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAXLINESIZE 1024
#define MAXWORDSIZE 256

int message_queue_id;
key_t key;

struct Node {
  char word[MAXWORDSIZE];
  unsigned long count;
  struct Node *next;
  struct Node *prev;
};


struct List {
  struct Node *head;
  struct Node *tail;
};

struct Worker {
  struct List *list;
  char *path;
};

struct Message {
  long count;
  char word[MAXWORDSIZE];
};

struct Node *create_node(char *word, long count) {
  struct Node *node = malloc(sizeof(struct Node)); //is this unnecessary due to MAXWORDSIZE?
  if (node == NULL) {
    fprintf (stderr, "%s: Couldn't create memory for the node; %s\n", "linkedlist", strerror(errno));
    exit(-1);
  }
  memset(node->word,0,MAXWORDSIZE);
  strcpy(node->word,word);
  node->count = count;
  node->next = NULL;
  node->prev = NULL;
  return node;
}

struct List *create_list() {
  struct List *list = malloc(sizeof(struct List));
  if (list == NULL) {
    fprintf (stderr, "%s: Couldn't create memory for the list; %s\n", "linkedlist", strerror (errno));
    exit(-1);
  }
  list->head = NULL;
  list->tail = NULL;
  return list;
}

struct Worker *create_worker(struct List *list, char *path) {
  struct Worker *worker = malloc(sizeof(struct Worker));
  if (worker == NULL) {
    fprintf (stderr, "%s: Couldn't create memory for the list; %s\n", "linkedlist", strerror (errno));
    exit(-1);
  }
  worker->list = list;
  worker->path = strdup(path);
  return worker;
}

struct Message *create_message(char *word, long count){
  struct Message *message = malloc(sizeof(struct Message)); //is this unnecessary due to MAXWORDSIZE?
  if (message == NULL) {
    fprintf (stderr, "%s: Couldn't create memory for the message; %s\n", "linkedlist", strerror(errno));
    exit(-1);
  }
  memset(message->word,0,MAXWORDSIZE);
  strcpy(message->word, word);
  message->count = count;
  return message;
}

void insert_tail(struct Node *node, struct List *list) {
  if (list->head == NULL && list->tail == NULL) {
    list->head = node;
    list->tail = node;
  } else {
    list->tail->next = node;
    node->prev = list->tail;
    list->tail = node;
  }
}

void print_list(struct List *list) {
  struct Node *ptr = list->head; 
  int i = 0; 
  while (ptr != NULL && i < 70) {
    i++;
    if (ptr != list->head) {
      printf("->");
    }
    printf("(%s,%ld)", ptr->word, ptr->count);
    ptr = ptr->next;
  }
  printf("\n");
}

void destroy_list(struct List *list) {
  struct Node *ptr = list->head;
  struct Node *tmp;  
  while (ptr != NULL) {
    free(ptr->word);
    tmp = ptr;
    ptr = ptr->next;
    free(tmp);
  }
  free(list);
}

long search_list(struct List *list, char *etok){
  int pos = 0;
  struct Node *ptr = list->head;
  //struct Node *tmp;
  while(ptr != NULL) {
    if (strcmp(ptr->word,etok) != 0) {
      ptr=ptr->next;
    }
    else {
      ptr->count++;
      return 0;
    }
  }
  insert_tail(create_node(etok,1),list);
  return -1;
}

int check_list(struct List *lista,struct List *listb, int no1, int no2, int lim) {
  int tot;
  struct Node *tmp1 = lista->head;
  for(int c = 0; c < no1; c = c + 1) {
    tmp1 = tmp1->next;
  }
  if(tmp1->count < lim) {
    return -1;
  }
  else {
    struct Node *tmp2 = listb->head;
    for(int d = 0; d < no2; d = d + 1) {
      if (strcmp(tmp1->word,tmp2->word) == 0 && tmp2->count>=lim){
        tot = tmp1->count + tmp2->count;
        return tot;
      }
      else {
        tmp2 = tmp2->next;
      }
    }
    return -1;
  }
}

void word_insert(struct List *list, char *itok) {
  struct Node *tmp;
  tmp = create_node(itok, 1);
  struct Node *ptr = list->head;
  while (ptr != NULL && strcmp(ptr->word,itok) < 0) {
    ptr = ptr->next;
  }
  if (list->head == NULL) { //empty list, just put it in
    insert_tail(tmp, list);
  }
  else if (ptr == NULL) { //insert at end of list
    insert_tail(tmp, list);
  }
  else if (ptr->prev == NULL) { //insert at beginning of list
    list->head->prev = tmp;
    tmp->next = list->head;
    list->head = tmp;
  }
  else if (ptr->prev != NULL && ptr != NULL) { //insert between prev and ptr
    ptr->prev->next = tmp;
    tmp->prev = ptr->prev;
    tmp->next = ptr;
    ptr->prev = tmp;
  }
}
  
void add_out(int id, int tcount, struct List *lista, struct List *listc,int l) {
  struct Node *tmp1 = lista->head;
  struct Node *tmp2;
  //struct Node *ptr = listc->head;
  for(int c = 0; c < id; c = c + 1) {
    tmp1 = tmp1->next;
  }
  tmp2 = create_node(tmp1->word,tcount);
  insert_tail(tmp2, listc);
  /*while (ptr != NULL && ptr->count > tcount) {
    ptr = ptr->next;
  }
  if (listc->head == NULL) { //empty list, just put it in
    insert_tail(tmp2, listc);
  }
  else if (ptr->prev == NULL) { //insert at beginning of list
    listc->head->prev = tmp2;
    tmp2->next = listc->head;
    listc->head = tmp2;
  }
  else if (ptr == NULL) { //insert at end of list
    insert_tail(tmp2, listc);
  }
  else if (ptr->prev != NULL && ptr != NULL) { //insert between prev and ptr
    ptr->prev->next = tmp2;
    tmp2->prev = ptr->prev;
    tmp2->next = ptr;
    ptr->prev = tmp2;
  }*/

}

void sort_out(struct List *list,int limit){
  struct Node *curr = list->head;
  while(curr->count < limit && curr != NULL){
    //print_list(list);
    curr=curr->next;
    free(list->head);
    list->head=curr;
  }
  struct Node *comp;
  if(curr == NULL){
    return;
  }
  struct Node *temp = curr->next;
  
  //if (nout == 0 || nout ==1){
   // return;
  //}
  while(temp != NULL){
    //printf("\n");
    //print_list(list);
    //printf(temp->word);
     // printf("%s",curr->word);
     curr= temp;
     if(curr->count < limit){
      if(curr->prev != NULL){
        curr->prev->next=curr->next;
      }
      if(curr->next != NULL){
        curr->next->prev= curr->prev;
      }
      temp = curr->next;
      free(curr);
      continue;
     }
     comp= list->head;
     while (curr != comp &&(curr->count<comp->count || (curr->count == comp->count && (strcmp(curr->word,comp->word) > 0) ))){
      comp = comp->next;
     }
     temp=curr->next;
     if(curr != comp) {
      if(curr->prev != NULL){
        curr->prev->next=curr->next;
      }
      if(curr->next != NULL){
        curr->next->prev= curr->prev;
      }
      if(comp==list->head){
        curr->prev=NULL;
        curr->next=comp;
        comp->prev=curr;
        list->head=curr;
      }
      else {
        curr->next= comp;
        curr->prev=comp->prev;
        comp->prev->next=curr;
        comp->prev=curr;
     }
   }

  }
  //print_list(list);
  //printf("function");
  //return list;
}



int main(int argc, char *argv[]) {
	int limit = atoi(argv[2]);
	FILE *fileOUT = fopen(argv[1], "w+");
	struct List *list = create_list();
	long qnum=1;
	int q;
	//fprintf(fileOUT,"hello?");
	if ((key = ftok("mapper.c", 1)) == -1) {
    	perror("ftok");
    	exit(1);
  	}
  	//printf("made it here");
  	if ((message_queue_id = msgget(key, 0666)) == -1) {
   		perror("msgget");
   		exit(1);
  	}
    struct Message *message;
    struct msqid_ds buff;

    //printf("before\n");
  	while(1){
      //printf("after\n");
      message=malloc(sizeof(struct Message));
  		if (msgrcv(message_queue_id, message, MAXWORDSIZE, 0, 0) == -1) {
    		perror("msgrcv");
    		exit(1);
  		}
      if (!strcmp(message->word,"thisistheending")){
        break;
      }
      //printf("%s",message->word);
      usleep(10);
  		long exists = search_list(list, message->word);
      	//printf("message recieved\n");
  		//q = msgctl(message_queue_id, IPC_STAT, &buff);
  		//qnum=buff.msg_qnum;
  	}
    //printf("after2");
  	sort_out(list,limit);
    //print_list(list);
    //printf("main");
  	struct Node *temp= list->head;
  	while(temp != NULL){
  		fprintf(fileOUT, "%s:%ld\n",temp->word,temp->count);
  		temp = temp->next;
  	}
	fclose(fileOUT);
	//destroy_list(list);
  msgctl(message_queue_id, IPC_RMID, NULL);
	return 0;
}
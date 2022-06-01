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

sem_t empty;
sem_t full;
sem_t mutex;
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

void remove_head(struct List *list){
  struct Message *message=create_message(list->head->word,1);
  struct Node *temp=list->head;
  if(list->head->next != NULL){
    list->head=list->head->next;
  }
  else{
    list->head=NULL;
    list->tail=NULL;
  }
  //free(temp->word);
  free(temp);
  if(msgsnd(message_queue_id, message, MAXWORDSIZE, 0) == -1) {
    perror("Error in msgsnd");
  }
  //printf("%s\n", message->word);
}

void print_list(struct List *list) {
  struct Node *ptr = list->head;  
  while (ptr != NULL) {
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

int search_list(struct List *list, char *etok){
  int pos = 0;
  struct Node *ptr = list->head;
  //struct Node *tmp;
  if (ptr ==NULL) {
    return -1;
  }
  else {
    while(ptr != NULL) {
      if (strcmp(ptr->word,etok) != 0) {
        ptr = ptr->next;
        pos = pos+1;
      }
      else {
        return pos;
      }
    }
  }
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
  for(int c = 0; c < id; c = c + 1) {
    tmp1 = tmp1->next;
  }
  tmp2 = create_node(tmp1->word,tcount);
  insert_tail(tmp2, listc);
}

void sort_out(struct List *list,int nout){
  struct Node *curr = list->head;
  struct Node *comp;
  struct Node *temp = curr->next;
  if (nout == 0 || nout ==1){
    return;
  }
  while(temp != NULL){
     // printf("%s",curr->word);
     curr= temp;
     comp= list->head;
     while (curr->count<=comp->count && curr != comp){
      comp = comp->next;
     }
     temp=curr->next;
     if(curr != comp) {
      if(temp != NULL){
        curr->prev->next=curr->next;
        curr->next->prev=curr->prev;
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

}


/*int do_get() {
  int tmp = buffer[use];
  use++;
  if (use == max) {
    use = 0;
  }
  return tmp;
}*/

/*void * producer(void *arg) {
  int i;
  for (i = 0; i < items; i++) {
    sem_wait(&empty);
    sem_wait(&mutex);
    do_fill(i);
    sem_post(&mutex);
    sem_post(&full);
    //printf("Producer - Item: %d is inserted\n", i);
  }


  // end case
  for (i = 0; i < consumers; i++) {
    sem_wait(&empty);
    sem_wait(&mutex);
    do_fill(-1);
    sem_post(&mutex);
    sem_post(&full);
  }

  return NULL;
}*/
                                           
/*void * consumer(void *arg) {
  int tmp = 0;
  while (tmp != -1) {
    sem_wait(&full);
    sem_wait(&mutex);
    tmp = do_get();
    sem_post(&mutex);
    sem_post(&empty);
    //if (tmp != -1) {
      //printf("Consumer%d - Item: %d is extracted.\n", (*(int *)arg), tmp);
    //}(
  }
  return NULL;
}*/

void * senderfunc(void *arg){
  struct Worker *base=(struct Worker*)arg;
  int flag= 0;
  while(flag == 0){
    sem_wait(&full);
    sem_wait(&mutex);
    if(strcmp(base->list->head->word,"end1ng")==0){
      flag =1;
      sem_post(&mutex);
      sem_post(&empty);
      break;
    }
    remove_head(base->list);
    sem_post(&mutex);
    sem_post(&empty);
  }
  pthread_exit(0);
}

void * threadfunc(void *arg){
  FILE *tfile;
  char *tok;
  struct Worker *base=(struct Worker*)arg;
  char *tfile_name = base->path;
  tfile = fopen(tfile_name, "r");
  while (fscanf(tfile, "%ms", &tok) == 1) {
      struct Node *nxt=create_node(tok,1);
      sem_wait(&empty);
      sem_wait(&mutex);
      insert_tail(nxt,base->list);
      sem_post(&mutex);
      sem_post(&full);
      free(tok);
    }
  fclose(tfile);
  pthread_exit(0);
}

void map_threads(char *dir,int limit) {
  sem_init(&empty, 0, limit); // max are empty 
  sem_init(&full, 0, 0);    // 0 are full
  sem_init(&mutex, 0, 1);   // mutex
  DIR *direct=opendir(dir);
  struct List *threads=create_list();
  struct List *bBuff=create_list();
  struct dirent *file;
  struct Worker *base=create_worker(bBuff,"t");
  chdir(dir);
  char *point;
  pthread_t ps;
  pthread_create(&ps,NULL,senderfunc,(void*)base);
  while((file = readdir(direct)) != NULL) {
    if((point = strrchr(file->d_name,'.')) != NULL ) {
      if(strcmp(point,".txt") == 0) {
        //create mapper thread
        char *buff=malloc(1024);
        sprintf(buff,"%s/%s",dir,file->d_name);
        struct Worker *base=create_worker(bBuff,buff);
        unsigned long temp;
        pthread_create(&temp,NULL,threadfunc,(void*)base); //where to join?
        struct Node *tnode=create_node("temp",temp);
        insert_tail(tnode,threads);
      }
    }
  }
  for(struct Node *t=threads->head;t!=NULL;t=t->next){
    pthread_join(t->count,NULL);
  }
  struct Node *end=create_node("end1ng",1);
  sem_wait(&empty);
  sem_wait(&mutex);
  insert_tail(end,bBuff);
  sem_post(&mutex);
  sem_post(&full);
  pthread_join(ps,NULL);
}

int main(int argc, char *argv[]) {
  FILE *fileIN;
  if ((key = ftok("mapper.c", 1)) == -1) {
    perror("ftok");
    exit(1);
  }
  if ((message_queue_id = msgget(key, 0666 | IPC_CREAT)) == -1) {
    perror("msgget");
    exit(1);
  }
  int limit = atoi(argv[2]);
  char directoryPath[MAXLINESIZE];
  fileIN = fopen(argv[1], "r");
  int x;
  int count = 0;
  while(fscanf(fileIN, "map %s\n", directoryPath)!= EOF){
    x=fork();
    if(x==0){
      map_threads(directoryPath,limit);
      exit(0);
    }
    else
      count++;
    }
  for(int i = 0; i < count; i++){
    wait(NULL); //will a process finish before this loop is reached?
  }
  fclose(fileIN);
  struct Message *message = create_message("thisistheending",1);
  if(msgsnd(message_queue_id, message, MAXWORDSIZE, 0) == -1) {
    perror("Error in msgsnd");
  }
  return 0;
}
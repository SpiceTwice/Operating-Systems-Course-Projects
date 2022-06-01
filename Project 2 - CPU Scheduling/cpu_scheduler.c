#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct Process {
  int pid;
  int atime;
  int btime;
  int ftime;
  int wtime;
  int ptime;
  struct Process *next;
  struct Process *prev;
};

struct Order {
  struct Process *head;
  struct Process *tail;
};

struct Process *create_Process (int i1, int i2, int i3) {
  struct Process *proc = malloc(sizeof(struct Process));
  if (proc == NULL) {
    fprintf (stderr, "Couldn't create memory for the %s %s\n", "process", strerror(errno));
    exit(-1);
  }
  proc->pid = i1;
  proc->atime = i2;
  proc->btime = i3;
  proc->ftime = 0;
  proc->wtime = 0;
  proc->ptime= 0;
  proc->next = NULL;
  proc->prev = NULL;
  return proc;
}

struct Order *create_Order() {
  struct Order *order = malloc(sizeof(struct Order));
  if (order == NULL) {
    fprintf (stderr, "%s: Couldn't create memory for the Order; %s\n", "linkedlist", strerror (errno));
    exit(-1);
  }
  order->head = NULL;
  order->tail = NULL;
  return order;
}

void insert_tail(struct Process *proc, struct Order *order) {
  if (order->head == NULL && order->tail == NULL) {
    order->head = proc;
    order->tail = proc;
  } else {
    order->tail->next = proc;
    proc->prev = order->tail;
    order->tail = proc;
  }
}

void sjf(struct Order *order, int tot) {
  int ctime = 0;
  int t;
  int f =0;
  int c=0;
  int reup;
  /*int twait=0;
  int ttemp;
  int tturn=0;
  double await;
  double aturn;*/
  struct Process *curr = order->head;
  struct Process *temp;
  struct Process *comp;
  struct Process *start;
  struct Process *fin = order->head;
  t = curr->btime;
  ctime = ctime + curr->btime;
  curr->btime = 0;
  curr->ftime = ctime;
  curr->wtime = ctime-t;
  c=c+1;
  while(c != tot){
    start = order->head;
    for(reup = 0; reup<c-1; ++reup){
      start = start->next;
    }
    while(fin->atime <= ctime && fin->next != NULL){
      fin=fin->next;
    }
    curr = start->next->next;
    while(curr != NULL && curr->atime<=ctime && f != 1){
      comp = curr->prev;
      while(curr->btime<comp->btime && comp != start){
       comp = comp->prev;
      }
      if(curr->next != NULL){
        curr->next->prev = curr->prev;
        curr->prev->next = curr->next;
        temp = curr->next;
        curr->prev = comp;
        curr->next = comp->next;
        comp->next = curr;
        curr->next->prev = curr;
        curr=temp;
      }
      else{
        curr->prev->next = NULL;
        curr->prev = comp;
        curr->next = comp->next;
        comp->next = curr;
        if(curr->next != NULL){
          curr->next->prev = curr;
        }
        else{
          f=1;
        }
      }
    }
    curr=start->next;
    t = curr->btime;
    ctime = ctime + curr->btime;
    curr->btime = 0;
    curr->ftime = ctime;
    curr->wtime = ctime-t-curr->atime;
    /*twait=twait+curr->wtime;
    ttemp=ctime-curr->atime;
    tturn=tturn+ttemp;*/
    c=c+1;
  }
  /*await=(double)twait / (double)c;
  aturn=(double)tturn / (double)c;
  printf("wait %f turn %f %d %d %d\n", await,aturn,c,twait,tturn);*/
}

void srtf(struct Order *order, int tot,FILE *fileOUT){
  int ctime = 0;
  int t;
  int c=0;
  int stopb;
  /*int twait=0;
  int ttemp;
  int tturn=0;
  double await;
  double aturn;*/
  struct Process *curr = order->head;
  struct Process *temp=curr->next;
  struct Process *temp2;
  //struct Process *comp;
  struct Process *burr;
  struct Process *fin;
  while(c != tot){
    if (curr==NULL)
      break;
    if(temp != NULL && curr->btime>temp->atime-ctime){
      stopb = temp->atime;
      curr->btime=curr->btime-(stopb-ctime);
      curr->ptime=curr->ptime+stopb-ctime;
      ctime = stopb;
      if(curr->btime>temp->btime){
        burr=temp;
        temp = burr->next;
        burr->next->prev=burr->prev;
        burr->prev->next=burr->next;
        burr->next=curr;
        burr->prev=curr->prev;
        curr->prev=burr;
        curr=burr;
      }
      /*else if(curr->btime==temp->btime){
        temp2=temp;
        temp=temp2->next;
        temp2->next->prev=temp2->prev;
        temp2->prev->next=temp2->next;
        temp2->next=curr->next;
        temp2->prev=curr;
        curr->next->prev=temp2;
        curr->next=temp2;
      }*/
      else if(curr->btime<=temp->btime){

          fin = temp->next;
          temp2 = temp;
          temp=temp->prev;
          while (temp2->btime < temp->btime)
            temp = temp->prev;
          if(temp2->next != NULL)
            temp2->next->prev=temp2->prev;
          temp2->prev->next=temp2->next;
          temp2->next=temp->next;
          temp2->prev=temp;
          temp->next=temp2;
          if(temp2->next != NULL)
            temp2->next->prev=temp2;
          temp = fin;

          /*if(temp->prev != curr){
            burr=curr->next->next;
            while(burr != fin){
              comp = burr->prev;
              while(burr->btime < comp->btime && comp != curr){
                comp = comp->prev;
              }
              burr->next->prev=burr->prev;
              burr->prev->next=burr->next;
              temp2 = burr->next;
              burr->prev = comp;
              burr->next = comp->next;
              comp->next = burr;
              burr->next->prev = burr;
              burr = temp2;
            }
            temp = fin;
          }
          else{
            temp = fin;
          }
        }
        else{
          comp=temp->prev;
          while(temp->btime<comp->btime && comp != curr){
            comp=comp->prev;
          }
          temp->prev->next=NULL;
          temp->next=comp->next;
          temp->prev=comp;
          comp->next= temp;
          if(comp->next != NULL){
            comp->next->prev=temp;
          }
          while(curr != NULL){
            t = curr->btime;
            curr->btime = 0;
            ctime=ctime + t;
            curr->ftime=ctime;
            curr->ptime=curr->ptime+t;
            curr->wtime=ctime-curr->ptime;
            c=c+1;
            curr=curr->next;
          }
        }
      }*/
    }
  }
    else  {
      t = curr->btime;
      curr->btime = 0;
      ctime=ctime + t;
      curr->ftime=ctime;
      curr->ptime=curr->ptime+t;
      curr->wtime=ctime-curr->atime-curr->ptime;
      /*twait=twait+curr->wtime;
      ttemp=ctime-curr->atime;
      tturn=tturn+ttemp;*/
      c=c+1;
      fprintf(fileOUT, "%d %d %d %d\n",curr->pid,curr->atime,curr->ftime,curr->wtime);
      curr=curr->next;
    }
  }
  /*await=(double)twait / (double)c;
  aturn=(double)tturn / (double)c;
  printf("wait %f turn %f %d %d %d\n", await,aturn,c,twait,tturn);*/
}








void print_Out(FILE *fileOUT,struct Order *order, int tot){
  struct Process *tmp = order->head;
  while(tmp != NULL) {
    fprintf(fileOUT, "%d %d %d %d\n",tmp->pid,tmp->atime,tmp->ftime,tmp->wtime);
    tmp=tmp->next;
  }
}

void destroy_Order(struct Order *order) {
  struct Process *ptr = order->head;
  struct Process *tmp;  
  while (ptr != NULL) {
    tmp = ptr;
    ptr = ptr->next;
    free(tmp);
  }
  free(order);
}

int main(int argc, char *argv[]) {
  FILE *fileIN;
  FILE *fileOUT;
  struct Order *order = create_Order();
  struct Process *proc;
  int limit;
  int i1;
  int i2;
  int i3;
  int l;
  int tot=0;


  if (argc != 5 && argc != 4) {
    printf("\nImproper number of inputs: %d\n",argc);
    return 0;
  }

  if (argc == 5) {
    int limit = atoi(argv[4]);
    fileIN = fopen(argv[1], "r");
    for(l = 0; l<limit; ++l) {
      fscanf(fileIN, "%d %d %d", &i1, &i2, &i3);
      proc = create_Process(i1,i2,i3);
      insert_tail(proc,order);
      tot = tot + 1;
    }
    fclose(fileIN);
  }
  else{
    fileIN = fopen(argv[1], "r");
    limit = 0;
    char co;
    for (co = getc(fileIN); co != EOF; co = getc(fileIN)){
      if (co == '\n'){
        limit = limit + 1;
      }
    }
    fclose(fileIN);
    fileIN =fopen(argv[1], "r");
    for(l = 0; l<limit; ++l) {
      fscanf(fileIN, "%d %d %d", &i1, &i2, &i3);
      proc = create_Process(i1,i2,i3);
      insert_tail(proc,order);
      tot = tot + 1;
    }
      fclose(fileIN);
  }

  if(strcmp(argv[3],"SJF")==0) {
    sjf(order,tot);
    fileOUT = fopen(argv[2], "w+");
    print_Out(fileOUT,order,tot);
    fclose(fileOUT);
  }
  else if(strcmp(argv[3],"SRTF")==0){
    fileOUT = fopen(argv[2],"w+");
    srtf(order,tot,fileOUT);
    fclose(fileOUT);
  }
    /*fileOUT = fopen(argv[2], "w+");
    print_Out(fileOUT,order,tot);
    fclose(fileOUT);
  }
  else {
  }*/

  destroy_Order(order);

  return 0;
}

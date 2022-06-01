#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


struct Node {
  char *word;
  int count;
  struct Node *next;
  struct Node *prev;
};

struct List {
  struct Node *head;
  struct Node *tail;
};

struct Node *create_node(char *word, int count) {
  struct Node *node = malloc(sizeof(struct Node));
  if (node == NULL) {
    fprintf (stderr, "%s: Couldn't create memory for the node; %s\n", "linkedlist", strerror(errno));
    exit(-1);
  }
  node->word = strdup(word);
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
  while (ptr != NULL) {
    if (ptr != list->head) {
      printf("->");
    }
    printf("(%s,%d)", ptr->word, ptr->count);
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

int main(int argc, char *argv[]) {
  struct List *list1 = create_list();
  struct List *list2 = create_list();
  struct List *list3 = create_list();
  struct Node *tmp = NULL;
  FILE *file1;
  FILE *file2;
  FILE *file3;

  //const char *del = " ";
  int limit = atoi(argv[3]);
  //char buff[64];
  char *tok;
  int exists;
  int f1count = 0;
  int f2count = 0;
  int f3count = 0;
  int meetsl;

  if (argc != 5) {
    printf("\nImproper number of inputs: %d\n",argc);
    return 0;
  }

  file1 = fopen(argv[1], "r");
  while (fscanf(file1, "%ms", &tok) == 1) {
      exists = search_list(list1, tok);
      if (exists == -1) {
        word_insert(list1, tok);
        f1count = f1count + 1;
      }
      else {
        struct Node *ptr = list1->head;
        for(int i = 0; i < exists; i = i + 1) {
          ptr = ptr->next;
        }
        ptr->count= ptr->count + 1;
      }
      free(tok);
    }
fclose(file1);

  file2 = fopen(argv[2], "r");
  while (fscanf(file2, "%ms", &tok) == 1) {
      exists = search_list(list2, tok);
      if (exists == -1) {
        word_insert(list2, tok);
        f2count = f2count + 1;
      }
      else {
        struct Node *ptr = list2->head;
        for(int i = 0; i < exists; i = i + 1) {
          ptr = ptr->next;
        }
        ptr->count= ptr->count + 1;
      }
      free(tok);
    }
  fclose(file2);

  file3 = fopen(argv[4], "w+");

  for (int b = 0; b < f1count; b = b + 1) {
    meetsl = check_list(list1,list2,b,f2count,limit);
    if (meetsl != -1) {
      add_out(b, meetsl, list1, list3, limit);
      f3count = f3count + 1;
    }
  }

  sort_out(list3,f3count);
tmp = list3->head;
for (int e = 0; e < f3count; e = e + 1) {
  fprintf(file3, "%s,%d\n",tmp->word,tmp->count);
  tmp = tmp->next;
}
fclose(file3);


  destroy_list(list1);
  destroy_list(list2);
  destroy_list(list3);

  return 0;
}

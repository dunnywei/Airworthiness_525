#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define M 10
#define N 20
#define BUFFER_SIZE 20
typedef struct node {
	struct node *next;
	char *data;
	int length;
} node_t;
node_t head=NULL,
node_t tail=NULL;
sem_t data_count;

pthread_mutex_t lock_1=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_2=PTHREAD_MUTEX_INITIALIZER;

int get_external_data(char *buffer, int bufferSizeInBytes);
void process_data(char *buffer, int bufferSizeInBytes);

void process_data(char *buffer, int bufferSizeInBytes)
{
	int i=0;
	if(buffer!=NULL)
	{
		printf("@process_data with thread %li -",pthread_self());
		
		while(i<bufferSizeInBytes)
		{
			printf("i is %d and buffer[i] is %c \n",i,buffer[i]);
			i++;  		
		}
		memset(buffer,0,bufferSizeInBytes);
	}else{
		printf("@process_data, error occurs for buffer==NULL");
	}
	return;
}
int get_external_data(char *buffer, int bufferSizeInBytes)
{
	int value=-1;
	char temp_char[]="abcdefghijlmnopqrstu";
	value=strlen(temp_char)+1;
	memcpy(buffer,temp_char,value);
	
	printf("@get_external_data, buffer is %s \n",buffer);
	
	return value;
}

void *reader_thread(void *arg)
{
  while(1)
  {
	  node_t *node_remove;
   
      int length=0;
	  if(sem_wait(&data_count)==1)
		   return NULL;
	   pthread_mutex_lock(&lock_1);
	   if(head!=NULL)
	   {
		   node_remove=head;
		   head=head->next;
	   }
       pthread_mutex_unlock(&lock_1);
	   
	   pthread_mutex_lock(&lock_2);
	   process_data(node_remove->data,node_remove->length);
       pthread_mutex_unlock(&lock_2);

       free(node_remove->data);
	   free(node_remove);

  }
  return NULL;
}
void *writer_thread(void *arg)
{
	int length;
	char *buffer;
	node_t *new_node;
	
	new_node-(node_t*)malloc(sizeof(node_t));
	
   while(1)
   {
	   
   }
  return NULL;
}



int main(int argc, char **argv)
{
  int i,j;
  for(i=0;i<N;i++)
  {
     pthread_create(NULL,NULL,reader_thread);
  }
  
  for(j=0;j<M;j++)
  {
     pthread_create(NULL,NULL,reader_thread);
  }
}
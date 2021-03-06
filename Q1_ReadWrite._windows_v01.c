
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

#define M 10
#define N 20
#define BUFFER_SIZE 20
typedef struct node {
	struct node *next;
	char *data;
	int length;
} node_t;
node_t *head=NULL;
node_t *tail=NULL;
sem_t data_count;

pthread_mutex_t lock_1=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_2=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_3=PTHREAD_MUTEX_INITIALIZER;


int get_external_data(char *buffer, int bufferSizeInBytes);
void process_data(char *buffer, int bufferSizeInBytes);

void process_data(char *buffer, int bufferSizeInBytes)
{
	int i=0;
	if(buffer!=NULL)
	{
		printf("@process_data with thread %li - \n",pthread_self());
		
		while(i<bufferSizeInBytes)
		{
			printf("i is %d and buffer[i] is %c \n",i,buffer[i]);
			i++;  		
		}
		memset(buffer,0,bufferSizeInBytes);
	}else{
		printf("@process_data, error occurs for buffer==NULL \n");
	}
	return;
}
int get_external_data(char *buffer, int bufferSizeInBytes)
{
	
	int value=-1;
	int fd;
	#if 1
	   char temp_char[]="abcdefghijlmnopqrstu";
	   value=strlen(temp_char)+1;
	   memcpy(buffer,temp_char,value);
	
	   printf("@get_external_data, buffer is %s \n",buffer);
	
	   return value;
	#else
		/*char temp_char[4];*/
        char temp_char[20];
		fd=open("/dev/xyz",O_RDWR);
		if(fd<0){
			printf("can't open \n");
			return -1;
		}
		printf("Open ok \n");

		if(read(fd,temp_char,20)<0){
		    printf("reading error \n");
			return -1;
		}
		printf("Read ok \n");
	   value=strlen(temp_char)+1;

       memcpy(buffer,temp_char,value);
	
	   printf("@get_external_data, buffer is %s \n",buffer);
	   close(fd);
	   return value;
	
	#endif
	
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
	
	
   while(1)
   {
	   buffer=(char *)malloc(sizeof(buffer)*BUFFER_SIZE);
	   new_node=(node_t*)malloc(sizeof(node_t));
	   pthread_mutex_lock(&lock_3);

	   length=get_external_data(buffer,BUFFER_SIZE);
	   if(length<0)
	   {
	       continue;
	   }
	   pthread_mutex_unlock(&lock_3);

	   new_node->next=NULL;
	   new_node->length=length;
	   new_node->data=buffer;
	   
	   pthread_mutex_lock(&lock_1);
	   
	   if(head==NULL)
	   {
		   head=new_node;
		   tail=new_node;
	   }else{
		
		tail->next=new_node;
	        tail=tail->next;
	   }
	   
	   pthread_mutex_unlock(&lock_1);
	   
           pthread_mutex_lock(&lock_2);
	   
	   printf("@writer_thread, thread %ld write with buffer %s \n", pthread_self(), buffer);
	    pthread_mutex_unlock(&lock_2);
   
   }
  return NULL;
}



int main(int argc, char **argv)
{
  int i,j;
  int sem_count_initial=sem_init(&data_count,0,0);
  pthread_t temp_t;
  for(i=0;i<N;i++)
  {
     pthread_create(&temp_t,NULL,reader_thread,NULL);
  }
  
  for(j=0;j<M;j++)
  {
     pthread_create(&temp_t,NULL,writer_thread,NULL);
  }
}

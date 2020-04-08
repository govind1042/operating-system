#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#define AVAILABLE 30001
#define BUSY 30002
#define BUFF 10
#define MAX 30000


void kill_All_Of_Childs(int child_id[]);
int is_Prime(int n);
void print_Array(int prime_Array[]);
int write_To_Pipe(int pipe_fd,int info);
int read_From_PipeBlocking(int pipe_fd,int *info);
int read_From_Pipe(int pipe_fd,int *info);
int add_To_The_Prime_List(int prime_Array[],int number);
int detecting_the_primes(int End_Of_Read,int End_Of_Write);
int dealing_with_childs(int End_Of_Read[],int End_Of_Write[],int prime_Array[]);

int numprimes = 0;
int N,K;


int is_Prime(int m)
{
	int i;
	int m_sqaure = m * m;
	for(i = 2; i*i <= m; i++){
		if(m%i == 0)
			return 0;
	}
	return 1;
}



void print_Array(int prime_Array[])
{	
	int j;
	for (j = 0; j < 2*K; ++j)
	{
		printf("%d ",prime_Array[j]);
		/* code */
	}
	printf("\n");
}

int write_To_Pipe(int pipe_fd,int info)
{
	char buf[BUFF];
	sprintf(buf,"%d",info);
	int bytes = write(pipe_fd,buf,BUFF);
	if(bytes == -1){
		perror("There is an error to write to the pipe");
		exit(1);
	}
	return bytes;
}

int read_From_PipeBlocking(int pipe_fd,int *info)
{
	char buf[BUFF];
	int bytes = read(pipe_fd,buf,BUFF);
	if(bytes == -1){
		perror("There is an error to read from the pipe");
		exit(1);
	}
	sscanf(buf,"%d",info);
	return bytes;
}

int read_From_Pipe(int pipe_fd,int *data)
{
	char buf[BUFF];
	int bytes = read(pipe_fd,buf,BUFF);
	if(bytes == -1){
		return bytes;
	}
	sscanf(buf,"%d",data);
	return bytes;
}

int add_To_The_Prime_List(int prime_Array[],int number)
{
	int i;
	for(i=0;i<numprimes;i++)
	{
		if(prime_Array[i] == number)
		{
			return numprimes;
		}
	}
	prime_Array[numprimes++] = number;
	return numprimes;
}

void kill_All_Of_Childs(int child_id[])
{
	int j;
	for(j=0;j<K;j++)
	{
		kill(child_id[j],SIGTERM);
	}
}


int detecting_the_primes(int End_Of_Read,int End_Of_Write)
{
	int numbers[K];
	int j;
	while(1)
	{
		write_To_Pipe(End_Of_Write,AVAILABLE);

		for (j = 0; j < K; ++j)
		{
			read_From_Pipe(End_Of_Read,&numbers[j]);
		}
		write_To_Pipe(End_Of_Write,BUSY);
		for (j = 0; j < K; ++j)
		{
			if(is_Prime(numbers[j]))
			{
				write_To_Pipe(End_Of_Write,numbers[j]);
			}
		}
	}
}



int dealing_with_childs(int End_Of_Read[],int End_Of_Write[],int prime_Array[])
{
	int m,j,flag=0;
	while(1)
	{
		for(m = 0; m < K; m++)
		{
			int num;
			int res = read_From_Pipe(End_Of_Read[m],&num);
			if(res == -1 || num == BUSY)
				continue;
			if(num != AVAILABLE)
			{
				add_To_The_Prime_List(prime_Array,num);
				if(numprimes >= 2*K)
				{
					flag = 1;
					break;
				}
			}
			else
			{
				for (j = 0; j < K; ++j)
				{
					int rand_no = rand()%MAX;
					write_To_Pipe(End_Of_Write[m],rand_no);
				}
				
			}
		}
		if(flag)
			break;
	}
	print_Array(prime_Array);
	return 0;
}


int main()
{
	printf("Please enter the ammount of processes you want to create:");
	scanf("%d",&K);
	N  = 2*K;
	int prime_Array[N];
	int numprimes = 0;
	int j,c_id;
	int child_id[K];
	int End_Of_Read[K];
	int End_Of_Write[K];

	for(j=0;j<K;j++)
	{
		int parent_to_child[2];
		int child_to_parent[2];
		pipe(parent_to_child);
		pipe(child_to_parent);
		c_id = fork();
		if(c_id==0)
		{
			detecting_the_primes(parent_to_child[0],child_to_parent[1]);  //Child's process 
		}
		else
		{
			child_id[j] = c_id;
			End_Of_Read[j] = child_to_parent[0];
			End_Of_Write[j] = parent_to_child[1];
			//deal_with_child(parent_to_child[1],child_to_parent[0]);
		}
	}
	sleep(2);
	dealing_with_childs(End_Of_Read,End_Of_Write,prime_Array);
	kill_All_Of_Childs(child_id);
	return 0;

}

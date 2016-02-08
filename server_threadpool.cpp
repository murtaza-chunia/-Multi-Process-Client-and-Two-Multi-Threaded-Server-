#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<iostream>
#include<string>
#include<stdio.h>
#include<cstring>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
//#include<string.h>
#include<vector>
#include<signal.h>

using namespace std;

//--------------------------------------GLOBAL VARIABLES-------------------------------------
struct fileWriteArgs{
    int childNumber;
    int sockfd;
    pthread_t *th;
};

pthread_t *thread_pointer;

pthread_mutex_t cerr_lock;
pthread_mutex_t data_lock;
pthread_mutex_t thread_remove_lock;
pthread_mutex_t finished_thread_lock;

//vector<int> finished_thread;
vector<pthread_t*> available_pool;
vector<pthread_t*>::iterator it;

const int parent_pid = getpid();
static int check = -1; // to check if the process is freed it doesn get freed again wen we do ctrl^c

//-----------------------------------------------------------------------------------------------

void thread(int signum);
void* fileWrite(void *fd);

int main(int argc, char *argv[])
{
//cout <<"hi"<< endl ;
signal(SIGINT,thread);
int sockfd,newsockfd,portno,n;
socklen_t clilen;
char buffer[255];
struct sockaddr_in serv_addr,cli_addr;


	if(argc < 2)
	{
		cerr << "No port provided" << endl;
		exit(1);
	}

sockfd = socket(AF_INET,SOCK_STREAM,0);

	if(sockfd < 0)
	{
		cerr << "Error opening socket"<<endl;
		exit(1);
	}

bzero((char*)&serv_addr,sizeof(serv_addr));
portno = atoi(argv[1]);

serv_addr.sin_family = AF_INET;
serv_addr.sin_port = htons(portno);
serv_addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
	{
		cerr << "Error on Binding" << endl;
		exit(1);
	}

int number_threads = atoi(argv[2]);
pthread_attr_t attr;
pthread_attr_init(&attr);
thread_pointer = new pthread_t[number_threads];

for(unsigned int i=0;i<number_threads;i++)
	{available_pool.push_back(&thread_pointer[i]);
	cout << "Address of thread " << available_pool[available_pool.size() - 1]<<endl;}
listen(sockfd,100);
clilen= sizeof(cli_addr);

cout << "Number of threads "<< available_pool.size()<<endl;

int noOfChilds = 0;


 while(1){
	while(available_pool.size() == 0); //blocking call unless threads are available.........
	newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
	//cout << newsockfd << endl ;
	pthread_mutex_lock(&data_lock); //data lock
        noOfChilds++; //childNumber

       	struct fileWriteArgs args;
        args.sockfd = newsockfd;  // socket-id of the client stored in the global stuct variable..
        args.childNumber = noOfChilds;
	args.th = (pthread_t *)available_pool[available_pool.size() - 1];
	pthread_t *newThread = (pthread_t *)&available_pool[available_pool.size() - 1];
	available_pool.pop_back();
		if(newsockfd < 0)
		{
			cerr << "Error on Accept" << endl;
			exit(1);
		}

	
	int x = pthread_create((pthread_t *)&newThread[0], &attr, &fileWrite, &args);

    }

   // pthread_attr_destroy(&attr);

return 0;
}


void* fileWrite(void *fd)
{
int n,size;
char buffer[10];
char *data;
struct fileWriteArgs *p = (struct fileWriteArgs*)fd;
int sockfd = p->sockfd;
int childNumber = p->childNumber;
pthread_t *th = p->th;
pthread_mutex_unlock(&data_lock); // data lock released....
bzero(buffer,10);
n = read(sockfd,buffer,10);
	if(n<0)
	{
		pthread_mutex_lock(&cerr_lock);
		cerr << "Error on reading in Thread : "<< childNumber << endl;
		pthread_mutex_unlock(&cerr_lock);
		close(sockfd);

		pthread_mutex_lock(&finished_thread_lock);
		it = available_pool.begin();
		available_pool.insert(it,th);
		pthread_mutex_unlock(&finished_thread_lock);

		pthread_exit(NULL);
	}

size = atoi(buffer);

cout << "Size for Thread " << childNumber << " is " << size << endl; 
bzero(buffer,10);
buffer[0] = 'o';
buffer[1] = 'K';
buffer[2] = '\0';
n = write(sockfd,buffer,10);

data = new char[size];
n = read(sockfd,data,size);
if(n<0)
	{
		pthread_mutex_lock(&cerr_lock);
		cerr << "Error on reading DATA in Thread : " << childNumber << endl;
		pthread_mutex_unlock(&cerr_lock);

		delete []data;
		close(sockfd);

		pthread_mutex_lock(&finished_thread_lock);
		it = available_pool.begin();
		available_pool.insert(it,th);
		pthread_mutex_unlock(&finished_thread_lock);

		pthread_exit(NULL);
	}


for (unsigned int i=0; i < size; i++)
        data[i] = toupper(data[i]);

	
n = write(sockfd,data,size);
	if(n<0)
	{
		pthread_mutex_lock(&cerr_lock);
		cerr << "Error on writing DATA in Thread : "<< childNumber << endl;
		pthread_mutex_unlock(&cerr_lock);

		delete []data;
		close(sockfd);

		pthread_mutex_lock(&finished_thread_lock);
		it = available_pool.begin();
		available_pool.insert(it,th);
		pthread_mutex_unlock(&finished_thread_lock);

		pthread_exit(NULL);
	}

delete []data;

pthread_mutex_lock(&finished_thread_lock); // lock to put childNumber in finished_thread vector...
it = available_pool.begin();
available_pool.insert(it,th);// putting the childNumber in the finished_thread vector so that parent can free the memory...
pthread_mutex_unlock(&finished_thread_lock);

close(sockfd);
}


void thread(int signum)
{
signal(SIGINT,SIG_IGN);
delete []thread_pointer;
exit(0);
}

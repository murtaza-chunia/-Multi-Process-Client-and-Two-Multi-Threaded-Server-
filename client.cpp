#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <iostream>
#include<fstream>
#include<sys/wait.h>
#include<pthread.h>
#include <time.h>

pthread_mutex_t connection_lock;
pthread_mutex_t cerr_lock;
timespec diff(timespec start, timespec end);

using namespace std;

int main(int argc, char * argv[])
{
int status = 0;
int wpid;

for(unsigned j=0;j<argc-3;j++)
	{
	cout << argv[j+3] << endl;
	}
	int portno,n,i,pid;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[256];
	if(argc<3)
	{
		cerr<<"Error";
		exit(0);
	}
	pid = getpid();
	portno = atoi(argv[2]);

	server = gethostbyname(argv[1]);
		if(server== NULL)
		{
			cerr<<"Hostdoes not exist";
		}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = PF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);
	serv_addr.sin_port=htons(portno);	

	
	for(i=0;i<argc-3;i++)
	{
	int pid_temp = getpid();
	if(pid_temp == pid)
	{
		int pidch = fork();
		if(pidch == 0)
		{	
			timespec time1, time2;
			clock_gettime(CLOCK_REALTIME, &time1);
			char *buffer;
			char len[5];
			pthread_mutex_lock(&connection_lock);

			int socketDescriptor=socket(PF_INET,SOCK_STREAM,0);
			if(socketDescriptor<0)
			{
				pthread_mutex_lock(&cerr_lock);
				cerr<<"Error Opening the socket for client : " << i + 1 << endl;
				pthread_mutex_unlock(&cerr_lock);
				exit(1);
			}
			
			pthread_mutex_unlock(&connection_lock);

			int connection= connect(socketDescriptor,(struct sockaddr *) &serv_addr, sizeof(serv_addr));
			

			if(connection<0)
			{
				pthread_mutex_lock(&cerr_lock);
				cerr<<"Error Connecting : " << i + 1 << endl;
				pthread_mutex_unlock(&cerr_lock);
				exit(1);
			}
			
			

			bzero(len,5);
			ifstream infile (argv[i+3]);
			infile.seekg (0,infile.end);
			long int size = infile.tellg();
			infile.seekg (0);
			
			sprintf(len,"%ld",size);
			int n= write(socketDescriptor,len,5);

			if(n<0)
			{
				pthread_mutex_lock(&cerr_lock);
				cerr<<"Error writing SIZE to SOCKET of client : " << i + 1 << endl;
				pthread_mutex_unlock(&cerr_lock);
				close(socketDescriptor);
				pthread_mutex_unlock(&cerr_lock);
				exit(1);
			}

			bzero(len,5);
			n = read(socketDescriptor,len,5);

			if(n<0)
			{
				pthread_mutex_lock(&cerr_lock);
				cerr << "Error on reading the SIZE_WRITE status of client : " << i + 1 << endl;
				pthread_mutex_unlock(&cerr_lock);
				close(socketDescriptor);
				exit(1);
			}
			pthread_mutex_lock(&cerr_lock);
			//cout << "SIZE reached successfully for client : " << i + 1 << " which is "<< len << endl;
			pthread_mutex_unlock(&cerr_lock);	
					
			buffer = new char[size];
			infile.read(buffer,size);
			n= write(socketDescriptor,buffer,size);

			if(n<0)
			{
				pthread_mutex_lock(&cerr_lock);
				cerr<<"Error writing DATA to SOCKET of client : " << i + 1 << endl;
				pthread_mutex_unlock(&cerr_lock);
				close(socketDescriptor);
				delete []buffer;
				exit(1);
			}

			bzero(buffer,size);
			n= read(socketDescriptor,buffer,size);

			if(n<0)
			{
				pthread_mutex_lock(&cerr_lock);
				cerr<<"Error reading DATA from  SOCKET of client : " << i + 1<< endl;
				pthread_mutex_unlock(&cerr_lock);
				close(socketDescriptor);
				delete []buffer;
				exit(1);
			}
			
			infile.close();
			ofstream myfile (argv[i+3]);
			myfile.write (buffer,size);
			clock_gettime(CLOCK_REALTIME, &time2);

			
			double t1 = (double)diff(time1,time2).tv_sec;
			double t2 = diff(time1,time2).tv_nsec*0.0000000001;
		
			cout<<"Time taken for client "<< i + 1<< " is " << t1 + t2<<endl;
			

			myfile.close();
			
			
			delete []buffer;
			close(socketDescriptor);
		}
	}	
	}

for(i=0;i<argc-3;i++)
wpid = wait(&status);

	return 0;
}


timespec diff(timespec start, timespec end)
{
	timespec temp;
	if ((end.tv_nsec - start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec - start.tv_sec - 1;
		temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
	} 
	else {
		temp.tv_sec = end.tv_sec - start.tv_sec;
		temp.tv_nsec =  (end.tv_nsec - start.tv_nsec);
	}
	return temp;
}

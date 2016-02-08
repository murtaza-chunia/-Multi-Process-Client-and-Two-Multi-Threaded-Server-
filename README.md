# -Multi-Process-Client-and-Two-Multi-Threaded-Server-
The motivation behind this project is to understand the impact of multi-threading on server(s) side to serve multiple client request.

The basic functionality of this program is to convert single/multiple lower case text files into upper case file(s).

Client Side :

Depending on the number of file arguments given on the command line to the client program, parent process creates that many individual client processes. At the beginning of the fork we get the current value of time and when the process is completed we get the instantaneous value of the time. This difference in time between the end & start time values gives the turnaround time for that particular client process. In the end we calculate the average turnaround time for a particular set of client process for different number od threads serving the client process on the server side.

Two server model were created to understand the impact of multi-threading.

One to one server model :

Here one thread is created for every incoming client request. Threads are created dynamically at run time & destroyed after serving the corresponding client process. Each new thread created is stored in a vector. After servicing the client, thread generates an interrupt which is handled by the parent process and it stores the thread ID in the finished vector. Interrupt Handler on the other hand deletes the first available thread based on the thread ID in the finished vector and returns.

Thread Pool – Many to Many Server model :

When the server process is started we explicitly mention from the command line the number of threads we want to create. Hence we create a thread pool in the system. If all the threads are serving client processes, then the remaining client processes have to wait in the queue for a thread to be released by the client process. This is more practical system as the server will not end up creating thread for each client process.

To compile this program:-

g++ server_one_to_one.cpp -pthreads -o server_one_to_one
g++ server_threadpool.cpp -pthreads -o server_threadpool
g++ client.cpp -o client

To run this program:-

One to one server model : 
Open two terminals. On one terminal type ./server_one_to_one <assign port_no between user port range 1024-49151>
On second terminal type ./client 127.0.0.1 <port assigned on server side> file1.txt file2.txt file3.txt

Thread Pool – Many to Many Server model :
Open two terminals. 
On one terminal type ./server_threadpool <assign port_no between user port range 1024-49151> <number of threads you want>
On second terminal type ./client 127.0.0.1 <port assigned on server side> file1.txt file2.txt file3.txt

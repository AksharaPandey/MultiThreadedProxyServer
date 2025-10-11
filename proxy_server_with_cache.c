#include "proxy_parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

//Define maximum clients
#define MAX_CLIENTS 10
typedef struct cache_element cache_element;
struct cache_element{
    char* data; //pointer to the data
    int len; //length of the data
    char* url; //url as the key
    time_t lru_time_track; //time to track LRU
    struct cache_element* next; //pointer to the next element
};
cache_element* find(char* url);
int add_cache_element(char* data,int size,char* url);
void remove_cache_element();

int port_number=8080;
int proxy_socketId; //socket id for the proxy server on the global level

//Number of threads will be equal to the number of clients connected
pthread_t tid[MAX_CLIENTS]; //thread id for each client 

//Semaphore and mutex
sem_t semaphore;
pthread_mutex_t lock;

cache_element* head; //head of the cache linked list
int cache_size;
void *thread_fn(void *socketNew){
    sem_wait(&semaphore); //wait for the semaphore
}
int main(int argc, char* argv[]){
    int client_socketId; //socket id for the client
    int client_len; //length of the client address
    struct sockaddr_in server_addr, client_addr; //server and client address structures
    sem_init(&semaphore,0,MAX_CLIENTS); //initialize semaphore

    //initialize mutex
    pthread_mutex_init(&lock,NULL);
    if(argv==2){
      // .proxy 9090
      port_number=atoi(argv[1]);
    }
    else{
        print("Too few arguments");
        exit(1);
    }

    print("Starting Proxy Server on port %d\n",port_number);
    //Create socket
    proxy_socketId=socket(AF_INET,SOCK_STREAM,0);
    if(proxy_socketId<0){
        print("Error in creating socket\n");
        exit(1);
    }
    int reuse=1;
    if(setsockopt(proxy_socketId,SOL_SOCKET,SO_REUSEADDR,(const char*)&reuse,sizeof(reuse))<0){
        print("Error in setting socket options\n");
        exit(1);
    }
    bzero((char*)&server_addr,sizeof(server_addr)); //clear the server address structure
    server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port_number); // Assigning port to the Proxy
	server_addr.sin_addr.s_addr = INADDR_ANY; // Any available adress assigned
    if(bind(proxy_socketId,(struct sockaddr*)&server_addr,sizeof(server_addr)<0)){
        perror("Error in binding\n");
        exit(1);
    }
    printf("Proxy Server is running on port %d\n",port_number);
    int listen_status=listen(proxy_socketId,MAX_CLIENTS);
    if(listen_status<0){
        perror("Error in listening\n");
        exit(1);
    }
    int i=0;
    int Connected_socketId[MAX_CLIENTS]; //array to store the connected socket ids
    while(1){
        bzero((char*)&client_addr,sizeof(client_addr)); //clear the client address structure
        client_len=sizeof(client_addr);
        client_socketId=accept(proxy_socketId,(struct sockaddr*)&client_addr,(socklen_t*)&client_len);
        if(client_socketId<0){
            perror("Error in accepting connection\n");
            exit(1);
        }
        else{
            Connected_socketId[i]=client_socketId;
        }
        struct sockaddr_in* client_pt = (struct sockaddr_in*)&client_addr;
        struct in_addr ip_addr=client_pt->sin_addr;
        char str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET,&ip_addr,str,INET6_ADDRSTRLEN);
        printf("Client is connected with port number %d and IP address %s\n",ntohs(client_pt->sin_port),str);


        pthread_create(&tid[i],NULL,thread_fn,(void*)&Connected_socketId[i]);
        i++;
    }
    close(proxy_socketId);
    return 0;

}
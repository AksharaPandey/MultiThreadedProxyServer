#include "proxy_parse.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

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
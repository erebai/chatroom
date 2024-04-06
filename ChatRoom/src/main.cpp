#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> 
#include <sys/epoll.h>
#include <string.h>

#include <../include/utils.h>
#include <../include/server.h>


int main(int argc,char* argv[]){
    if(argc<=3){
        printf("Using Port:%s \n",argv[1]);
        // printf("Using IP:%n \n",INADDR_ANY);
    }
    int port = atoi(argv[1]);

    Server* room1 = new Server(port);
    room1->listenLoop();
}
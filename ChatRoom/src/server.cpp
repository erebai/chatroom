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

#define MAX_BUF_SIZE 1024
#define MAX_EVENT_NUMBERS 100
char message[MAX_BUF_SIZE];
struct client_data
{
    sockaddr_in address;        //用户地址
    char* write_buf;            //需要写的数据的位置
    char buf[MAX_BUF_SIZE];     //读到的数据储存位置
};

bool Server::readMessageFromClient(int socket){
    int m_read_idx = 0;
    int bytes_read = 0;
    while(true)
        {
            bytes_read = recv(socket, &message + m_read_idx, MAX_BUF_SIZE - m_read_idx, 0);
            if (bytes_read == -1){
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;
                else{
                    printf("Unknown Error Encountered!\n");
                    return false;
                }
            }
            else if (bytes_read == 0){
                printf("client close socket!\n");
                return false;
            }
            m_read_idx += bytes_read;
        }
    return true;
}

void Server::dealwithclient(){
    sockaddr clientaddr;
    socklen_t clientaddr_len = sizeof(clientaddr);
    int connfd = accept(listenfd,(struct sockaddr*)&clientaddr,&clientaddr_len);
    if(connfd>0){
        printf("accpet\n");
        Util::addfd(epollfd,connfd,false);
    }
    else
        printf("Wrong\n");
};

void Server::dealwithread(int socket){
    //after deal with read data,reset ontshot
    memset(&message,'\0',MAX_BUF_SIZE);
    if(readMessageFromClient(socket)){
        printf("message from clinet:%s \n",message);
        Util::resetEpollOneShot(epollfd,socket);
    }
    else{
        printf("Unexpected errors encoutered \n");
        Util::removefd(epollfd,socket);
        close(socket);
    }
};

void Server::dealwithwrite(int socket){

};

void Server::listenLoop(){
    //init socket
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    listenfd = socket(PF_INET,SOCK_STREAM,0);
    if(listenfd == -1){
        printf("Error:Fail to create socket\n");
        return;
    }
    
    //bind
    int result = 0;
    result = bind(listenfd,(struct sockaddr*)&address,sizeof(address));
    if(result == -1){
        printf("Error:Fail to bind socke\n");
        return;
    }

    //listen
    result = listen(listenfd,5);
    if(result == -1){
        printf("Error:Fail to create listen\n");
        return;
    }

    epoll_event events[MAX_EVENT_NUMBERS+1];
    client_data* users = new client_data[MAX_EVENT_NUMBERS];
    epollfd = epoll_create(5);
    Util::addfd(epollfd,listenfd);

    while(1){
        result = epoll_wait(epollfd,events,MAX_EVENT_NUMBERS,-1);
        if(result < 0){
            printf("epoll failed\n");
            break;
        }
        for(int i=0;i<result;++i){
            int sockfd = events[i].data.fd;
            if((sockfd == listenfd) &&(events[i].events & EPOLLIN)){
                printf("Epoll:New client connected!\n");
                dealwithclient();
            }
            else if(events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                printf("client disconnected!\n");
                Util::removefd(epollfd,sockfd);
                break;
            }
            else if(events[i].events & EPOLLIN){
                printf("Epoll:message from client.\n");
                dealwithread(sockfd);
            }
            else if(events[i].events & EPOLLOUT){
                dealwithwrite(sockfd);
            }
        }
    }
    close(listenfd);
}

Server::Server(int port):port(port)
{

}
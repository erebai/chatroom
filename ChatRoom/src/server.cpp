#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> 
#include <sys/epoll.h>
#include <string.h>

#include <../include/utils.h>

#define MAX_BUF_SIZE 1024
#define MAX_EVENT_NUMBERS 100
char message[MAX_BUF_SIZE];

bool readMessageFromClient(int epollfd,int fd){
    int m_read_idx = 0;
    int bytes_read = 0;
    while(true)
        {
            bytes_read = recv(fd, &message + m_read_idx, MAX_BUF_SIZE - m_read_idx, 0);
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

void dealwithclient(int epollfd,int sockfd){
    sockaddr clientaddr;
    socklen_t clientaddr_len = sizeof(clientaddr);
    int connfd = accept(sockfd,(struct sockaddr*)&clientaddr,&clientaddr_len);
    Util::addfd(epollfd,connfd,false);
};

void dealwithread(int epollfd,int fd){
    //after deal with read data,reset ontshot
    memset(&message,'\0',MAX_BUF_SIZE);
    if(readMessageFromClient(epollfd,fd)){
        printf("message from clinet:%s \n",message);
        Util::resetEpollOneShot(epollfd,fd);
    }
    else{
        Util::removefd(epollfd,fd);
        close(fd);
    }
    
};

void dealwithwrite(int epollfd,int fd){

};

int main(int argc,char* argv[]){
    if(argc<=3){
        printf("Using Port:%s \n",argv[1]);
        // printf("Using IP:%n \n",INADDR_ANY);
    }
    int port = atoi(argv[1]);
    //init socket
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    int listenfd = socket(PF_INET,SOCK_STREAM,0);
    if(listenfd == -1){
        printf("Error:Fail to create socket\n");
        return 0;
    }
    
    //bind
    int result = 0;
    result = bind(listenfd,(struct sockaddr*)&address,sizeof(address));
    if(result == -1){
        printf("Error:Fail to bind socke\n");
        return 0;
    }

    //listen
    result = listen(listenfd,5);
    if(result == -1){
        printf("Error:Fail to create listen\n");
        return 0;
    }

    epoll_event events[MAX_EVENT_NUMBERS+1];
    int epollfd = epoll_create(5);
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
                dealwithclient(epollfd,sockfd);
            }
            else if(events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                printf("client disconnected!\n");
                Util::removefd(epollfd,sockfd);
                break;
            }
            else if(events[i].events & EPOLLIN){
                printf("Epoll:message from client.\n");
                dealwithread(epollfd,sockfd);
            }
            else if(events[i].events & EPOLLOUT){
                dealwithwrite(epollfd,sockfd);
            }
        }
    }

    close(listenfd);
    return 1;
}
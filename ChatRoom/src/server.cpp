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
using namespace std;

bool Server::readMessageFromClient(int socket){
    memset(clients[socket].buf,0,sizeof(clients[socket].buf));
    int m_read_idx = 0;
    int bytes_read = 0;
    while(true)
        {
            bytes_read = recv(socket, &clients[socket].buf + m_read_idx, MAX_BUF_SIZE - m_read_idx, 0);
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

void Server::dealwithclient(int socket){
    struct sockaddr_in clientaddr;
    socklen_t clientaddr_len = sizeof(clientaddr);
    int connfd = accept(listenfd,(struct sockaddr*)&clientaddr,&clientaddr_len);
    if(connfd>0){
        Util::addfd(epollfd,connfd);
        clients[connfd].address = clientaddr;
        clients[connfd].socket = connfd;
        num_users++;
    }
    else printf("fail to accept\n");
    
    if(readMessageFromClient(connfd)){
        memcpy(clients[connfd].username,clients[connfd].buf,MAX_USERNAME_LENGTH);
        printf("New client connected! UserID:%s \n",clients[connfd].username);
    }
    else{
        printf("wrong!!!!!!!!\n");
        close(connfd);
        num_users--;
    }

};

void Server::dealwithread(int socket){
    //after deal with read data,reset ontshot
    if(readMessageFromClient(socket)){
        printf("message from %s :%s\n",clients[socket].username,clients[socket].buf);
        memcpy(clients[socket].buf+strlen(clients[socket].username)+1,clients[socket].buf,strlen(clients[socket].buf));
        clients[socket].buf[strlen(clients[socket].username)]  = ':';
        memcpy(clients[socket].buf,clients[socket].username,strlen(clients[socket].username));
        Util::resetEpollOneShot(epollfd,socket);
    }
    else{
        Util::removefd(epollfd,socket);
        close(socket);
    }
};

void Server::dealwithwrite(int socket){
    //分发给所有用户
    int result = send(socket,clients[socket].write_buf,strlen(clients[socket].write_buf),0);
    if(result  == -1)
        std::cout<<"sending not end"<<std::endl;
};

void Server::listenLoop(){
    //init socket
    
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    listenfd = socket(PF_INET,SOCK_STREAM,0);
    int reuse=1;
    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));
    if(listenfd == -1){
        printf("Error:Fail to create socket\n");
        return;
    }
    
    //bind
    int result = 0;
    result = bind(listenfd,(struct sockaddr*)&address,sizeof(address));
    if(result == -1){
        printf("Error:Fail to bind socket\n");
        return;
    }

    //listen
    result = listen(listenfd,5);
    if(result == -1){
        printf("Error:Fail to create listen\n");
        return;
    }

    epoll_event events[MAX_EVENT_NUMBERS+1];
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
                dealwithclient(sockfd);
            }
            else if(events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                printf("client disconnected!\n");
                Util::removefd(epollfd,sockfd);
                break;
            }
            else if(events[i].events & EPOLLIN){
                dealwithread(sockfd);
                epoll_event event;
                int offset = 0;
                int index = 0;
                for(int j=0;j<MAX_EVENT_NUMBERS;++j){
                    if(clients[j].socket != -1){
                        offset = j;
                        break;
                    }
                }
                for(int j=0;j<num_users;++j){
                    if(sockfd == offset+j) continue;
                    index = offset+j;
                    event.data.fd = index;
                    event.events = EPOLLOUT | EPOLLET;
                    clients[index].write_buf = clients[sockfd].buf;
                    epoll_ctl(epollfd,EPOLL_CTL_MOD,index,&event);
                }
            }       
            else if(events[i].events & EPOLLOUT){
                dealwithwrite(sockfd);
                epoll_event event;
                event.data.fd = sockfd;
                event.events = EPOLLIN | EPOLLET;
                epoll_ctl(epollfd,EPOLL_CTL_MOD,sockfd,&event);
            }
        }
    }
    close(listenfd);
}

Server::Server(int port):port(port)
{

}
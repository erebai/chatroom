#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> 
#include <sys/epoll.h>
#include <string.h>
#include <../include/Util.h>
#define MAX_BUF_SIZE 1024
#define MAX_EVENT_NUMBERS 100
#define MAX_USERNAME_LENGTH 10
char message[MAX_BUF_SIZE];
bool readMessageFromClient(int socket){
    memset(message,0, sizeof(message)); 
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


int main(int argc,char* argv[]){
    if(int(strlen(argv[3]))>MAX_USERNAME_LENGTH){
        printf("Username too long!\n");
        return 1;
    }

    //init socket
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(argv[1]);
    address.sin_port = htons(atoi(argv[2]));
    char * username = argv[3];
    std::cout<<"user:"<<username<<" Logging in..."<<std::endl;
    int sock = socket(PF_INET,SOCK_STREAM,0);
    if(sock == -1){
        printf("Error:Fail to create socket");
        return 0;
    }
    
    //bind
    int result = 0;
    result = connect(sock,(struct sockaddr*)&address,sizeof(address));
    if(result == -1){
        std::cout<<"Fail to connect host:"<<argv[1]<<std::endl;
        close(sock);
        return 1;
    }
    std::cout<<"Successful!"<<std::endl;
    if(-1 == send(sock,username,strlen(username),0)){
        std::cout<<"Fail to send username:"<<argv[3]<<std::endl;
        close(sock);
        return 1;
    }

    epoll_event events[MAX_EVENT_NUMBERS+1];
    int epollfd = epoll_create(5);
    Util::addfd(epollfd,0);                 //注册标准输入
    Util::addfd(epollfd,sock,false);        //注册连接socket，事件：读

    int pipefd[2];
    result = pipe(pipefd);
    if(result == -1){
        printf("fail to create pipe.\n");
        close(sock);
        return 1;
    }

    while (1)
    {
        result = epoll_wait(epollfd,events,MAX_EVENT_NUMBERS,-1);
        if(result < 0){
            printf("Epoll failed!");
            break;
        }
        for(int i=0;i<result;++i){
            int sockfd = events[i].data.fd;
            if((sockfd == 0)&&(events[i].events & EPOLLIN)){
                splice(0,NULL,pipefd[1],NULL,MAX_BUF_SIZE,SPLICE_F_MORE|SPLICE_F_MOVE);
                splice(pipefd[0],NULL,sock,NULL,MAX_BUF_SIZE,SPLICE_F_MORE|SPLICE_F_MOVE);
                //Keyboard standard input
            }
            else if(events[i].events & EPOLLIN){
                if(readMessageFromClient(sockfd))
                    printf("Message from %s",message);
                else{
                    return 1;
                }
            }
        }
    }
    
    close(sock);
    return 1;
}
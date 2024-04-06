// #ifndef
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> 
#include <sys/epoll.h>
#include <string.h>
#include <fcntl.h>

class Util
{
    public:
        static void addfd(int epollfd,int fd,bool oneshot = false){
            epoll_event event;
            event.data.fd = fd;
            event.events = EPOLLIN|EPOLLET;
            if(oneshot){
                event.events |= EPOLLONESHOT;
            }
            epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
            setnonblock(fd);
        }

        static int setnonblock(int fd){
            int old_options = fcntl(fd,F_GETFL);
            int new_options = old_options|O_NONBLOCK;
            fcntl(fd,F_SETFL,new_options);
            return old_options;
        }

        static void resetEpollOneShot(int epollfd,int fd){ 
            epoll_event event;
            event.data.fd=fd;
            event.events=EPOLLIN|EPOLLET|EPOLLONESHOT;
            epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&event);
        }

        static void removefd(int epollfd,int fd){ 
            epoll_ctl(epollfd, EPOLL_CTL_DEL,fd,0);
        }
};
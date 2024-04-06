class Server
{
    public:
        void listenLoop();
        void init();
        void dealwithread(int socket);
        void dealwithwrite(int socket);
        void dealwithclient();
        bool readMessageFromClient(int socket);
        Server(int port);

    private:
        int epollfd;        //epoll事件
        int port;           //本机监听端口
        int listenfd;       //监听socket
};




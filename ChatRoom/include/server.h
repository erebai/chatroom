#define MAX_BUF_SIZE 1024
#define MAX_EVENT_NUMBERS 100
#define MAX_USERNAME_LENGTH 10

struct client_data
{   
    int socket = -1;
    char username[MAX_USERNAME_LENGTH];
    sockaddr_in address;        //用户地址
    char* write_buf;            //需要写的数据的位置
    char buf[MAX_BUF_SIZE];     //读到的数据储存位置
};

class Server
{
    public:
        void listenLoop();
        void init();
        void dealwithread(int socket);
        void dealwithwrite(int socket);
        void dealwithclient(int socket);
        bool readMessageFromClient(int socket);
        Server(int port);

    private:
        int epollfd;                        //epoll事件
        int port;                           //本机监听端口
        int listenfd;                       //监听socket
        client_data clients[MAX_EVENT_NUMBERS];   //所有客户
        char message[MAX_BUF_SIZE];
        int num_users;
};




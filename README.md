# ChatRomm
## 1.使用说明:
下载后在终端输入以下代码编译：
~~~
cd chatroom/ChatRoom/build
rm -rf *
cmake ..
make
~~~

编译后生成可执行文件server，调用方式如下
~~~
./server [端口号]
//example:
./server 6006
~~~

然后唤起客户端,在终端输入以下代码编译：
~~~
cd chatroom/ChatRoomClient/build
rm -rf *
cmake ..
make
~~~

编译后生成可执行文件client，调用方式如下
~~~
./client [服务器ip地址] [端口号] [用户名]
//example:
./client ./client 127.0.0.1 6006 user1
~~~
## 2.Todo
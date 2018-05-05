## How to run
* server:
./main

* epoll_server
./main

* client:
./main 47.91.245.251 3030 0.0.0.0 3030

或者使用nc,可以使用nc来调试程序
nc 47.91.245.251 3030

* clientstress
用fork创建多个进程来访问服务器
不同进程之间生成一个随机数来区分client socket的port

* clientSession:
连接上服务器socket之后，可以读取终端输入的命令发送到服务器端
./main 47.91.245.251 3030 0.0.0.0 3030


## todo
* 修改epoll_server 的makefile文件
* 在服务器和客户端添加一个命令的构造器，支持redis支持的5种命令
* 在epoll服务器前端添加一个 消息队列，并使用令牌桶算法(Token Bucket)限制流量



## question
解决epoll在EL模式下的丢包问题：
https://blog.csdn.net/drdairen/article/details/53896354

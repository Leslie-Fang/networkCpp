//
// Created by leslie-fang on 2018/4/28.
//
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define MAXEPOLL    10000   /* 对于服务器来说，这个值可以很大的！ */
#define SERVPORT 3030   // 服务器监听端口号
#define BACKLOG 20  // 最大同时连接请求数
#define MAXDATASIZE 1000
#define MAX_LISTEN_EVENT 100

//epoll 使用EL模式时，必须要将监听事件的socket配置为非阻塞的IO
int setnonblocking( int fd )
{
    if( fcntl( fd, F_SETFL, fcntl( fd, F_GETFD, 0 )|O_NONBLOCK ) == -1 )
    {
        printf("Set blocking error : %d\n", errno);
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[]){
    //std::cout<<"Hello!"<<std::endl;
    int sock_fd,client_fd;
    int epoll_fd;
    int ret_number;
    int sin_size;
    int cur_fds=0;//记录已经在监听的fd的个数
    struct sockaddr_in server_addr;
    struct sockaddr_in remote_addr;
    struct  epoll_event ev;
    struct  epoll_event evs[MAXEPOLL];
    char buf[MAXDATASIZE];
    int recvbytes;

    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Create Server socket error！");
        exit(1);
    }else{
        std::cout<<"socket des: "<<sock_fd<<std::endl;
    }
    long flag = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag));

    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(SERVPORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero),8);

    if(bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) != 0) {
        perror("bind socket error！");
        exit(1);
    }
    if(listen(sock_fd, BACKLOG) == -1) {
        perror("set socket listen error！");
        exit(1);
    }
    setnonblocking(sock_fd);
    //create epoll fd
    epoll_fd = epoll_create( MAXEPOLL );    //!> create
    ev.events = EPOLLIN | EPOLLET;      //!> accept Read!
    ev.data.fd = sock_fd;                 //!> 将listen_fd 加入
    if( epoll_ctl( epoll_fd, EPOLL_CTL_ADD, sock_fd, &ev ) < 0 )
    {
        printf("Epoll Error : %d\n", errno);
        exit( EXIT_FAILURE );
    }
    cur_fds++;
    int count = 0;
    while(1){
        //注意:这里某个socket断开连接的时候，也会唤醒epoll_wait，并进入EPOLLIN事件，只是读取时返回的大小为0，此时在socket端应该删除监听
        if((ret_number = epoll_wait(epoll_fd, evs, MAX_LISTEN_EVENT, -1 )) == -1 )
        {
            printf( "Epoll Wait Error : %d\n", errno );
            exit( EXIT_FAILURE );
        }
        count++;
        printf("loop count %d\n", count);
        printf("wake up events %d\n", ret_number);
        for(int i = 0; i < ret_number; i++ ){
            if(evs[i].data.fd == sock_fd && cur_fds<MAXEPOLL){
                //监听到有socket请求连接，accept并创建连接，设置新连接到epoll的监听中
                //这里while循环很重要，因为对epoll使用了ET(边沿触发的模式),监听着的sock_fd，如果在一次事件中，有多个客户端请求连接需要处理，不使用while循环只会处理一次，导致丢失accept的事件
                //用while循环可以处理完所有的connect请求,但是一定要把监听的socket设置为非阻塞模式，否则最后会等待新的请求而阻塞在这里
                sin_size = sizeof(struct sockaddr_in);
                while((client_fd = accept( sock_fd, (struct sockaddr *)&remote_addr, (socklen_t*)&sin_size) ) != -1 ){
                    printf("accept a new connection!\n");
                    setnonblocking(client_fd);
                    ev.events = EPOLLIN | EPOLLET;      //!> accept Read!
                    ev.data.fd = client_fd;                   //!> 将conn_fd 加入
                    if( epoll_ctl( epoll_fd, EPOLL_CTL_ADD, client_fd, &ev ) < 0 ){
                        printf("Epoll Error : %d\n", errno);
                        exit( EXIT_FAILURE );
                    }
                    cur_fds++;
                }

//                if( (client_fd = accept( sock_fd, (struct sockaddr *)&remote_addr, (socklen_t*)&sin_size) ) == -1 )
//                {
//                    printf("Accept Error : %d\n", errno);
//                    exit( EXIT_FAILURE );
//                }
//                printf("accept a new connection!\n");
//                setnonblocking(client_fd);
//                ev.events = EPOLLIN | EPOLLET;      //!> accept Read!
//                ev.data.fd = client_fd;                   //!> 将conn_fd 加入
//                if( epoll_ctl( epoll_fd, EPOLL_CTL_ADD, client_fd, &ev ) < 0 )
//                {
//                    printf("Epoll Error : %d\n", errno);
//                    exit( EXIT_FAILURE );
//                }
//                cur_fds++;
            }else if(evs[i].events & EPOLLIN){
                //读事件
                if((client_fd=evs[i].data.fd)<0){
                    printf("read socket Error : %d\n", errno);
                    continue;
                }

//                //因为使用边沿触发的模式，需要在一次epoll事件中将所有数据读完，否则在下一个epoll事件中，不会包含上一个epoll事件中没读完的数据的event
//                //这是边沿触发，如果是水平触发，则在下一个epoll事情中，会去处理上一个epoll事情中没处理完的event
//                int buffer_space_count=0;
//                while((recvbytes=recv(client_fd, buf+buffer_space_count, MAXDATASIZE, 0)) > 0){
//                    buffer_space_count += recvbytes;
//                    if(buffer_space_count >= MAXDATASIZE-1){
//                        printf("buffer is out of space.\n");
//                        //todo::solve the recv buufer is out of space error
//                        break;
//                    }
//                }
//                if (recvbytes == -1 && errno != EAGAIN)
//                {
//                    perror("recv error");
//                }
//                //todo: don;t know why, 但是这里最终退出的状态就是recvbytes == -1 && errno == EAGAIN
//                //结束与当前socket的通信
//                buf[buffer_space_count] = '\0';
//                printf("Received: %s",buf);
//                //write data back
//                if(send(client_fd, "Hello, received your message!\n", 30, 0) == -1) {
//                    perror("send error！");
//                }
//                printf("Close connected socket.\n");
//                close(client_fd);
//                epoll_ctl( epoll_fd, EPOLL_CTL_DEL, evs[i].data.fd, &ev );
//                cur_fds--;
                if((recvbytes=recv(client_fd, buf, MAXDATASIZE, 0)) == -1) {
                    perror("recv error！");
                }else if(recvbytes == 0){
                    //结束与当前socket的通信
                    //注意:这里某个socket断开连接的时候，也会唤醒epoll_wait，并进入EPOLLIN事件，只是读取时返回的大小为0，此时在socket端应该删除监听
                    //https://blog.csdn.net/yongqingjiao/article/details/78819791
                    //客户端调用 close()），在服务器端会触发一个 epoll 事件。在低于 2.6.17 版本的内核中，这个 epoll 事件一般是 EPOLLIN
                    printf("Close connected socket.\n");
                    close(client_fd);
                    epoll_ctl( epoll_fd, EPOLL_CTL_DEL, evs[i].data.fd, &ev );
                    cur_fds--;
                }else{
                    buf[recvbytes] = '\0';
                    printf("Received: %s",buf);
                    //write data back
                    if(send(client_fd, "Hello, received your message!\n", 30, 0) == -1) {
                        perror("send error！");
                    }
                }
            }
            //todo::添加并尝试写事件
        }
    }
    close( sock_fd );
    close( epoll_fd );
    return 0;
}


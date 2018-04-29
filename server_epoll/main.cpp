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
#define BACKLOG 10  // 最大同时连接请求数
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
        if( ( ret_number = epoll_wait(epoll_fd, evs, MAX_LISTEN_EVENT, -1 ) ) == -1 )
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
                sin_size = sizeof(struct sockaddr_in);
                if( ( client_fd = accept( sock_fd, (struct sockaddr *)&remote_addr, (socklen_t*)&sin_size) ) == -1 )
                {
                    printf("Accept Error : %d\n", errno);
                    exit( EXIT_FAILURE );
                }
                printf("accept a new connection!\n");
                setnonblocking(client_fd);
                ev.events = EPOLLIN | EPOLLET;      //!> accept Read!
                ev.data.fd = client_fd;                   //!> 将conn_fd 加入
                if( epoll_ctl( epoll_fd, EPOLL_CTL_ADD, client_fd, &ev ) < 0 )
                {
                    printf("Epoll Error : %d\n", errno);
                    exit( EXIT_FAILURE );
                }
                cur_fds++;
            }else if(evs[i].events & EPOLLIN){
                //读事件
                if((client_fd=evs[i].data.fd)<0){
                    printf("read socket Error : %d\n", errno);
                    continue;
                }
                if((recvbytes=recv(client_fd, buf, MAXDATASIZE, 0)) == -1) {
                    perror("recv error！");
                }else if(recvbytes == 0){
                    //结束与当前socket的通信
                    //注意:这里某个socket断开连接的时候，也会唤醒epoll_wait，并进入EPOLLIN事件，只是读取时返回的大小为0，此时在socket端应该删除监听
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


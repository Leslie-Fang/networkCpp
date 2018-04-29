#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SERVPORT 3030   // 服务器监听端口号
#define BACKLOG 10  // 最大同时连接请求数
#define MAXDATASIZE 1000
int main(int argc, char *argv[]) {
    int sock_fd,client_fd;
    int sin_size;
    struct sockaddr_in server_addr;
    struct sockaddr_in remote_addr;
    char buf[MAXDATASIZE];
    int recvbytes;
    //create the socket
    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Create Server socket error！");
        exit(1);
    }else{
        std::cout<<"socket des: "<<sock_fd<<std::endl;
    }
    //set the socket option
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
    while(1) {
        sin_size = sizeof(struct sockaddr_in);
        if((client_fd = accept(sock_fd, (struct sockaddr *)&remote_addr, (socklen_t*)&sin_size)) == -1) {
            perror("accept error");
            continue;
        }
        printf("received a connection from %s:%u\n", inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port));
        if(!fork()) {   // 子进程代码段，fork返回0表示子进程
            if((recvbytes=recv(client_fd, buf, MAXDATASIZE, 0)) == -1) {
                perror("recv error！");
                return 0;
            }
            buf[recvbytes] = '\0';
            printf("Received: %s",buf);
            if(send(client_fd, "Hello, you are connected!\n", 26, 0) == -1) {
                perror("send error！");
            }
            close(client_fd);
            exit(0);//退出子进程
        }
        close(client_fd);
    }
    close(sock_fd);
    return 0;
}
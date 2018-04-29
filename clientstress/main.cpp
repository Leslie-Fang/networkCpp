//
// Created by leslie-fang on 2018/4/29.
//
#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <time.h>
#define MAXDATASIZE 1000
int trySocket(in_addr_t server_ip,in_port_t server_port,in_addr_t my_ip,in_port_t my_port);
int main(int argc, char *argv[]){
    pid_t fpid;
    int number_pid = 10;
    in_addr_t server_ip = inet_addr(argv[1]);
    in_port_t server_port = atoi(argv[2]);
    in_addr_t my_ip = inet_addr(argv[3]);
    in_port_t my_port = atoi(argv[4]);
    if (argc != 5) {
        printf("Input format error\n");
        return 0;
    }
    for(int i=0;i<number_pid;i++){
        fpid=fork();
        usleep(100);
        if(fpid <0){
            printf("error in fork!\n");
            exit(-1);
        }else if(fpid == 0){
            printf("son\n");
            //* count = (* count) + 1;
            break;//这个break很重要，这里一共创建了3个进程
        }else{
            printf("father\n");
        }
    }
    srand(getpid());
    my_port = my_port+rand()%200;
    std::cout << my_port<<std::endl;
    if(trySocket(server_ip,server_port,my_ip,my_port)==-1){
        exit(-1);
    }
    if(fpid == 0){
        //所有子进程
        exit(0);
    }
    //最原始的父进程
    int status = -1;
    wait(&status);
    printf("end\n");
    exit(0);
}

int trySocket(in_addr_t server_ip,in_port_t server_port,in_addr_t my_ip,in_port_t my_port){
    int sock_fd = -1;
    char buf[MAXDATASIZE];
    int recvbytes, sendbytes, len;

    //create socket
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        return -1;
    }
    //set socket option
    long flag = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag));

    // bind the client ip
    struct sockaddr_in my_addr;
    my_addr.sin_addr.s_addr = my_ip;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(my_port);

    if (bind(sock_fd, (sockaddr*)&my_addr, sizeof(sockaddr)) < 0) {
        perror("bind error");
        return -1;
    }
    //printf("Try to connect server(%s:%u)\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

    //set server ip and port
    struct sockaddr_in server_addr;
    server_addr.sin_addr.s_addr = server_ip;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if(connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        perror("connect error！");
        return -1;
    }
    printf("Connect server success(%s:%u)\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    //sleep(10);
    if(send(sock_fd, "Hello, server I am try to connected!\n", 50, 0) == -1) {
        perror("send error！");
        return -1;
    }
    if((recvbytes=recv(sock_fd, buf, MAXDATASIZE, 0)) == -1) {
        perror("recv error！");
        return -1;
    }
    buf[recvbytes] = '\0';
    printf("Received: %s",buf);
    close(sock_fd);
    return 0;

}




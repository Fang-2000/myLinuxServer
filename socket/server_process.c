#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#define SERVERIP "127.0.0.1"
#define PORT 6666

void recycleChild(int arg){
    //while：处理多个信号
    while (1)
    {
        int ret = waitpid(-1, NULL, WNOHANG);
        if(ret == -1){
            //所有子进程都回收了
            break;
        }else if(ret == 0){
            //还有子进程活着
            break;
        }else{
            //回收子进程
            printf("子进程 %d 被回收了\n", ret);
        }
    }
}

int main(){

    //注册信号捕捉
    struct sigaction act;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    act.sa_handler = recycleChild;
    sigaction(SIGCHLD, &act, NULL);

    //1.创建socket
    //AF_INET代表协议簇为ipv4,SOCK_STREAM : 流式协议
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1){
        perror("socket");
        exit(-1);
    }

    //2.绑定
    struct sockaddr_in server_addr;
    server_addr.sin_family = PF_INET;
    //点分十进制转化为网络字节序
    inet_pton(AF_INET, SERVERIP, &server_addr.sin_addr.s_addr);
    // 服务端也可以绑定0.0.0.0即任意地址
    // server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    int ret = bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret == -1){
        perror("bind");
        exit(-1);
    }

    //3.监听
    ret = listen(listenfd, 8);
    if(ret == -1){
        perror("listen");
        exit(-1);
    }

    // 不断循环等待客户端连接
    while(1){
        //4.接受客户端连接
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int connectfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_addr_len);
        if(connectfd == -1){
            // 用于处理信号捕捉导致的accept: Interrupted system call
            if(errno == EINTR){
                continue;
            }
            perror("accept");
            exit(-1);
        }
        pid_t pid = fork();
        if(pid == 0){
            //子进程
            //输出客户端信息
            char client_ip[16] = {0};
            inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip));
            unsigned short client_port = ntohs(client_addr.sin_port);
            printf("ip: %s, port: %d\n", client_ip, client_port);

            //5.开始通信
            // 服务端先接收客户端信息，再向客户端发送数据
            // 接收数据
            char recv_buf[1024] = {0};
            while (1)
            {
                ret = read(connectfd, recv_buf, sizeof(recv_buf));
                if(ret == -1){
                    perror("read");
                    exit(-1);
                }else if(ret > 0){
                    printf("recv client data: %s\n", recv_buf);
                }else{
                    printf("client closed...\n");
                    break;
                }

                //发送数据
                char* send_buf = "hello, this is server";
                write(connectfd, send_buf, strlen(send_buf));
            }

            close(connectfd);
        }
    }
    close(listenfd);
    return 0;
}
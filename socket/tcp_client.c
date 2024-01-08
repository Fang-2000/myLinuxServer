#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<string.h>

#define SERVERIP "127.0.0.1"
#define PORT 6666

int main(){

    //1.创建socket
    int connectfd = socket(AF_INET, SOCK_STREAM, 0);    
    if(connectfd == -1){
        perror("socket");
        exit(-1);
    }

    //2.连接服务器
    struct sockaddr_in server_addr;
    server_addr.sin_family = PF_INET;
    //点分十进制转化为网络字节序
    inet_pton(AF_INET, SERVERIP, &server_addr.sin_addr.s_addr);
    server_addr.sin_port = htons(PORT);
    int ret = connect(connectfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret == -1){
        perror("connect");
        exit(-1);
    }

    //3.通信
    char recv_buf[1024] = {0};
    while (1)
    {
        char* send_buf = "client message";
        write(connectfd, send_buf, strlen(send_buf));
        sleep(1);
        ret = read(connectfd, recv_buf, sizeof(recv_buf));
        if(ret == -1){
            perror("read");
            exit(-1);
        }else if(ret > 0){
            printf("recv server data: %s\n", recv_buf);
        }else{
            printf("server closed...\n");
            break;
        }
    }

    close(connectfd);
    return 0;
    

}
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "csi_fun.h"

#define BUFSIZE 4096

int quit;

/**
 * @brief Handle the Ctrl+C signal
 * @param sig
 */
void handle_sigint(int sig)
{
    if (sig == SIGINT) {
        /* 打印信号值 */
        printf("Caught signal %d\n", sig);

        /* 设置退出标志 */
        quit = true;
    }
}

/**
 * @brief Setup the UDP broadcast socket
 * @param socket_fd
 * @param server_addr
 * @param ip_addr
 * @param port
 */
void setup_udp_broadcast(int *socket_fd, struct sockaddr_in *server_addr, const char *ip_addr, const int port) {
    int broadcast = 1;

    if ((*socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    if (setsockopt(*socket_fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        perror("setsockopt");
        exit(1);
    }

    bzero(server_addr, sizeof(struct sockaddr_in));
    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = inet_addr(ip_addr);
    server_addr->sin_port = htons(port);
}

/**
 * @brief Broadcast the data to the client
 * @param socket_fd
 * @param client_addr
 * @param data
 * @param len
 */
void broadcast_udp(int socket_fd, struct sockaddr_in *client_addr, unsigned char *data, int len) {

    if (sendto(socket_fd, data, len, 0, (struct sockaddr *)client_addr, sizeof(struct sockaddr)) < 0) {
        perror("sendto");
        exit(1);
    }
}

/**
 * @brief Main function
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char* argv[])
{
    int             csi_fd;         // for csi

    /* 注册信号处理函数 */
    signal(SIGINT, handle_sigint);

    /* 检测输入参数情况 程序名 + BroadCast IP + 端口号 */
    if (argc != 3) {
        printf("Usage: %s <BroadCast IP> <Port>\n", argv[0]);
        return 0;
    }

    /* 设置CSI设备 */
    csi_fd = open_csi_device();
    if (csi_fd < 0) {
        perror("Failed to open the device...");
        return errno;
    }

    /* 设置UDP广播 */
    int socket_fd;
    struct sockaddr_in broadcast_addr;

    /* 从命令行获取广播地址，端口号 */
    const char *ip_addr = argv[1];
    const int port = atoi(argv[2]);
    setup_udp_broadcast(&socket_fd, &broadcast_addr, ip_addr, port);

    /* 准备CSI数据缓冲区 */
    unsigned char buf_addr[BUFSIZE] = {0};
    int cnt = 0;

    /* 设置退出标志 */
    quit = false;

    /* 计时器 */
    struct timeval start, end;

    /* 获取当前时间 */
    gettimeofday(&start, NULL);

    /* 计数器 */
    int count = 0;

    /* 循环读取CSI数据 */
    while(!quit) {

        /* 读取CSI数据，数据存放在buf_addr中，返回值为数据长度 */
        cnt = read_csi_buf(buf_addr, csi_fd, BUFSIZE);

        /* 如果读取到数据 */
        if (cnt > 0) {
            /* 广播数据 */
            broadcast_udp(socket_fd, &broadcast_addr, buf_addr, cnt);
        }
    }

    /* 关闭文件 */
    close_csi_device(csi_fd);

    /* 退出程序 */
    return 0;
}
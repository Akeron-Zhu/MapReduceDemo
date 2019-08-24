#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <bits/stdc++.h>

using namespace std;
#define BUFFER_SIZE 4096
//socket发送数据
void data_send(int sockfd, char sendline[], int len)
{
    if (send(sockfd, sendline, len, 0) < 0)
    {
        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
}
//定义了三个端品，在本地设立了三个进程，模仿三个服务器
string IP[3]={"192.168.0.101","192.168.0.102","192.168.0.103"};
int PORT[3] = {10009, 10010, 10011};

int main(int argc, char **argv)
{
    //如果输入的参数不对，则提示使用方法
    if (argc != 5)
    {
        printf("usage: %s Map Reduce Input Output\n",argv[0]);
        return 0;
    }
    //定义Socket的一些基本变量，
    int sockfd, rec_len;
    struct sockaddr_in servaddr;
    //定义客户端这边接收的buffer的大小。
    char buf[BUFFER_SIZE];
    //得到要处理的文件的名字
    char maper[6] = "Maper";
    char reducer[8] = "Reducer";
    char sendName[128];
    bzero(sendName, sizeof(sendName));
    for (int i = 0; i < strlen(argv[3]); i++)
    {
        sendName[i] = argv[3][i];
    }
    //以二进制读取方式打开文件
    FILE *in_map;
    FILE *in_reduce;
    in_map = fopen(argv[1], "rb");
    in_reduce = fopen(argv[2], "rb");
    //如果不能打开则代表出错
    if (!in_map)
    {
        cout << "Read Map file error!" << endl;
        return 0;
    }
    //如果不能打开则代表出错
    if (!in_reduce)
    {
        cout << "Read Reduce file error!" << endl;
        return 0;
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    for (int i = 0; i < 4; i++)
    {
        fseek(in_map, 0, 0);
        bzero(buf, sizeof(buf)); //若不清零，则下次传送名字出错。
        //得到sockfd用来通信
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
            exit(0);
        }
        //选取不同的端口，因为这里是本地模拟，如果是在不同的服务器，则要将端口与IP都进行更换。
        //更换IP，也即选择不同的节点
        servaddr.sin_port = htons(PORT[i % 3]);
        if (inet_pton(AF_INET, IP[i%3].c_str(), &servaddr.sin_addr) <= 0)
        {
            printf("inet_pton error\n");
            exit(0);
        }
        //进行连接
        if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        {
            printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
            exit(0);
        }
        //前三次发送的名称为InputFilename，而不是Maper，但内容是Maper的内容。
        if (i < 3) 
        {
            string file = argv[3];
            file += char(i + '0');
            strncpy(buf, file.c_str(), strlen(file.c_str()));
        }
        else
            strncpy(buf, reducer, strlen(reducer));
        printf("send msg to server: \n");
        data_send(sockfd, buf, BUFFER_SIZE);
        //发送数据，read_length表示每次读到多少，count用来计数判断是否超过了文件三分之一
        int read_length = 0;
        //读取文件并发送

        while (read_length = fread(buf, sizeof(char), BUFFER_SIZE, i < 3 ? in_map : in_reduce))
        {
            //printf("read_lenght=%d\n",read_length);
            data_send(sockfd, buf, read_length);
        }

        printf("Part %d transfer Finished!\n", i);
        close(sockfd);
    }
    //显示发送的文件名，并且关闭Maper,Reducer的读取
    printf("%s\n", sendName);
    fclose(in_map);
    fclose(in_reduce);

    /***************************等待结果回传*********************************/
    int connect_fd, port = 9999, cnt = 0;
    char fileName[128] = {0};
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //IP地址设置成INADDR_ANY,让系统自动获取本机的IP地址。
    servaddr.sin_port = htons(port);              //设置的端口为DEFAULT_PORT
    //得到sockfd
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
    //将本地地址绑定到所创建的套接字上
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
    //开始监听是否有客户端连接
    if (listen(sockfd, 10) == -1)
    {
        printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }

    while (1)
    {
        printf("======waiting for client's request======\n");
        //阻塞直到有客户端连接，不然多浪费CPU资源。
        if ((connect_fd = accept(sockfd, (struct sockaddr *)NULL, NULL)) == -1)
        {
            printf("accept socket error: %s(errno: %d)", strerror(errno), errno);
            continue;
        }
        else
        {
            printf("Connect success\n");
        }
        cnt++;
        bzero(buf, sizeof(buf));
        bzero(fileName, sizeof(fileName));
        //得到回传的文件名
        int recv_len = recv(connect_fd, buf, BUFFER_SIZE, 0);
        if (recv_len < 0)
            printf("Error occur!\n");
        strncpy(fileName, buf, strlen(buf));
        fileName[strlen(buf)] = '\0';
        printf("fileName=%s\n", fileName);
        //创建以outputFilename为名的文件，并接收文件。
        FILE *out = fopen(argv[4], "wb");
        int count = 0, length = 0;
        while (length = recv(connect_fd, buf, BUFFER_SIZE, 0))
        {
            if (length < 0)
            {
                printf("Recieve Data From Server  Failed!\n");
                break;
            }
            count += length;
            int write_len = fwrite(buf, sizeof(char), length, out);
            if (write_len < length)
            {
                printf("File Write Failed!\n");
                break;
            }
            bzero(buf, BUFFER_SIZE);
        }
        //写入文件
        fclose(out);
        //为了跳出while(1)循环，如果接收完了，则跳出，否则一直等待。
        if (cnt > 0)
            break;
    }
    return 0;
}

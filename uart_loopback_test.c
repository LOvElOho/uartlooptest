/*********************************************************************************
 *      Copyright:  (C) 2019 wujinlong<547124558@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  serial_com.c
 *    Description:  This file Achieve serial communication
 *                 
 *        Version:  1.0.0(17/05/19)
 *         Author:  wujinlong<547124558@qq.com>
 *      ChangeLog:  1, Release initial version on "17/05/19 15:57:27"
 * 	感谢 wujinlong 原作者
 *      在原来基础上更新通过参数的形式配置 UART节点 以及需要发送的数据 20211020 
 *	arm-linux-gnueabihf-gcc  -o uart_loopback_android uart_loopback_test.c -static 
 *	arm-linux-gnueabihf-gcc  -o uart_loopback_linux uart_loopback_test.c
 *	./uart_loopback   /dev/ttyUSB*  senddata
 *               
 ********************************************************************************/

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<errno.h>
#include <strings.h>
#include <string.h>

int main(int argc,char *argv[])
{
    struct termios old_term,new_term;//该结构体控制驱动程序控制字符的输入
    /*struct termios
     * {unsigned short c_iflag;  输入模式标志
     * unsigned short c_oflag;  输出模式标志
     * unsigned short c_cflag;  控制模式标志
     * unsigned short c_lflag; 区域模式标志或本地模式标志或局部模式
     * unsigned char c_line; 行控制line discipline 
     * unsigned char c_cc[NCC];  控制字符特性,所有可以更改的特殊字符
     * }; */
    //char *serialport_path = "/dev/ttyUSB0";
    //打开串口
    int serial_fd = -1;
    printf("uartname  %s \n",argv[1]);
   
    /* O_NDELAY 函数使read函数在端口没有字符存在的情况下，立刻返回0,与 fcntl(fd,F_SETFL,FNDELAY);一样 */
   if((serial_fd = open(argv[1],O_RDWR | O_NOCTTY | O_NDELAY)) < 0)
   {
       printf("open a serialport failure:%s\n",strerror(errno));
       return -1;
   }
    if(isatty(serial_fd) == 0)
    {
        printf("open fd is not a terminal device\n");
        return -1;
    }
        if(tcgetattr(serial_fd,&old_term) < 0)
    {
        printf("tcgetattr failure:%s\n",strerror(errno));
        return -1;
    }
    bzero((void *)&new_term,sizeof(new_term));
        new_term.c_cflag |= CLOCAL | CREAD;//用于本地连接和接收使用
    //new_term.c_lflag |= ICANON;//规范输入
    //数据位
    new_term.c_cflag &= ~CSIZE;//设置数据位前必须先使用CSIZE做位屏蔽
    new_term.c_cflag |= CS8;
    new_term.c_cflag &= ~CRTSCTS;//无硬件流控 
    //奇偶校验
    new_term.c_cflag |= PARENB;//启用奇偶检验
    new_term.c_iflag |= INPCK;//打开输入奇偶校验
    new_term.c_cflag &= ~PARODD;//偶检验
    //new_term.c_iflag |= IGNPAR;//无奇偶检验位 
    new_term.c_oflag = 0; //输出模式 
    new_term.c_lflag = 0; //不激活终端模式 
    //停止位
    new_term.c_cflag &=  ~CSTOPB;
    //波特率
    new_term.c_oflag &= ~OPOST;/*如果不是开发终端之类的，只是串口传输数据，而不需要串口来处理，那么使用原始模式(Raw Mode)方式来通讯。*/
    if(cfsetispeed(&new_term,B115200) < 0)
    {
        printf("cfsetispeed failure:%s\n",strerror(errno));
        return -3;
    }
    if(cfsetospeed(&new_term,B115200) < 0)
    {
        printf("cfsetospeed failure:%s\n",strerror(errno));
        return -3;
    }
    //只有在输出队列为空时才能改变一个终端的属性，所以要用tcflush;
    tcflush(serial_fd,TCIFLUSH);
    if(tcsetattr(serial_fd,TCSANOW,&new_term) != 0)
    {
        printf("tcsetattr failure:%s\n",strerror(errno));
        return -4;
    }
    //tcsetattr函数执行了任意一种所要求的动作，也会返回OK，需调用tcgetaddr检查；
    //char buff[] = "hello\n";
   printf("sendata  %s \n",argv[2]);
    char buf[1024];
    memset(&buf,0,sizeof(buf));
    fcntl(serial_fd,F_SETFL,FNDELAY);
    int rv = -1;
    int count=10;
   while(count--){
   if((rv = write(serial_fd,argv[2],strlen(argv[2]))) < 0)
   {
       printf("write failure:%s\n",strerror(errno));
       return -5;
          }
   printf("write %d bytes to serial ok!wait 1s...\n",rv);
   sleep(1);
   if((rv = read(serial_fd,buf,sizeof(buf))) < 0)
   {
       printf("read failure:%s\n",strerror(errno));
       return -5;
   }
   printf("read data from serialport:%s \n",buf);
   }
   tcflush(serial_fd,TCIOFLUSH);
   tcsetattr(serial_fd,TCSANOW,&old_term);

   close(serial_fd);
   return 0;
}
       


/*********************************************************************************
 *      Copyright:  (C) 2023 Hu Ben Yuan<2017603273@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  comport.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(06/23/2023)
 *         Author:  Hu Ben Yuan <2017603273@qq.com>
 *      ChangeLog:  1, Release initial version on "06/23/2023 05:27:55 PM"
 *                 
 ********************************************************************************/


#include "comport.h"

//打开串口文件
int tty_open(comport_tty_t *comport_tty)
{
    if (!comport_tty)
    {
        printf("The argument invalid!\n");
        return -1;
    }

    comport_tty->fd = open(comport_tty->serial_name, O_RDWR|O_NOCTTY|O_NONBLOCK);  //可读可写，非终端，非阻塞。
    if (comport_tty->fd < 0)
    {
        printf("Open serial file failure:%s\n", strerror(errno));
        return -2;
    }

    if (!isatty(comport_tty->fd))
    {
        printf("%s is not a terminal equipment!\n", comport_tty->serial_name);
        return -3;
    }

    printf("Open %s successfully!\n", comport_tty->serial_name);
    return 0;
}


//关闭串口文件
int tty_close(comport_tty_t *comport_tty)
{
    int retval = -1;

    if (!comport_tty)
    {
        printf("The argument invalid!\n");
        return -1;
    }

    retval = tcflush(comport_tty->fd, TCIOFLUSH);//清空输入输出
    if (retval < 0)
    {
        printf("Failed to clear the input/output buffer:%s\n", strerror(errno));
        return -2;
    }
    
    retval = tcsetattr(comport_tty->fd, TCSANOW, &(comport_tty->old_termios));//恢复串口原始属性，TCSANOW:不等数据传输完毕
    if (retval < 0)
    {
        printf("Set old termios failure:%s\n", strerror(errno));
        return -3;
    }

    close(comport_tty->fd);
    
    printf("Excute tty_close() successfully!\n");

    return 0;
}


//初始化串口
int tty_init(comport_tty_t *comport_tty)
{
    int     retval = -1;
    char    baudrate_buf[32] = {0};
    struct  termios new_termios;

    if (!comport_tty)
    {
        printf("The argument invalid!\n");
        return -1;
    }

    memset(&new_termios, 0, sizeof(new_termios));
    memset(&(comport_tty->old_termios), 0, sizeof(comport_tty->old_termios));

    retval = tcgetattr(comport_tty->fd, &(comport_tty->old_termios));  //获取与终端相关的参数
    if (retval < 0)
    {
        printf("Failed to obtain the current serial port properties!\n");
        return -2;
    }

    retval = tcgetattr(comport_tty->fd, &new_termios);
    if (retval < 0)
    {
        printf("Failed to obtain the current serial port properties!\n");
        return -3;
    }

    new_termios.c_cflag |= CLOCAL;//忽略解调器线路状态

    new_termios.c_cflag |= CREAD;

    new_termios.c_cflag &= ~CSIZE;//启动接收器，能够从串口中读取输入数据

    new_termios.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    /* 
     * ICANON: 标准模式
     * ECHO: 回显所输入的字符
     * ECHOE: 如果同时设置了ICANON标志，ERASE字符删除前一个所输入的字符，WERASE删除前一个输入的单词
     * ISIG: 当接收到INTR/QUIT/SUSP/DSUSP字符，生成一个相应的信号
     *
     * */

    new_termios.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    /* 
     * BRKINT: BREAK将会丢弃输入和输出队列中的数据(flush)，并且如果终端为前台进程组的控制终端，则BREAK将会产生一个SIGINT信号发送到这个前台进程组
     * ICRNL: 将输入中的CR转换为NL
     * INPCK: 允许奇偶校验
     * ISTRIP: 剥离第8个bits
     * IXON: 允许输出端的XON/XOF流控
     *
     * */

     /* OPOST: 表示处理后输出，按照原始数据输出 */ 
    new_termios.c_oflag &= ~(OPOST);

    if (comport_tty->baudrate)
    {
        snprintf(baudrate_buf, sizeof(baudrate_buf), "B%d", comport_tty->baudrate);
        cfsetispeed(&new_termios, (intptr_t)baudrate_buf);//输入波特率
        cfsetospeed(&new_termios, (intptr_t)baudrate_buf);//输出波特率
    }
    else
    {
        cfsetispeed(&new_termios, B115200);//默认波特率
        cfsetospeed(&new_termios, B115200);
    }

    switch (comport_tty->databits)//数据位
    {
        case 5:
        {
            new_termios.c_cflag |= CS5;//5位
            break;
        }

        case 6:
        {
            new_termios.c_cflag |= CS6;//6位
            break;
        }

        case 7:
        {
            new_termios.c_cflag |= CS7;//7位
            break;
        }

        case 8:
        {
            new_termios.c_cflag |= CS8;//8位
            break;
        }

        default:
        {
            new_termios.c_cflag |= CS8;//默认
            break; 
        }
    }

#if 1
    switch(comport_tty->parity)//奇偶校验位
    {
        case 'n':
        case 'N':
        {
            new_termios.c_cflag &= ~PARENB;//无校验
            break;
        }

        case 'o':
        case 'O':
        {
            new_termios.c_cflag |= (PARODD | PARENB);//奇校验
            break;
        }

        case 'e':
        case 'E':
        {
            new_termios.c_cflag &= ~PARENB;//偶校验
            new_termios.c_cflag &= ~PARODD;
        }

        case 'b':
        case 'B':
        {
            new_termios.c_cflag &= ~PARENB;
            new_termios.c_cflag &= ~CSTOP;//空格
        }

        default:
        {
            new_termios.c_cflag &= ~PARENB;//默认无校验
            break;
        }
    }

    switch(comport_tty->stopbits)//停止位
    {
        case 1:
        {
            new_termios.c_cflag &= ~CSTOPB;//1位停止位
            break;
        }

        case 2:
        {
            new_termios.c_cflag |= CSTOPB;//2位停止位
            break;
        }

        default:
        {
            new_termios.c_cflag &= ~CSTOPB;//默认1位停止位
            break;
        }
    }
#endif


    //MIN =0 TIME =0 时，如果有数据可读，则read最多返回所要求的字节数，如果无数据可用，则read立即返回0；
    new_termios.c_cc[VTIME] = 0;
    new_termios.c_cc[VMIN] = 0;

    comport_tty->msend_len = 128;//最长数据发送长度

    retval = tcflush(comport_tty->fd, TCIOFLUSH);//清空输入输出
    if (retval < 0)
    {
        printf("Failed to clear the input/output buffer:%s\n", strerror(errno));
        return -3;
    }

    retval = tcsetattr(comport_tty->fd, TCSANOW, &new_termios);//启用新的串口文件属性
    if(retval < 0)
    {
        printf("Failed to set new properties of the serial port:%s\n", strerror(errno));
        return -4;
    }

    printf("Successfully set new properties of the serial port!\n");
    return 0;
}


//向串口发送数据
int tty_send(comport_tty_t *comport_tty, char *send_buf, int sbuf_len)
{
    int     retval = -1;
    int     write_rv = 0;
    char   *ptr = NULL; 
	char   *end = NULL;


    if (!comport_tty || !send_buf || (sbuf_len < 0))
    {
        printf("[tty_send] The argument invalid!\n");
        return -1;
    }
	
	//对能发送的最长的信息和需要发送信息的长度作比较
    if (sbuf_len > comport_tty->msend_len)
    {
        ptr = send_buf;
        end = send_buf + sbuf_len;
        do
        {   
            if(comport_tty->msend_len <(end-ptr))
            {
                retval = write(comport_tty->fd, ptr, comport_tty->msend_len);

                if ((retval <= 0) || (retval != comport_tty->msend_len))
                {
                    printf("[tty_send] Write data to fd[%d] failure:%s\n",comport_tty->fd, strerror(errno));
                    return -2;
                }

                write_rv += retval;
                ptr += comport_tty->msend_len;
            }
            else
            {
                retval = write(comport_tty->fd, ptr, (end - ptr));

                if ((retval <= 0) || (retval != (end - ptr)))
                {
                    printf("[tty_send] Write data to fd[%d] failure:%s\n",comport_tty->fd, strerror(errno));
                    return -3;
                }

                write_rv += retval;
                ptr += (end - ptr);
            }
            
        }while(ptr < end);
        
    }
    else
    {
        retval = write(comport_tty->fd, send_buf, sbuf_len);
        if((retval <= 0) || (retval != sbuf_len))
        {
            printf("Write data to fd[%d] failure:%s\n",comport_tty->fd, strerror(errno));
            return -4;
        }
        write_rv += retval;
    }

    printf("[tty_send]send_buf:%s\n", send_buf);
    printf("[tty_send]write_rv: %d\n", write_rv);

    return write_rv;
}


//这里不固定接收buffer的大小，因为不同的AT指令返回的字符串大小不一样，如果要查看所有SMS信息的话，就可能很大
int tty_recv(comport_tty_t *comport_tty, char *recv_buf, int rbuf_len, int timeout)
{
    int     read_rv = -1;
    int     rv_fd = -1;
    fd_set  rdset;
    struct  timeval time_out;

	memset(&time_out, 0,sizeof(time_out)); 
    if (!comport_tty || (rbuf_len < 0))
    {
        printf("[tty_recv]The argument invalid!\n");
        return -1;
    }
    
    if (timeout)
    {
		time_out.tv_sec = (time_t)timeout;
		time_out.tv_usec = 0;

        FD_ZERO(&rdset);
        FD_SET(comport_tty->fd, &rdset);

        rv_fd = select(comport_tty->fd + 1, &rdset, NULL, NULL, &time_out);
        if(rv_fd < 0)
        {
            printf("[tty_recv]Select() listening for file descriptor error: %s!\n", strerror(errno));
            return -2;
        }
        else if(rv_fd == 0)
        {
            printf("[tty_recv]Select() listening for file descriptor timeout!\n");
            return -3;
        }
    }

    read_rv = read(comport_tty->fd, recv_buf, rbuf_len);
    if (read_rv <= 0)
    {
        printf("[tty_recv]Read data from fd[%d] failure:%s\n", comport_tty->fd, strerror(errno));
        return -4;
    }
    
    printf("[tty_recv]recv_buf:%s\n",recv_buf);
    
    return read_rv;
}


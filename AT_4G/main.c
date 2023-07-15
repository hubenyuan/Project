/*********************************************************************************
 *      Copyright:  (C) 2023 Hu Ben Yuan<2017603273@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  main.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(02/07/23)
 *         Author:  Hu Ben Yuan<2017603273@qq.com>
 *      ChangeLog:  1, Release initial version on "02/07/23 10:48:40"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <signal.h>

#include "comport.h"
#include "atcmd.h"

#define CONFIG_DEBUG

#ifdef  CONFIG_DEBUG
#define dbg_print(format, args...) printf(format, ##args)
#else 
#define dbg_print(format, args...) do {} while(0)
#endif

#define    TIMEOUT  2


void print_usage(char *program_name);

void sigusr1_handler(int signum);

void sigusr2_handler(int signum);

void install_signal(void);

void handler(int sig);


int g_stop = 0;


int main(int argc, char *argv[])
{
    
    int             i;
    int             ch;
    int             rv = - 1;
    int             rv_fd = -1;
    char            send_buf[128];
    char            recv_buf[128];
    fd_set          rdset;
	pid_t           pid;
	pid          =  fork();
    comport_tty_t   comport_tty;
	comport_tty_t  *comport_tty_ptr;
	comport_tty_ptr = &comport_tty;

    struct option opts[] = {
        {"baudrate", required_argument, NULL, 'b'},
        {"databits", required_argument, NULL, 'd'},
        {"parity", required_argument, NULL, 'p'},
        {"stopbits", required_argument, NULL, 's'},
        {"serial_name", required_argument, NULL, 'm'},
        {"help", no_argument, NULL, 'h'},
        {0,0,0,0}
    };

    while((ch = getopt_long(argc, argv, "b:d:p:s:m:h", opts, NULL)) != -1)
    {
        switch(ch)
        {
            case 'b':
            {
                comport_tty_ptr->baudrate = atoi(optarg);
                break;
            }
            case 'd':
            {
                comport_tty_ptr->databits = atoi(optarg);
                break;
            }
            case 'p':
            {
                comport_tty_ptr->parity = optarg[0];
                break;
            }
            case 's':
            {
                comport_tty_ptr->stopbits = atoi(optarg);
                break;
            }
            case 'm':
            {
                strncpy(comport_tty_ptr->serial_name, optarg, 64);
                break;
            }
            case 'h':
            {
                print_usage(argv[0]);
                return 0;
            }
            default:
            {
                printf("input invalid argument!\n");
                return -1;
            }
        }
    }
    
	if(pid < 0)
	{
		printf("fork() create child process failure: %s\n", strerror(errno));
        return -1;
    }

    if(pid == 0)
    {
        //注册信号处理函数
        signal(SIGUSR1, sigusr1_handler);
        signal(SIGUSR2, sigusr2_handler);
        dbg_print("Child process PID[%d] running...\n", getpid());
        while(1)
        {
            sleep(1);
        }
    }

	else
	{
        dbg_print("Parent process PID[%d] running...\n", getpid());

		if(0 == strlen(comport_tty_ptr->serial_name))
		{
			printf("Failed to obtain the device name!\n");
			return -2;
		}

		install_signal();

		if(tty_open(comport_tty_ptr) < 0)
		{
			printf("Failed to open the device file");
			return -3;
		}

		if(tty_init(comport_tty_ptr) < 0)
		{
			printf("Failed to initialize the serial port\n");
			return -4;
		}

		if(check_sim_all(comport_tty_ptr) < 0)
		{
			printf(" SIM initialize failure!\n");
			return -5;
		}

		while(1)
		{
			FD_ZERO(&rdset);//清空文件描述符集合
			FD_SET(comport_tty_ptr->fd, &rdset);//将串口文件fd加入集合
			FD_SET(STDIN_FILENO, &rdset);//将标准输入文件fd加入集合
			//select多路复用非阻塞监听文件描述符
			rv_fd = select(comport_tty_ptr->fd + 1, &rdset, NULL, NULL, NULL);
			if(rv_fd < 0)
			{
				printf("Select listening for file descriptor error: %s\n",strerror(errno));
				rv = -6;
				goto CleanUp;
			}
			else if(0 == rv_fd)
			{
				printf("Select listening for file descriptor timeout!\n");
				rv = -7;
				goto CleanUp;
			}
			else
			{
				if(FD_ISSET(STDIN_FILENO, &rdset))//判断是否是标准输入响应
				{
					memset(send_buf, 0, sizeof(send_buf));//清空buffer

					fgets(send_buf, sizeof(send_buf), stdin);
					i = strlen(send_buf);
					strcpy(&send_buf[i-1], "\r");//发送AT指令时，需要在指令后面加上\r
					if(tty_send(comport_tty_ptr, send_buf, strlen(send_buf)) < 0)
					{
						printf("Failed to send data through the serial port\n");
						rv = -8;
						goto CleanUp;
					}
					dbg_print("Send data:%s\n", send_buf);
					fflush(stdin);//冲洗输入流
				}
				else if(FD_ISSET(comport_tty_ptr->fd, &rdset))//判断是否是串口文件描述符响应
				{
					memset(recv_buf, 0, sizeof(recv_buf));
					//读串口发来的信息
					if(tty_recv(comport_tty_ptr, recv_buf, sizeof(recv_buf), TIMEOUT) < 0)
					{
						printf("Failed to receive serial port data!\n");
						rv = -9;
						goto CleanUp;
					}
					printf("Receiving port data:%s\n", recv_buf);
					fflush(stdout);//冲洗输出流
				}
			}
		}
	}

    return 0;
CleanUp: 
    tty_close(comport_tty_ptr);
    return rv;
}


//打印帮助信息
void print_usage(char *program_name)
{
    printf("Usage:%s[OPTION]\n\n", program_name);
    printf("-b[baudrate]:Select baud rate, for example 115200 and 9600.\n");
    printf("-p[parity]:Select parity check, for example n N e E o O.\n");
    printf("-s[stopbits]:Select stop bit, for example 1 and 2.\n");
    printf("-m[serial_name]:Select device file, for example /dev/ttyUSB0.\n");
    printf("-h[help]:Printing Help Information.\n"); 
    printf("For example:./SMS -b 115200 -p n -s 1 -m /dev/ttyUSB3 \n\n");

}

void sigusr1_handler(int signum)
{
    printf("Received SIGUSR1. Starting pppd...\n");
    system("sudo pppd call rasppp");
}

void sigusr2_handler(int signum)
{
    printf("Received SIGUSR2. Stopping pppd...\n");
    system("sudo poff rasppp");
    exit(0);
}

//对信号进行处理
void handler(int sig)
{
    switch(sig)
    {
        case SIGINT:
        {
            printf("Process captured SIGINT signal!\n");
            g_stop = 1;
            break;
        }
        case SIGTERM:
        {
            printf("Process captured SIGTERM signal!\n");
            g_stop = 1;
            break;
        }
        case SIGSEGV:
        {
            printf("Process captured SIGSEGV signal!\n");
            g_stop = 1;
            exit(0);
            break;
        }
        case SIGPIPE:
        {
            printf("Process captured SIGPIPE signal!\n");
            g_stop = 1;
            break;
        }
        default:
            break;
    }

    return ;
}

//注册信号
void install_signal(void)
{
    struct sigaction sigact;

    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigact.sa_handler = handler;

    sigaction(SIGINT, &sigact, 0);
    sigaction(SIGTERM, &sigact, 0);
    sigaction(SIGPIPE, &sigact, 0);
    sigaction(SIGSEGV, &sigact, 0);

    return ;
}


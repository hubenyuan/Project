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

int g_stop = 0;
pid_t  fork_pid1;

void print_usage(char *program_name);
void sig_handle(int signum);
void sigusr1_handler(int signum);
void sigusr2_handler(int signum);
void exit_handler();


int main(int argc, char *argv[])
{
    
    int             i;
	int             shmid;
    int             ch;
    int             rv = - 1;
    int             rv_fd = -1;
	int             serial_fd;
    char            send_buf[128];
    char            recv_buf[128];
	int             sim_signal;
    fd_set          rdset;
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
    

	if(0 == strlen(comport_tty_ptr->serial_name))
	{
		printf("Failed to obtain the device name!\n");
		return -2;
	}
	
	//注册信号
	signal(SIGINT,sig_handle);
	signal(SIGTERM,sig_handle);
	signal(SIGINT,exit_handler);

	//打开串口
	if(tty_open(comport_tty_ptr) < 0)
	{
		printf("Failed to open the device file");
		return -3;
		goto CleanUp;
	}

	//串口初始化
	if(tty_init(comport_tty_ptr) < 0)
	{
		printf("Failed to initialize the serial port\n");
		return -4;
		goto CleanUp;
	}

	//检测串口是否打开，SIM卡在不在，有没有注册网络，
	if(check_sim_all(comport_tty_ptr) < 0)
	{
		printf("SIM Card don't get ready\n");
		return -5;
		goto CleanUp;
	}

    //创建子进程
	fork_pid1 = fork();
	if(fork_pid1 < 0)
	{
		printf("Creat process ONE failure\n");
		return -1;
	}

	else if(fork_pid1 == 0)
	{
		printf("Procession One PID: %d\n",getpid());
		signal(SIGUSR1, sigusr1_handler);
		signal(SIGUSR2, sigusr2_handler);

		while(1)
		{
			sleep(1);

		}	
		exit(0);
	}

	else
	{
		printf("Process Parent PID: %d\n",getpid());
		sleep(3);

		while(1)
		{
			//获取SIM Card信号强度
			if(check_sim_signal(comport_tty_ptr,&sim_signal) < 0)
			{
				printf("SIM Card signal instability!\n");
				return -6;
				goto CleanUp;
			}

			dbg_print("SIM Card Signal is: %d\n",sim_signal);

			if(sim_signal>7 && sim_signal<32)
			{
				printf("The signal is good, Start pppd dial\n");
				//kill(fork_pid2, SIGUSR1);
				//system("sudo pppd call rasppp");  //进行pppd拨号上网
			}
			else
			{
				printf("The signal isn't good, Stop pppd dial\n");
				//kill(fork_pid2, SIGUSR2);
				//system("sudo poff rasppp");
			}
			sleep(10);
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


// 信号处理函数，SIGUSR1信号启动pppd拨号，SIGUSR2信号停止pppd拨号
void sigusr1_handler(int signum)
{
    printf("Received SIGUSR1. Starting pppd...\n");
    system("sudo pppd call rasppp");
}

void sigusr2_handler(int signum)
{
    printf("Received SIGUSR2. Stopping pppd...\n");
    system("sudo poff rasppp");
}

//注册进程结束信号，当父进程结束运行时结束子进程一的运行
void exit_handler() 
{
	if(fork_pid1 > 0)
	{
		// 发送SIGKILL信号给子进程一
		kill(fork_pid1, SIGKILL);
	}
	exit(0);
}


//安装信号
void sig_handle(int signum)
{
    printf("catch signal [%d]\n",signum);
    g_stop = 1;
}

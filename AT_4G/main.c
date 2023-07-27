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
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <gpiod.h>

#include "comport.h"
#include "atcmd.h"

#define CONFIG_DEBUG

#ifdef  CONFIG_DEBUG
#define dbg_print(format, args...) printf(format, ##args)
#else 
#define dbg_print(format, args...) do {} while(0)
#endif

#define RED_FONT    "\033[1;31m"
#define GREEN_FONT  "\033[1;32m"
#define DEFAULT_FONT    "\033[0m"

#define GPRS_CFG0_PIN       "PD15"
#define GPRS_CFG1_PIN       "PD17"
#define GPRS_PWREN_PIN      "PA31"


#define    TIMEOUT  2

int  g_stop = 0;
int *p_sign = 0;
pid_t  fork_pid1,pid;

void print_usage(char *program_name);

void install_signal(void);
void handler(int sig);

void sigusr1_handler(int signum);
void sigusr2_handler(int signum);
void exit_handler();

int gprs_check_present(void);
int gprs_turn_power(int status);

int main(int argc, char *argv[])
{
    int             shmid;
    int             ch;
	int             p_status;
    int             rv = - 1;
	int             sim_signal;
    void           *shmaddr;
	char            apn[256];
    comport_tty_t   comport_tty;
    comport_tty_t  *comport_tty_ptr;
    comport_tty_ptr = &comport_tty;
	const char *netPath = "/sys/class/net/ppp0";
	const char *command = "nohup ping baidu.com -I ppp0 -c 4 &";
	struct stat    st;

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
	install_signal();
	signal(SIGINT,exit_handler);

	//检查4G模块是否存在
	gprs_check_present();
	if(status == 0x3)
	{
		printf("Inexistence 4G module!\n");
		return -3;
	}

	//给4G模块上电
	if(gprs_turn_power(&status) < 0)
	{
		printf("The module fails to be powered on\n");
		return -4;
	}

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

	//检测串口是否打开，SIM卡在不在，有没有注册网络，信号行不行
	if(check_sim_all(comport_tty_ptr,&sim_signal) < 0)
	{
		printf("SIM Card don't get ready\n");
		return -5;
		goto CleanUp;
	}

	/*
	if(check_sim_apn(comport_tty_ptr,apn) < 0)
	{
		printf("Can't gain APN");
		return -6;
	}
	*/

    // 创建共享内存段
    shmid = shmget(IPC_PRIVATE, 128, IPC_CREAT | 0666);
    if (shmid < 0) 
	{
        perror("shmget");
        exit(1);
    }

    // 连接共享内存
    shmaddr = shmat(shmid, NULL, 0);
    if (shmaddr == (void *)-1) 
	{
        perror("shmat");
        exit(1);
    }

	p_sign = (int *)shmaddr;

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
		printf("Listening SIGUSR1 and SIGUSR2 Signal!\n");

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

		while(!g_stop)
		{
			if(! *p_sign)
			{
				//判断是否存在ppp0网络,stat等于0表示存在
				if(stat(netPath, &st) == 0)
				{
					printf("ppp0 network interface exists.\n");
					//检测ppp0能不能ping通
					p_status = system(command);
					if(WIFEXITED(p_status) && WEXITSTATUS(p_status) == 0)
					{
						printf("ppp0 network is good, No data loss\n");
					}

					else
					{
						printf("ppp0 network is poor, closs ppp0");
						system("sudo poff rasppp"); //停止pppd拨号上网
					}
				}
				
				else
				{
					printf("ppp0 network interface does not exist.\n");
					//获取SIM Card信号强度
					if(check_sim_signal(comport_tty_ptr,&sim_signal) < 0)
					{
						printf("SIM Card signal instability!\n");
					}

					dbg_print("SIM Card Signal is: %d\n",sim_signal);

					//SIM Card 信号在8~31是正常的状态,数值越高信号越好
					if(sim_signal>7 && sim_signal<32)
					{
						printf("--The signal is good, Start pppd dial--\n");
	                    system("nohup sudo pppd call rasppp &");    //开始pppd拨号上网
					}
					else
					{
						printf("The signal isn't good, Can't pppd dial\n"); //不能pppd拨号上网
					}
				}

			}

			else
			{
				printf("--Waiting USR1 signal arrval--\n");
			}

			sleep(5);
		}

        // 分离共享内存
        if (shmdt(p_sign) == -1) 
		{
            perror("shmdt");
            exit(1);
        }

        // 删除共享内存
        if (shmctl(shmid, IPC_RMID, 0) == -1) 
		{
            perror("shmctl");
            exit(1);
        }

		wait(NULL);  // 等待子进程结束
		
		return 0;
	}

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
    printf("For example:./ppp-4G -b 115200 -p n -s 1 -m /dev/ttyUSB3. \n");

}


// 信号处理函数，SIGUSR1信号启动pppd拨号，SIGUSR2信号停止pppd拨号
void sigusr1_handler(int signum)
{
    printf("Received SIGUSR1. Starting pppd...\n");
	*p_sign = 0;
}

void sigusr2_handler(int signum)
{
    printf("Received SIGUSR2. Stopping pppd...\n");
	system("sudo poff rasppp"); //停止pppd拨号
	*p_sign = 1;
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
		case SIGUSR1:
		{
		     printf("Process captured SIGUSR1 signal!\n");
			 break;
		}
		case SIGUSR2:
		{
			printf("Process captured SIGUSR2 signal!\n");
			break;
		}
        default:
            break;
    }

    return ;
}

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
    sigaction(SIGUSR1, &sigact, 0);
    sigaction(SIGUSR2, &sigact, 0);

    return ;
}

/*  check 4G module present or not */
int gprs_check_present(void)
{
    int       status = 0x3; /* 00:4G 01/10:Reserved 11:NoCard */
    char      bit[2];

    if( (bit[0]=at91_gpio_get(GPRS_CFG0_PIN)) < 0)
    {
        goto out;
    }

    if( (bit[1]=at91_gpio_get(GPRS_CFG1_PIN)) < 0)
    {
        goto out;
    }

    status = bit[1]<<1 | bit[0];

out:
    return status;
}
                  
                  
/*  Turn 4G module power stauts: ON or OFF  */
int gprs_turn_power(int status)
{
    if( status )
        return at91_gpio_set(GPRS_PWREN_PIN, LOWLEVEL);
    else
        return at91_gpio_set(GPRS_PWREN_PIN, HIGHLEVEL);
}

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
#include <pthread.h>

#include "comport.h"
#include "atcmd.h"
#include "network.h"
#include "gpio_l.h"

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

int   g_stop = 0;
int   led_sign = 0;
int   led_symbol = 0;
int   p_sign = 0;
int   ptd_sign = 0;

gpiod_led_t  gpiod_led;

void print_usage(char *program_name);

void install_signal(void);
void handler(int sig);

void sigusr1_handler(int signum);
void sigusr2_handler(int signum);

void *thread_worker1(void *args);
void *thread_worker2(void *args);
void *thread_worker3(void *args);


int main(int argc, char *argv[])
{
    int             shmid;
    int             ch;
	int             p_status;
    int             rv = - 1;
	int             sim_signal;
	int             jud_net;
    void           *shmaddr;
	char            apn[256];
	pthread_t       tid;
	pthread_attr_t  thread_attr;
    comport_tty_t   comport_tty;
    comport_tty_t  *comport_tty_ptr;
    comport_tty_ptr = &comport_tty;
	const char *netPath = "/sys/class/net/ppp0";
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
	signal(SIGUSR2, sigusr2_handler);
	signal(SIGUSR1, sigusr1_handler);
	
	//初始化GPIO
	gpio_init(&gpiod_led);
	//点亮灯
	led_bright(&gpiod_led);

	//打开串口
	if(tty_open(comport_tty_ptr) < 0)
	{
		printf("Failed to open the device file\n");
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
	printf("Process PID: %d\n",getpid());
    
	if( pthread_attr_init(&thread_attr) )
	{
			printf("pthread_attr_init() failure: %s\n", strerror(errno));
			return -1;
			goto CleanUp;
	}

	if( pthread_attr_setstacksize(&thread_attr, 120*1024) )
	{
			printf("pthread_attr_setstacksize() failure: %s\n", strerror(errno));
			return -1;
			goto CleanUp; 
	}

	if( pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED) )
	{
			printf("pthread_attr_setdetachstate() failure: %s\n", strerror(errno));
			return -1;
			goto CleanUp;
	}


	while(!g_stop)
	{
		if(p_sign)
		{
			ptd_sign = 0;
			//判断是否存在ppp0网络,stat等于0表示存在
			if(stat(netPath, &st) == 0)
			{
				printf("ppp0 network interface exists.\n");
				//检测ppp0能不能ping通
				if(jud_ppp0_net() == 1)
				{
					if(led_sign == 1)
					{
						pthread_attr_destroy(&thread_attr);
						pthread_create(&tid, &thread_attr, thread_worker1, &led_symbol);
						led_sign = 0;
					}
					printf("ppp0 network is good, No data loss\n");
				}

				else
				{
					printf("ppp0 network is poor, closs ppp0");
					led_bright(&gpiod_led);
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
					if(led_sign == 0)
					{
						pthread_attr_destroy(&thread_attr);
						pthread_create(&tid, NULL, thread_worker2, &led_symbol);
						led_sign = 1;
					}

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
			if(!ptd_sign)
			{
			    pthread_attr_destroy(&thread_attr);
			    pthread_create(&tid, NULL, thread_worker3, &led_symbol);
				ptd_sign = 1;
		    }
			led_bright(&gpiod_led);
			printf("--Waiting USR1 signal arrval--\n");
		}

		sleep(5);
	}

	pthread_join(tid, NULL);  //等待线程退出
	led_close(&gpiod_led);
	tty_close(comport_tty_ptr);

	return 0;

CleanUp: 
    tty_close(comport_tty_ptr);
	led_close(&gpiod_led);
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
	p_sign = 1;
}

void sigusr2_handler(int signum)
{
    printf("Received SIGUSR2. Stopping pppd...\n");
	system("sudo poff rasppp"); //停止pppd拨号
	p_sign = 0;
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

    return ;
}


//led 慢闪
void *thread_worker1(void *args)
{

	int *ptr = (int *)args;
	
	if(args < 0)
	{
		printf("%s() get invalid arguments\n", __FUNCTION__);
		pthread_exit(NULL);
	}
	printf("Thread workder 1 [%ld] start running...\n", pthread_self());

	*ptr = 1;

	led_slow(&gpiod_led,ptr);

}


//led 快闪
void *thread_worker2(void *args)
{
	int *ptr = (int *)args;

	if(args < 0)
	{
		printf("%s() get invalid arguments\n", __FUNCTION__);
		pthread_exit(NULL);
	}
	printf("Thread workder 2 [%ld] start running...\n", pthread_self());

	*ptr = 0;

	led_fast(&gpiod_led,ptr);

}

void *thread_worker3(void *args)
{
	int *ptr = (int *)args;

	if(args < 0)
	{
		printf("%s() get invalid arguments\n", __FUNCTION__);
		pthread_exit(NULL);
	}

	printf("Thread workder 3 [%ld] start running...\n", pthread_self());


	*ptr = 2;
}

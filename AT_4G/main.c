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
pid_t      fork_pid1,fork_pid2;

void print_usage(char *program_name);
void install_signal(void);
void handler(int sig);
void sigusr1_handler(int signum);
void sigusr2_handler(int signum);

typedef struct 
{
	int value;
} shared_data;

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
	int             status;
	int             sim_signal;
	pid_t           pid;
    fd_set          rdset;
	shared_data    *data;
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

	//创建共享内存
	shmid = shmget(IPC_PRIVATE, sizeof(shared_data), IPC_CREAT | 0666);
	if(shmid < 0)
	{
		printf("Creat failure\n");
		exit(0);
	}

	// 将共享内存映射到当前进程的地址空间
	data = (shared_data *)shmat(shmid, NULL, 0);
	if(data == (shared_data *)(-1))
	{
		printf("shmat");
		exit(-1);
	}

	//安装信号
	install_signal();

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

    //创建子进程一
	fork_pid1 = fork();
	if(fork_pid1 < 0)
	{
		printf("Creat process ONE failure\n");
		return -1;
	}

	if(fork_pid1 == 0)
	{
		//安装信号
		install_signal();
		printf("Child process One: Read value as %d\n", data->value);
		data->value *= 2;
		printf("Child process One: Doubled value is %d\n", data->value);

		printf("Procession One PID: %d\n",getpid());

		while(1)
		{

			if(check_sim_all(comport_tty_ptr) < 0)
			{
				printf("SIM Card don't get ready\n");
				return -2;
			}

			if(check_sim_signal(comport_tty_ptr,&sim_signal) < 0)
			{
				printf("SIM Card signal instability!\n");
				return -3;
			}

			dbg_print("SIM Card Signal is: %d\n",sim_signal);

			//判断SIM Card信号强度 
			if(sim_signal>7 && sim_signal<32)
			{
				printf("The signal is good, Start pppd dial\n");
				//kill(getppid(), SIGUSR1);
				//system("sudo pppd call rasppp");  //进行pppd拨号上网
			}
			else
			{
				printf("The signal isn't good, Stop pppd dial\n");
				kill(getppid(), SIGUSR2);
				//system("sudo poff rasppp");      // 停止pppd拨号信号
			}
			sleep(100);
		}	
		tty_close(comport_tty_ptr);
		return 0;
	}
 
	else if(fork_pid1 > 0)
	{
		//创建子进程二
		fork_pid2=fork();
		if(fork_pid2 < 0)
		{
			printf("Creat process Two failure\n");
			return -3;
		}
		
		if(fork_pid2 == 0)
		{
			//安装信号
			install_signal();
			sleep(1);
			printf("Process Two PID: %d\n",getpid());
			printf("Process Tow running...\n");
			//信号SIGUSR1和信号SIGUSR2处理函数
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
			data->value = 256;
			printf("Parent process: Set value to %d\n",data->value);
			printf("Process Parent PID: %d\n",getpid());
			sleep(3);

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
				else if(rv_fd == 0)
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
	}
/*
  // 如果父进程接收到退出信号（例如SIGINT），则向子进程一和子进程二发送SIGTERM信号
	if (SIGINT) 
	{
		kill(fork_pid1,SIGTERM);
		kill(fork_pid2,SIGTERM);
	}
*/

	kill(fork_pid1,SIGKILL);  //结束进程一运行
	waitpid(fork_pid1, NULL, 0);
	kill(fork_pid2,SIGKILL);
	waitpid(fork_pid2, NULL, 0);


    // 解除共享内存的映射
    if (shmdt(data) == -1) 
	{
        printf("shmdt\n");
        exit(1);
    }

    // 删除共享内存
    if (shmctl(shmid, IPC_RMID, 0) == -1) 
	{
        printf("shmctl\n");
        exit(1);
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

    sigaction(SIGINT,  &sigact, 0);
    sigaction(SIGTERM, &sigact, 0);
    sigaction(SIGPIPE, &sigact, 0);
    sigaction(SIGSEGV, &sigact, 0);

    return ;
}



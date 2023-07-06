/*********************************************************************************
 *      Copyright:  (C) 2023 Hu Ben Yuan <2017603273@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  atcmd.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(02/07/23)
 *         Author:  Hu Ben Yuan <2017603273@qq.com>
 *      ChangeLog:  1, Release initial version on "02/07/23 19:50:50"
 *                 
 ********************************************************************************/

#include <stdlib.h>
#include "atcmd.h"

#include "comport.h"


int send_recv_atcmd(comport_tty_t *comport_tty,char *atcmd,char *expect_recv,char *rmsg,int msgsize,int timeout)
{
    int     rv = 0;
    char    temp_msg[512] = {0};
	timeout  = 10;

    if(!atcmd || !comport_tty)
    {
        printf("Unable to send AT commond,Invalid input arguments\n");
        return -1;   
    }

    rv = tty_send(comport_tty,atcmd,strlen(atcmd));
    if(rv < 0)
    { 
        printf("Send AT cmd failure and rv: %d\n",rv);
        return -2;
    }

    usleep(10000);

    rv = tty_recv(comport_tty,temp_msg,sizeof(temp_msg),timeout);
    if(rv < 0)
    {
        printf("Receive AT response failure and rv: %d\n",rv);
        return -3;
    }


    if(!strstr(temp_msg,expect_recv))
    {
        printf("temp_msg: %s",temp_msg);
        printf("recive AT response have not expect data and not in recive AT response\n");
        return -4;
    }

    if(rmsg)
    {
        strncpy(rmsg,temp_msg,msgsize);
    }

    return 0;
}

//发送指令为AT,期望收到OK，检测串口能否通信
int check_sim_ready(comport_tty_t *comport_tty)
{
    int             rv = 0;

    if(!comport_tty)
    {
        printf("check_serial_ready Invalid input arguments\n");
        return -1;
    }

    rv = send_recv_atcmd(comport_tty,"AT\r","OK",NULL,0,2);

    if(rv < 0)
    {
        printf("send_resp_serial() of failure and rv: %d\n",rv);
        return -2;
    }

    return 0;
}

//发送指令AT+CIMI，检查SIM卡是否读卡成功，返回OK表示成功
int check_sim_cimi(comport_tty_t *comport_tty)
{
	int         rv = 0;

	if(!comport_tty)
	{
		printf("check_serial_ready Invalid input arguments\n");
		return -1;
	}

	rv = send_recv_atcmd(comport_tty,"AT+CIMI\r","OK",NULL,0,2);

	if(rv < 0)
	{
		printf("send_resp_serial() of failure and rv: %d\n",rv);
		return -2;
	}

	return 0;
}


//发送指令AT+CSCON=0,检查网络信号连接状态，收到OK表示网络连接状态很好
int check_serial_state(comport_tty_t *comport_tty)
{
	int             rv = 0;
	if(!comport_tty)
	{
		printf("check_serial_state Invalid input arguments\n");
		return -1;
	}

	rv = send_recv_atcmd(comport_tty,"AT+CSCON=0\r","OK",NULL,0,2);

	if(rv < 0)
	{
		printf("send_resp_serial() of failure and rv: %d\n",rv);
		return -2;
	}

	return 0;
}

//发送指令AT+CGATT? 回复是1表示已经连上基站
int check_nbiot_cgatt(comport_tty_t *comport_tty)
{
    int             rv = 0;
    if(!comport_tty)
    {   
        printf("check_serial_state Invalid input arguments\n");
        return -1; 
    }   

    rv = send_recv_atcmd(comport_tty,"AT+CGATT?\r","1",NULL,0,2);

    if(rv < 0)
    {   
        printf("send_resp_serial() of failure and rv: %d\n",rv);
        return -2; 
    }   

    return 0;
}

//送命令AT+CGMI，查看产商
int check_sim_cgmi(comport_tty_t *comport_tty)
{
    int             rv = 0;
    if(!comport_tty)
    {   
        printf("check_serial_state Invalid input arguments\n");
        return -1; 
    }   

    rv = send_recv_atcmd(comport_tty,"AT+CGMI\r","",NULL,0,2);

    if(rv < 0)
    {   
        printf("send_resp_serial() of failure and rv: %d\n",rv);
        return -2; 
    }   

    return 0;
}

//发送命令AT+CGSN,查看产品序列号
int check_sim_cgsn(comport_tty_t *comport_tty)
{
    int             rv = 0;
    if(!comport_tty)
    {   
        printf("check_serial_state Invalid input arguments\n");
        return -1; 
    }   

    rv = send_recv_atcmd(comport_tty,"AT+CGSN\r","",NULL,0,2);

    if(rv < 0)
    {   
        printf("send_resp_serial() of failure and rv: %d\n",rv);
        return -2; 
    }   

    return 0;
}


//发送命令ATI，获取产家信息，设备型号，固件版本信息
int check_sim_ati(comport_tty_t *comport_tty)
{
    int             rv = 0;
    if(!comport_tty)
    {   
        printf("check_serial_state Invalid input arguments\n");
        return -1; 
    }   

    rv = send_recv_atcmd(comport_tty,"ATI\r","",NULL,0,2);

    if(rv < 0)
    {   
        printf("send_resp_serial() of failure and rv: %d\n",rv);
        return -2; 
    }   

    return 0;
}

//发送指令为AT+CPIN?,期望收到READY,检测SIM卡是否安装，
int check_sim_exist(comport_tty_t *comport_tty)
{
    int             rv = 0;

    if(!comport_tty)
    {
        printf("exit Invalid input arguments\n");
        return -1;
    }

    rv = send_recv_atcmd(comport_tty,"AT+CPIN?\r","READY",NULL,0,2);
    
	if(rv < 0)
    {
        printf("send_resp_serial() of %s failure and rv: %d\n",__func__,rv);
        return -2;
    }

    return 0;
}

//发送指令为AT+CREG?,期望收到0,1或者0,3.检测SIM卡是否注册上了。
int check_sim_register(comport_tty_t *comport_tty)
{
    int             rv1 = 0;
	int             rv2 = 0;

    if(!comport_tty)
    {
        printf("register Invalid input arguments\n");
        return -1;
    }

    rv1 = send_recv_atcmd(comport_tty,"AT+CREG?\r","0,1",NULL,0,2);
    rv2 = send_recv_atcmd(comport_tty,"AT+CREG?\r","0,3",NULL,0,2);

    if(rv1 && rv2) 
    {
        printf("SIM Card is not regsiter\n");
        return -2;
    }

    return 0;
}

//发送指令AT+CEREG? 回复0,1 表示网络注册成功。
int check_nbiot_register(comport_tty_t *comport_tty)
{
    int             rv = 0;

    if(!comport_tty)
    {   
        printf("register Invalid input arguments\n");
        return -1; 
    }   

    rv = send_recv_atcmd(comport_tty,"AT+CEREG?\r","0,1",NULL,0,2);

    if(rv < 0) 
    {   
        printf("NBIot is not regsiter\n");
        return -2; 
    }   

    return 0;
}

//发送指令为AT+CSQ,期望收到0~31,99。  0~31为信号强度，若信号强度为99则表示无信号，第二位通常为99
int check_sim_signal(comport_tty_t *comport_tty)
{
    int             i = 0;
    int             rv = 0;
    int             signal_strength;
    char            str_signal[8] = {0};
	char            msg[128] = {0};

    if(!comport_tty)
    {
        printf(" signal Invalid input arguments\n");
        return -1;
    }


    rv = send_recv_atcmd(comport_tty,"AT+CSQ\r","+CSQ",msg,sizeof(msg),2);
    
	if(rv < 0) 
    {
        printf("send_resp_serial() of failure and rv: %d\n",rv);
        return -2;
    }


    for(i=0; i < sizeof(msg); i++)
    {
        if(msg[i] == ',')
        {
            if(msg[i-2] == ' ')
                strncpy(str_signal,&msg[i-1],1); // the signal is single-signal, eg: 6,99
            else
                strncpy(str_signal,&msg[i-2],2); // the signal is double-signal, eg: 21,99
            break;
        }
    }

    signal_strength = atoi(str_signal);

    if(signal_strength < 7 || signal_strength > 31)
    {
        printf("sim card signal strength is too low or no signal and signal strength\n");
        return -3;
    }

    return 0;
}

//检测所有发送的指令，若全部满足返回0，否则返回-1，-2···
int check_sim_all(comport_tty_t *comport_tty)
{
	if(check_sim_ready(comport_tty) < 0)
	{
		printf("The serial port is not ready!\n");
		return -1;
	}

	if(check_serial_state(comport_tty) < 0)
	{
		printf("Check signal failure!\n");
		return -2;
	}

	if(check_sim_signal(comport_tty) < 0)
	{
		printf("Can not check signal!\n");
		return -3;
	}

	if(check_sim_cimi(comport_tty) < 0)
	{
		printf("Can not check signal!\n");
		return -4;
	}

	if(check_nbiot_register(comport_tty) < 0)
	{
		printf("Can not check signal!\n");
		return -5;
	}

	if(check_nbiot_cgatt(comport_tty) < 0)
	{
		printf("Can not check signal!\n");
		return -6;
	}

	if(check_sim_cgmi(comport_tty) < 0)
	{
		printf("Can not check signal!\n");
		return -7;
	}

	if(check_sim_cgsn(comport_tty) < 0)
	{
		printf("Can not check signal!\n");
		return -8;
	}

	if(check_sim_ati(comport_tty) < 0)
	{
		printf("Can not check signal!\n");
		return -9;
	}

}


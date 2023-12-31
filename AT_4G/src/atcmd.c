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
        printf("Send AT cmd failure!\n");
        return -2;
    }

    usleep(10000);

    rv = tty_recv(comport_tty,temp_msg,sizeof(temp_msg),timeout);
    if(rv < 0)
    {
        printf("Receive AT response failure\n");
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
        printf("[%s] Invalid input arguments\n",__func__);
        return -1;
    }

    rv = send_recv_atcmd(comport_tty,"AT\r","OK",NULL,0,2);

    if(rv < 0)
    {
        printf("Send AT But failure\n");
        return -2;
    }

    return 0;
}

//发送指令AT+QNWINFO,获取mmc,mcc
int check_sim_mcc(comport_tty_t *comport_tty,char *pmcc,char *pmnc)
{
        int      rv =0;
        char     buf[128]={0};
    char     buff[8]={0};
        char    *useless1;
        char    *useless2;
        char    *useless3;

        if(!comport_tty)
        {
                printf("[%s] Invalid input arguments\n",__func__);
                return -1;
        }

        rv = send_recv_atcmd(comport_tty,"AT+QNWINFO\r","OK",buf,sizeof(buf),2);

        if(rv < 0)
        {
                printf("Send AT+QNWINFO But failure\n");
                return -2;
        }

        useless1 = strtok(buf,"\"");
        useless2 = strtok(NULL,"\"");
        useless3 = strtok(NULL,"\"");
        strcpy(buff,strtok(NULL,"\""));

        strncpy(pmcc, buff, 3);
        pmcc[3] = '\0';
        strncpy(pmnc, buff+3, 2);
        pmnc[2] = '\0';

        printf("pmmc=%s\n",pmcc);
        printf("pmnc=%s\n",pmnc);

        return 0;
}

//AT命令设置APN
int check_sim_Apn(comport_tty_t *comport_tty,char *papn)
{
        int         rv = 0;
        char        buf[128]={0};

        if(!comport_tty)
        {
                printf("[%s] Invalid input arguments\n",__func__);
                return -1;
        }

        snprintf(buf,50,"AT+CGDCONT=1,\"IP\",\"%s\",\"0.0.0.0\",0,0\r",papn);

        rv = send_recv_atcmd(comport_tty,buf,"OK",buf,sizeof(buf),2);

        if(rv < 0)
        {
                printf("Setting APN failure!\n");
                return -2;
        }
		printf("Setting APN successful\n");
}

//发送指令AT+CGATT? 回复是1表示已经连上基站
int check_sim_cgatt(comport_tty_t *comport_tty)
{
    int             rv = 0;
    if(!comport_tty)
    {   
        printf("[%s] Invalid input arguments\n",__func__);
        return -1; 
    }   

    rv = send_recv_atcmd(comport_tty,"AT+CGATT?\r","1",NULL,0,2);

    if(rv < 0)
    {   
        printf("Send AT+CGATT? But failure\n");
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
        printf("[%s] Invalid input arguments\n",__func__);
        return -1; 
    }   

    rv = send_recv_atcmd(comport_tty,"AT+CGMI\r","",NULL,0,2);

    if(rv < 0)
    {   
        printf("Send AT+CGMI But failure\n");
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
        printf("[%s] Invalid input arguments\n",__func__);
        return -1; 
    }   

    rv = send_recv_atcmd(comport_tty,"AT+CGSN\r","",NULL,0,2);

    if(rv < 0)
    {   
        printf("Send AT+CGSN But failure\n");
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
        printf("[%s] Invalid input arguments\n",__func__);
        return -1; 
    }   

    rv = send_recv_atcmd(comport_tty,"ATI\r","",NULL,0,2);

    if(rv < 0)
    {   
        printf("Send ATI But failure\n");
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
        printf("[%s] Invalid input arguments\n",__func__);
        return -1;
    }

    rv = send_recv_atcmd(comport_tty,"AT+CPIN?\r","READY",NULL,0,2);
    
	if(rv < 0)
    {
        printf("Send AT+CPIN? But failure\n");
        return -2;
    }

    return 0;
}

//发送指令为AT+CREG?,期望收到0,1或者0,3.检测SIM卡是否注册上了。
int check_sim_register(comport_tty_t *comport_tty)
{
    int             rv = 0;

    if(!comport_tty)
    {
        printf("[%s] Invalid input arguments\n",__func__);
        return -1;
    }

    rv = send_recv_atcmd(comport_tty,"AT+CREG?\r","0,1",NULL,0,2);

    if(rv < 0) 
    {
        printf("SIM Card is not regsiter\n");
        return -2;
    }

    return 0;
}


//发送指令为AT+CSQ,期望收到0~31,99。  0~31为信号强度，若信号强度为99则表示无信号，第二位通常为99
int check_sim_signal(comport_tty_t *comport_tty,int *sim_signal)
{
    int             i = 0;
    int             rv = 0;
    int             signal_strength;
    char            str_signal[8] = {0};
	char            msg[128] = {0};

    if(!comport_tty)
    {
        printf("[%s] Invalid input arguments\n",__func__);
        return -1;
    }


    rv = send_recv_atcmd(comport_tty,"AT+CSQ\r","+CSQ",msg,sizeof(msg),2);
    
	if(rv < 0) 
    {
        printf("Send AT+CSQ But failiure\n");
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
	*sim_signal = atoi(str_signal);

    if(signal_strength < 7 || signal_strength > 31)
    {
        printf("sim card signal strength is too low or no signal and signal strength\n");
        return -3;
    }

    return 0;
}

/* 
//发送指令AT+COPS? 获取运营商信息和APN
int check_sim_apn(comport_tty_t *comport_tty,char *apn)
{
	int     i  = 0;
	int     j  = 0;
	int     rv = 0;
	char    msg[128] = {0};
	char   *useless;

	if(!comport_tty)
	{
		printf("[%s] Invalid input arguments\n",__func__);
		return -1;
	}


	rv = send_recv_atcmd(comport_tty,"AT+COPS?\r","+COPS",msg,sizeof(msg),2);

	if(rv < 0)
	{
		printf("Send AT+COPS? But failiure\n");
		return -2;
	}

	useless = strtok(msg,"\"");
	apn = strtok(NULL,"\"");

	printf("APN= %s\n",apn);

}
*/


//检测所有发送的指令，若全部满足返回0，否则返回-1，-2···
int check_sim_all(comport_tty_t *comport_tty,int *sim_signal)
{
	//发送指令为AT,期望收到OK，检测串口能否通信
    if(check_sim_ready(comport_tty) < 0)
	{
		printf("The serial port is not ready!\n");
		return -1;
	}

	//发送指令为AT+CPIN?,期望收到READY,检测SIM卡是否安装
	if(check_sim_exist(comport_tty) < 0)
	{
		printf("No SIM detected!\n");
		return -2;
	}

	//发送指令为AT+CREG?,期望收到0,1或者0,3.检测SIM卡是否注册上了
	if(check_sim_register(comport_tty) < 0)
	{
		printf("SIM card has no regsiter!\n");
		return -3;
	}

	//发送指令为AT+CSQ,期望收到0~31,99。  0~31为信号强度，若信号强度为99则表示无信号，第二位通常为99
	if(check_sim_signal(comport_tty,sim_signal) < 0)
	{
		printf("The SIM card has no signal!\n");
		return -4;
	}

}


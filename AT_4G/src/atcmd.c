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

/*-----------------------------------------------------------------------
|  funtion：Check comport_tty port whether already ready.        
|                                                                        
| argument: serial_fd: comport_tty port fd.                                   
|                                                                        
|   return: rv: success: == 0  ;  error: < 0                             
|-----------------------------------------------------------------------*/
int check_serial_ready(comport_tty_t *comport_tty)
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
        printf("send_resp_serial() of %s failure and rv: %d\n",__func__,rv);
        return -2;
    }

    return 0;
}

/*-----------------------------------------------------------------------
|  funtion：Check sim cart whether already exist.                        
|                                                                        
| argument: serial_fd: comport_tty port fd.                                   
|                                                                        
|   return: rv: success: == 0  ;  error: < 0                             
|-----------------------------------------------------------------------*/
int check_sim_exist(comport_tty_t *comport_tty)
{
    int             rv = 0;

    if(!comport_tty)
    {
        printf("%s,Invalid input arguments\n",__func__);
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

/*-----------------------------------------------------------------------
|  funtion：Check sim cart whether already register.                     
|                                                                        
| argument: serial_fd: serial port fd.                                   
|                                                                        
|   return: rv: success: == 0  ;  error: < 0                             
|-----------------------------------------------------------------------*/
int check_sim_register(comport_tty_t *comport_tty)
{
    int             rv1 = 0;
	int             rv2 = 0;

    if(!comport_tty)
    {
        printf("%s,Invalid input arguments\n",__func__);
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

/*--------------------------------------------------------------------------
|  funtion：Check sim cart whether already signal.                          
|                                                                           
| argument: serial_fd: serial port fd.                                      
|                                                                           
|   return: rv: success: == 0  ;  error: < 0                                
|---------------------------------------------------------------------------*/
int check_sim_signal(comport_tty_t *comport_tty)
{
    int             i = 0;
    int             rv = 0;
    int             signal_strength;
    char            str_signal[8] = {0};
	char            msg[128] = {0};

    if(!comport_tty)
    {
        printf("%s,Invalid input arguments\n",__func__);
        return -1;
    }


    rv = send_recv_atcmd(comport_tty,"AT+CSQ\r","+CSQ",msg,sizeof(msg),2);
    
	if(rv < 0) 
    {
        printf("send_resp_serial() of %s failure and rv: %d\n",__func__,rv);
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



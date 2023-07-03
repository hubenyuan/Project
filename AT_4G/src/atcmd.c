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

#include "atcmd.h"
#include "comport.h"


int send_recv_atcmd(comport_tty_t *comport_tty,atcmd_ctx_t *atcmd)
{
    int     rv = 0;
    char    recv_tranfer[512] = {0};

    if(!atcmd || !comport_tty)
    {
        printf("%s,Invalid input arguments\n",__func__);
        return -1;   
    }

    rv = tty_send(comport_tty,atcmd->send_at_buf,strlen(atcmd->send_at_buf));
    if(rv < 0)
    { 
        printf("comport_tty port send AT cmd failure and rv: %d\n",rv);
        return -2;
    }

    usleep(10000);

    rv = tty_recv(comport_tty,recv_tranfer,sizeof(recv_tranfer));
    if(rv < 0)
    {
        printf("comport_tty port receive AT response failure and rv: %d\n",rv);
        return -3;
    }


    if(!strstr(recv_tranfer,atcmd->succe_at_buf))
    {
        printf("recv_tranfer: %s",recv_tranfer);
        printf("recive AT response have not expect data and [%s] not in recive AT response\n",atcmd->succe_at_buf);
        return -4;
    }

    if(atcmd->use_len)
    {
        strncpy(atcmd->use_buf,recv_tranfer,atcmd->use_len);
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
    atcmd_ctx_t     atcmd;

    if(!comport_tty)
    {
        printf("%s,Invalid input arguments\n",__func__);
        return -1;
    }

    memset(&atcmd,0,sizeof(atcmd_ctx_t));

    strncpy(atcmd.send_at_buf,"AT\r",sizeof(atcmd.send_at_buf));
    strncpy(atcmd.succe_at_buf,"OK",sizeof(atcmd.succe_at_buf));
    atcmd.resp_timeout = 2;

    rv = send_recv_atcmd(comport_tty,&atcmd);
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
    atcmd_ctx_t     atcmd;

    if(!comport_tty)
    {
        printf("%s,Invalid input arguments\n",__func__);
        return -1;
    }

    memset(&atcmd,0,sizeof(atcmd_ctx_t));

    strncpy(atcmd.send_at_buf,"AT+CPIN?\r",sizeof(atcmd.send_at_buf));
    strncpy(atcmd.succe_at_buf,"READY",sizeof(atcmd.succe_at_buf));
    atcmd.resp_timeout = 2;

    rv = send_recv_atcmd(comport_tty,&atcmd);
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
    int             rv = 0;
    atcmd_ctx_t     atcmd;

    if(!comport_tty)
    {
        printf("%s,Invalid input arguments\n",__func__);
        return -1;
    }

    memset(&atcmd,0,sizeof(atcmd_ctx_t));

    strncpy(atcmd.send_at_buf,"AT+CREG?\r",sizeof(atcmd.send_at_buf));
    strncpy(atcmd.succe_at_buf,"0,1",sizeof(atcmd.succe_at_buf));
    atcmd.resp_timeout = 2;

    rv = send_recv_atcmd(comport_tty,&atcmd);

    if(rv < 0) 
    {
        printf("send_resp_serial() of %s failure and rv: %d\n",__func__,rv);
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
int check_sim_signal(comport_tty_t *comport_tty,gsm_ctx_t *gsm_ctx)
{
    int             i = 0;
    int             rv = 0;
    atcmd_ctx_t     atcmd;
    char            judge_sig[8] = {0};
    int             judge_sig_int = 0;

    if(!comport_tty || !gsm_ctx)
    {
        printf("%s,Invalid input arguments\n",__func__);
        return -1;
    }

    memset(&atcmd,0,sizeof(atcmd_ctx_t));

    strncpy(atcmd.send_at_buf,"AT+CSQ\r",sizeof(atcmd.send_at_buf));
    strncpy(atcmd.succe_at_buf,"+CSQ",sizeof(atcmd.succe_at_buf));
    atcmd.use_len = sizeof(atcmd.use_buf);
    atcmd.resp_timeout = 2;

    rv = send_recv_atcmd(comport_tty,&atcmd);
    if(rv < 0) 
    {
        printf("send_resp_serial() of %s failure and rv: %d\n",__func__,rv);
        return -2;
    }

    if(!atcmd.use_buf)
    {
        printf("%s not copy AT response to use_buf\n",__func__);
        return -3;
    }

    for(i=0; i < atcmd.use_len; i++)
    {
        if(',' == atcmd.use_buf[i])
        {
            if(' ' == atcmd.use_buf[i-2])
                strncpy(judge_sig,&atcmd.use_buf[i-1],1); // the signal is single-signal, eg: 6,99
            else
                strncpy(judge_sig,&atcmd.use_buf[i-2],2); // the signal is double-signal, eg: 21,99
            break;
        }
    }

    judge_sig_int = atoi(judge_sig);

    if(judge_sig_int < 7 || judge_sig_int > 31)
    {
        printf("sim card signal strength is too low or no signal and signal strength: %d\n",judge_sig_int);
        return -4;
    }

    gsm_ctx->sim_signal = judge_sig_int;

    return 0;
}

/*-----------------------------------------------------------------------
|  funtion：Check sim card and serial port whether already ready.        
|                                                                        
| argument: serial: serial port property struct.                                   
|                                                                        
|   return: rv: success: == 0  ;  error: < 0                             
|-----------------------------------------------------------------------*/
int check_sim_serial(comport_tty_t *comport_tty,gsm_ctx_t *gsm_ctx)
{
    int     rv = 0;

    if(!comport_tty || !gsm_ctx)
    {
        printf("%s,Invalid input arguments\n",__func__);
        return -1;
    }

    switch(gsm_ctx->status)
    {
        case GSM_STATUS_INIT:
            if((rv = check_serial_ready(comport_tty)) < 0)
            {
                printf("check comport_tty port failure or [not ready] and rv: %d\n",rv);
                break;
            }
            //printf("comport_tty port alread ready\n");

            gsm_ctx->status ++;

        case GSM_STATUS_SERIAL_READY:
            if((rv = check_sim_exist(comport_tty)) < 0)
            {
                printf("check sim card failure or [not exist] and rv: %d\n",rv);
                break;
            }
            //printf("sim card alread exist\n");

            gsm_ctx->status ++;
        
        case GSM_STATUS_SIM_EXIST:
            if((rv = check_sim_register(comport_tty)) < 0)
            {
                printf("check sim card failure or [not register] and rv: %d\n",rv);
                break;
            }
            //printf("sim card alread register\n");

            gsm_ctx->status ++;

        case GSM_STATUS_SIM_REG:
            if((rv = check_sim_signal(comport_tty,gsm_ctx)) < 0)
            {
                printf("check sim card failure or [not signal] and rv: %d\n",rv);
                break;
            }
            //printf("sim card have signal\n");

            gsm_ctx->status ++;

        default: break;
    }

    return gsm_ctx->status;
}


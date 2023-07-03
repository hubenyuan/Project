/********************************************************************************
 *      Copyright:  (C) 2023 Hu Ben Yuan <2017603273@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  atcmd.h
 *    Description:  This head file 
 *
 *        Version:  1.0.0(02/07/23)
 *         Author:  Hu Ben Yuan <2017603273@qq.com>
 *      ChangeLog:  1, Release initial version on "02/07/23 19:54:06"
 *                 
 ********************************************************************************/

#ifndef  _ATCMD_H_
#define  _ATCMD_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>

#include "comport.h"

#define SEND_BUF    128

enum
{
    GSM_STATUS_INIT = 0,        //SIM card and serial port initial status.
    GSM_STATUS_SERIAL_READY,    //Serial port ready.
    GSM_STATUS_SIM_EXIST,       //SIM card exist.
    GSM_STATUS_SIM_REG,         //SIM card already register.
    GSM_STATUS_SIM_SIG,         //SIM card have signal/
    GSM_STATUS_READY = GSM_STATUS_SIM_SIG,  //SIM card and serial port all ready.
};


/*-----------------------------------------------------------------------
|  funtion：AT command send and receive respond.                                          
|                                                                        
| argument: serial: serial port property struct.                                
|           2、atcmd: send at command relevant parameters struct.        
|                                                                        
|   return: rv: success: == 0  ;  error: < 0                             
|-----------------------------------------------------------------------*/
int send_recv_atcmd(comport_tty_t *comport_tty,char *atcmd,char *expect_recv,char *rmsg,int msgsize,int timeout);


/*-----------------------------------------------------------------------
|  funtion：Check sim card and serial port whether already ready.        
|                                                                        
| argument: serial: serial port property struct.                                   
|                                                                        
|   return: rv: success: == 0  ;  error: < 0                             
|-----------------------------------------------------------------------*/
int check_sim_serial(comport_tty_t *comport_tty);


/*-----------------------------------------------------------------------
|  funtion：Check serial port whether already ready.        
|                                                                        
| argument: serial_fd: serial port fd.                                   
|                                                                        
|   return: rv: success: == 0  ;  error: < 0                             
|-----------------------------------------------------------------------*/
int check_serial_ready(comport_tty_t *comport_tty);


/*-----------------------------------------------------------------------
|  funtion：Check sim cart whether already exist.                        
|                                                                        
| argument: serial_fd: serial port fd.                                   
|                                                                        
|   return: rv: success: == 0  ;  error: < 0                             
|-----------------------------------------------------------------------*/
int check_sim_exist(comport_tty_t *comport_tty);


/*-----------------------------------------------------------------------
|  funtion：Check sim cart whether already register.                     
|                                                                        
| argument: serial_fd: serial port fd.                                   
|                                                                        
|   return: rv: success: == 0  ;  error: < 0                             
|-----------------------------------------------------------------------*/
int check_sim_register(comport_tty_t *comport_tty);


/*--------------------------------------------------------------------------
|  funtion：Check sim cart whether already signal.                          
|                                                                           
| argument: serial_fd: serial port fd.                                      
|                                                                           
|   return: rv: success: == 0  ;  error: < 0                                
|---------------------------------------------------------------------------*/
int check_sim_signal(comport_tty_t *comport_tty);


#endif   /* ----- #ifndef _ATCMD_H_  ----- */


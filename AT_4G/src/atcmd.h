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

int send_recv_atcmd(comport_tty_t *comport_tty,char *atcmd,char *expect_recv,char *rmsg,int msgsize,int timeout);

int check_sim_ready(comport_tty_t *comport_tty);

int check_sim_exist(comport_tty_t *comport_tty);

int check_sim_register(comport_tty_t *comport_tty);

int check_sim_cgatt(comport_tty_t *comport_tty);

int check_sim_signal(comport_tty_t *comport_tty,int *sim_signal);

int check_sim_cimi(comport_tty_t *comport_tty);

int check_sim_cgmi(comport_tty_t *comport_tty);

int check_sim_cgsn(comport_tty_t *comport_tty);

int check_sim_ati(comport_tty_t *comport_tty);

int check_sim_all(comport_tty_t *comport_tty,int *sim_signal);


#endif   /* ----- #ifndef _ATCMD_H_  ----- */


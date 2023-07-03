/********************************************************************************
 *      Copyright:  (C) 2023 Hu Ben Yuan<2017603273@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  comport.h
 *    Description:  This head file 
 *
 *        Version:  1.0.0(02/07/23)
 *         Author:  Hu Ben Yuan<2017603273@qq.com>
 *      ChangeLog:  1, Release initial version on "02/07/23 11:11:26"
 *                 
 ********************************************************************************/

#ifndef  COMPORT_H
#define  COMPORT_H

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

typedef struct comport_tty_s{
    int     fd;//文件描述符
    int     baudrate;//波特率
    int     databits;//数据位
    char    parity;//奇偶校验位
    int     stopbits;//停止位
    char    serial_name[64];//设备文件名
    int     msend_len;//单次最大发送长度
    int     timeout;//读数据时的最长延时
    struct  termios old_termios;//保存串口初始属性
}comport_tty_t;

int tty_open(comport_tty_t *comport_tty);

int tty_close(comport_tty_t *comport_tty);

int tty_init(comport_tty_t *comport_tty);

int tty_send(comport_tty_t *comport_tty, char *send_buf, int sbuf_len);

int tty_recv(comport_tty_t *comport_tty, char *recv_buf, int rbuf_len, int timeout);

#endif

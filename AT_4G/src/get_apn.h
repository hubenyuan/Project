/********************************************************************************
 *      Copyright:  (C) 2023 Hu Ben Yuan<2017603273@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  get_apn.h
 *    Description:  This is a obtain apn file 
 *
 *        Version:  1.0.0(08/12/2023)
 *         Author:  Hu Ben Yuan <2017603273@qq.com>
 *      ChangeLog:  1, Release initial version on "08/12/2023 09:16:39 PM"
 *                 
 ********************************************************************************/

#ifndef  GET_APN_
#define  GET_APN_

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/xmlmemory.h>
#include <libxml2/libxml/tree.h>

#define   FILE_NAME  "/home/hubenyuan/hby/test/apns-full-conf.xml"

void query_apn(char *file_name,int *pmcc,int *pmnc,char *papn);

#endif 

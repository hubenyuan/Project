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

#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlversion.h>
#include <libxml/xmlstring.h>
#include <libxml/tree.h>

#define   FILE_NAME  "/home/hubenyuan/hby/test/apns-full-conf.xml"

int query_apn(char *file_name,char *pmcc,char *pmnc,char *papn);

#endif 

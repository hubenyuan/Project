/*********************************************************************************
 *      Copyright:  (C) 2023 Hu Ben Yuan<2017603273@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  get_apn.c
 *    Description:  This is a obtain apn file 
 *                 
 *        Version:  1.0.0(08/12/2023)
 *         Author:  Hu Ben Yuan <2017603273@qq.com>
 *      ChangeLog:  1, Release initial version on "08/12/2023 05:51:21 PM"
 *                 
 ********************************************************************************/

#include "get_apn.h"

int query_apn(char *file_name,char *pmcc,char *pmnc,char *papn)
{
        xmlDocPtr       doc;        //文档指针
        xmlNodePtr      cur;        //根节点
        xmlChar         *mcc;       //移动国家代码
        xmlChar         *mnc;       //移动网络代码
        xmlChar         *apn;       //PPP拨号APN
		/*      目标mcc与mnc      */
        char    *qmcc;
        char    *qmnc;
		qmnc  =  pmnc;
		qmcc  =  pmcc;

		printf("qmcc=%s\n",qmcc);
 
        if( !file_name )
        {
                printf("Invalid input arguments\n");
        }
 
        doc = xmlReadFile(file_name, "UTF-8", XML_PARSE_RECOVER);  //读入需要解析的xml文件
        if (doc == NULL)
        {
                printf("Failed to read xml file:%s\n", file_name);
                goto cleanup;
        }
 
        cur = xmlDocGetRootElement(doc);  //获取根节点
        if (cur == NULL)
        {
                printf("Root is empty.\n");
                goto cleanup;
        }
 
        if ( xmlStrcmp(cur->name, (const xmlChar *)"apns") )    //判断根节点是否正确，注意强制类型转换
        {
                printf("The root is not apns.\n");
                goto cleanup;
        }
 
        cur = cur->xmlChildrenNode;     //由根节点指向子节点
 
        while (cur != NULL)
        {
			if ( xmlStrcmp(cur->name, (const xmlChar *)"apn")==0 )  //现在的cur->name子节点的name，判断name里有没有apn，若有，往下走
            {
                /*      xmlGetProp取出其属性值      */
                mcc = xmlGetProp(cur, "mcc");
                mnc = xmlGetProp(cur, "mnc");
 
                if( xmlStrcmp(mcc, (const xmlChar *)qmcc)==0 && xmlStrcmp(mnc, (const xmlChar *)qmnc)==0 )  //若mcc和mnc是460 03同时成立，则获取该子节点APN
                {
                    apn = xmlGetProp(cur, "apn");
					sprintf(papn,"%s",apn);
                    printf("mcc:%s mnc:%s apn:%s\n",mcc, mnc, papn);

 
                    break;
                }
            }
            cur = cur->next;        //若不满足条件则指向下一个子节点
        }
cleanup:
		if (doc)
        {
            xmlFreeDoc(doc);
        }
 
}

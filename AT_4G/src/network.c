/*********************************************************************************
 *      Copyright:  (C) 2023 Hu Ben Yuan<2017603273@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  network.c
 *    Description:  This Judgment networking file 
 *                 
 *        Version:  1.0.0(07/30/2023)
 *         Author:  Hu Ben Yuan <2017603273@qq.com>
 *      ChangeLog:  1, Release initial version on "07/30/2023 10:14:25 AM"
 *                 
 ********************************************************************************/

#include "network.h"


int jud_ppp0_net(void)
{
	int         result;
	char        buffer[256];
	struct      stat stt;
	FILE       *file;
	const char *netpath = "/sys/class/net/ppp0";
	char command[128] = "nohup ping -c 4 -I ppp0 baidu.com > ping_result.txt &";

	result = system(command);

	if (result == -1)
	{
		printf("Command execution failure\n");
		return -1;
	}

	// 等待ping命令执行完成
	sleep(6);

	// 读取ping结果文件
	file = fopen("ping_result.txt", "r");
	if (file == NULL)
	{
		printf("Can't Open the file\n");
		fclose(file);
		return -2;
	}

	// 解析ping结果文件
	while (fgets(buffer, sizeof(buffer), file) != NULL)
	{
		if (strstr(buffer, "4 packets transmitted, 4 received") != NULL)
		{
			fclose(file);
			return 1;  // 网络连接成功
		}
	}

	fclose(file);
	return 0;  // 网络连接失败
}

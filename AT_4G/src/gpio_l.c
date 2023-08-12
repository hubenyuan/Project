/*********************************************************************************
 *      Copyright:  (C) 2023 Hu Ben Yuan<2017603273@qq.com>
 *                  All rights reserved.
 *
 *    Description:  This GPIO Command pin file
 *                 
 *        Version:  1.0.0(07/30/2023)
 *         Author:  Hu Ben Yuan <2017603273@qq.com>
 *      ChangeLog:  1, Release initial version on "07/30/2023 07:30:50 PM"
 *                 
 ********************************************************************************/

#include <gpiod.h>

#include "gpio_l.h"

//初始化LED灯的GPIO引脚
int gpio_init(gpiod_led_t *gpiod_led)
{
	const char *chipname = "gpiochip1";
	
	// Open GPIO chip
	gpiod_led->chip = gpiod_chip_open_by_name(chipname);
	
	if (!gpiod_led->chip)
	{
		printf("Error opening GPIO chip\n");
		return -1;
	}

	// Open GPIO line
	gpiod_led->G4_led_line = gpiod_chip_get_line(gpiod_led->chip, 14);

	if (!gpiod_led->G4_led_line)
	{
		printf("Error getting GPIO line\n");
		gpiod_chip_close(gpiod_led->chip);
		return -2;
	}

	// Open switch line for output 设置为输出模式
	if( gpiod_line_request_output(gpiod_led->G4_led_line, "eth_led", 0) < 0 )
	{
		printf("Error setting GPIO line direction\n");
		gpiod_line_release(gpiod_led->G4_led_line);
		gpiod_chip_close(gpiod_led->chip);
		return -3;
	}

	return 0;
}

//初始化PD15和PD17的GPIO引脚，并判断4G模块在不在，并判断PA31引脚，给模块上电
int module_inin(gpiod_led_t *gpiod_led)
{
	const char *chipname1 = "gpiochip0";  //管理PA31
	const char *chipname2 = "gpiochip3";  //管理PD15 和 PD17

	// Open GPIO chip
	gpiod_led->chip2 = gpiod_chip_open_by_name(chipname2);

	if (!gpiod_led->chip2)
	{
		printf("Error opening GPIO chip\n");
		return -1;
	}

	gpiod_led->chip1 = gpiod_chip_open_by_name(chipname1);

	if (!gpiod_led->chip1)
	{
		printf("Error opening GPIO chip\n");
		return -1;
	}

	// Open GPIO line
	gpiod_led->PD15_line = gpiod_chip_get_line(gpiod_led->chip2, 15);
	gpiod_led->PD17_line = gpiod_chip_get_line(gpiod_led->chip2, 17);
	if (!gpiod_led->PD15_line && !gpiod_led->PD17_line)
	{
		printf("Error getting GPIO line\n");
		gpiod_chip_close(gpiod_led->chip2);
		return -2;
	}

	gpiod_led->PA31_line = gpiod_chip_get_line(gpiod_led->chip1, 31);
	if(!gpiod_led->PA31_line)
	{
		printf("Error getting GPIO line\n");
		gpiod_chip_close(gpiod_led->chip1);
		return -2;
	}

	if( gpiod_line_request_output(gpiod_led->PA31_line, "eth_led", 0) < 0 )
	{
		printf("Error setting GPIO line direction\n");
		gpiod_line_release(gpiod_led->PA31_line);
		gpiod_chip_close(gpiod_led->chip1);
		return -3;
	}

	return 0;

}

//拉低PA31引脚电平，给模块上电
int pull_up(gpiod_led_t *gpiod_led)
{
	gpiod_line_set_value(gpiod_led->PA31_line, 0);
}

//拉高PA31引脚电平,给模块停电
int pull_down(gpiod_led_t *gpiod_led)
{
	gpiod_line_set_value(gpiod_led->PA31_line, 1);
}

//led灯慢闪
int led_slow(gpiod_led_t *gpiod_led,int *ptr) 
{
	while(*ptr == 1)
	{
	    gpiod_line_set_value(gpiod_led->G4_led_line, 0);
	    usleep(1000000);
	    gpiod_line_set_value(gpiod_led->G4_led_line, 1);
	    usleep(1000000);
	}

	return 0;
}

//led快闪
int led_fast(gpiod_led_t *gpiod_led,int *ptr) 
{
    while(*ptr == 0)
    {   
        gpiod_line_set_value(gpiod_led->G4_led_line, 0); 
        usleep(500000);
        gpiod_line_set_value(gpiod_led->G4_led_line, 1); 
        usleep(500000);
    }

	return 0;
}

//led一直亮
int led_bright(gpiod_led_t *gpiod_led)
{
	gpiod_line_set_value(gpiod_led->G4_led_line, 0);

}

//关闭led灯
int led_close(gpiod_led_t *gpiod_led)
{
	gpiod_line_set_value(gpiod_led->G4_led_line, 1);	
	gpiod_line_release(gpiod_led->G4_led_line);
	gpiod_chip_close(gpiod_led->chip);
}

int gpio_close(gpiod_led_t *gpiod_led)
{
	gpiod_line_release(gpiod_led->PA31_line);
	gpiod_chip_close(gpiod_led->chip1);
}









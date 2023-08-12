/********************************************************************************
 *      Copyright:  (C) 2023 Hu Ben Yuan<2017603273@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  gpio_l.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(07/30/2023)
 *         Author:  Hu Ben Yuan <2017603273@qq.com>
 *      ChangeLog:  1, Release initial version on "07/30/2023 08:29:53 PM"
 *                 
 ********************************************************************************/

#ifndef   GPIO_L_H
#define   GPIO_L_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

typedef struct gpiod_led_s
{
	struct          gpiod_chip *chip;         /* gpio 芯片 */
	struct          gpiod_chip *chip1;
	struct          gpiod_chip *chip2;
	struct          gpiod_line *G4_led_line;  /* gpio控制口 */
	struct          gpiod_line *PD15_line;
	struct          gpiod_line *PD17_line;
	struct          gpiod_line *PA31_line;
} gpiod_led_t;

extern int gpio_init(gpiod_led_t *gpiod_led);

extern int module_inin(gpiod_led_t *gpiod_led);

extern int pull_up(gpiod_led_t *gpiod_led);

extern int pull_down(gpiod_led_t *gpiod_led);

extern int led_slow(gpiod_led_t *gpiod_led,int *ptr);

extern int led_fast(gpiod_led_t *gpiod_led,int *ptr);

extern int led_bright(gpiod_led_t *gpiod_led);

extern int led_close(gpiod_led_t *gpiod_led);

extern int gpio_close(gpiod_led_t *gpiod_led);

#endif

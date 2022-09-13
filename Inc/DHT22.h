#ifndef _DHT22_H
#define _DHT22_H
#include "main.h"

void read_dht22_RH(void);
extern uint16_t get_tem_tick;
extern unsigned char U8RH_data_H;
extern unsigned char U8RH_data_L;
extern unsigned char U8T_data_H;
extern unsigned char U8T_data_L;
extern void delay_us(uint32_t i);
extern uint16_t g_temp;
extern uint16_t g_hump;
#endif


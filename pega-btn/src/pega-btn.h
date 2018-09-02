#ifndef _BTN_EVENT_H
#define _BTN_EVENT_H

#define WIX_NO_BTN 			-1
#define WIX_BTN_POWER			12	//power
#define WIX_BTN_LCM_SELECT		5	//lcd_select
#define WIX_BTN_LCM_SCROLL		4	//lcd_scroll
#define WIX_BTN_RESET			9	//reset
#define WIX_BTN_HDD_OVT			15	//hdd ovt
#define WIX_BTN_CPU_OVT			8	//cpu ovt
#define WIX_BTN_HDD_CHANGE		0	//hdd change

#define	READ_BUTTON_NONBLOCKING	_IOWR(0xF4, 150, int)
#define	READ_BUTTON_BLOCKING	_IOWR(0xF4, 151, int)

#endif
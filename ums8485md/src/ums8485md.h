#ifndef __LCM_CONTROL_H
#define __LCM_CONTROL_H
typedef struct lcm_member_s {
	int ctrl;
	int page;
	int column;
	int size;
	unsigned char data[128];
} lcm_member_t;

#define	WIX_LCM_CMD_PON			0x01
#define	WIX_LCM_CMD_POFF		0x02
#define	WIX_LCM_CMD_RESET		0x03
#define	WIX_LCM_CMD_DISP_NORMAL 	0x04
#define	WIX_LCM_CMD_DISP_REVERSE 	0x05
#define	WIX_LCM_CMD_ENTIRE_DISP_ON	0x06
#define	WIX_LCM_CMD_ENTIRE_DISP_OFF	0x07
#define	WIX_LCM_CMD_ADC_SELECT_NORMAL 	0x08
#define	WIX_LCM_CMD_ADC_SELECT_REVERSE	0x09
#define	WIX_LCM_CMD_OUTPUT_NORMAL	0x0a
#define	WIX_LCM_CMD_OUTPUT_REVERSE	0x0b
#define	WIX_LCM_CMD_WRITE_DATA		0x0c

#if 0
#define	WIX_LCM_CMD_WRITE_DISP_DATA
#define	WIX_LCM_CMD_READ_DISP_DATA
#endif

#define IOCTL_DISPLAY_COMMAND 		_IOWR(0xF4, 0, lcm_member_t)
#define	IOCTL_DISPLAY_WRITE		_IOWR(0xF4, 1, lcm_member_t)
#define	IOCTL_DISPLAY_READ		_IOWR(0xF4, 2, lcm_member_t)

#endif


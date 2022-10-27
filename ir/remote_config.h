#ifndef  _REMOTE_CONFIG_H
#define  _REMOTE_CONFIG_H
#include <stdint.h>
#include <asm/ioctl.h>

#define REMOTE_IOC_SET_KEY_NUMBER        _IOW('I', 3, uint32_t)
#define REMOTE_IOC_SET_KEY_MAPPING_TAB   _IOW('I', 4, uint32_t)
#define REMOTE_IOC_SET_SW_DECODE_PARA    _IOW('I', 5, uint32_t)
#define REMOTE_IOC_GET_DATA_VERSION      _IOR('I', 121, uint32_t)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

typedef   struct{
       uint32_t *key_map;
       uint32_t *repeat_key_map;
       uint32_t *mouse_map;
	   unsigned int *factory_customercode_map;
       unsigned int repeat_delay;
       unsigned int repeat_peroid;
       unsigned int work_mode ;
       unsigned int mouse_speed;
	unsigned int repeat_enable;
	unsigned int factory_infcode;
	unsigned int factory_unfcode;
	unsigned int factory_code;
	unsigned int release_delay;
	unsigned int release_fdelay;
	unsigned int release_sdelay;
	unsigned int debug_enable;
//sw
	unsigned int 	bit_count;
	unsigned int 	tw_leader_act;
	unsigned int 	tw_bit0;
	unsigned int   tw_bit1;
	unsigned int   tw_bit2;
	unsigned int   tw_bit3;
	unsigned int 	tw_repeat_leader;
//reg
	unsigned int  reg_base_gen;
	unsigned int  reg_control;
	unsigned int  reg_leader_act;
	unsigned int  reg_leader_idle;
	unsigned int  reg_repeat_leader;
	unsigned int  reg_bit0_time;

	unsigned int fn_key_scancode;
        unsigned int left_key_scancode;
	unsigned int right_key_scancode;
	unsigned int up_key_scancode;
	unsigned int down_key_scancode;
	unsigned int ok_key_scancode;
	unsigned int pageup_key_scancode;
	unsigned int pagedown_key_scancode;
}remote_config_t;

//these string must in this order and sync with struct remote_config_t
static char*  config_item[33]={
    "repeat_delay",
    "repeat_peroid",
    "work_mode",
    "mouse_speed",
    "repeat_enable",
    "factory_infcode",
    "factory_unfcode",
    "factory_code",
    "release_delay",
    "release_fdelay",
    "release_sdelay",
    "debug_enable",
//sw
    "bit_count",
    "tw_leader_act",
    "tw_bit0",
    "tw_bit1",
    "tw_bit2",
    "tw_bit3",
    "tw_repeat_leader",
//reg
    "reg_base_gen",
    "reg_control",
    "reg_leader_act",
    "reg_leader_idle",
    "reg_repeat_leader",
    "reg_bit0_time",

    "fn_key_scancode",
    "left_key_scancode",
    "right_key_scancode",
    "up_key_scancode",
    "down_key_scancode",
    "ok_key_scancode",
    "pageup_key_scancode",
    "pagedown_key_scancode",
};

extern int get_config_from_file(FILE *fp, remote_config_t *remote);

#define CUSTOM_NAME_LEN 64
struct cursor_codemap {
	uint16_t fn_key_scancode;
	uint16_t cursor_left_scancode;
	uint16_t cursor_right_scancode;
	uint16_t cursor_up_scancode;
	uint16_t cursor_down_scancode;
	uint16_t cursor_ok_scancode;
};

union _codemap {
	struct ir_key_map {
		uint16_t keycode;
		uint16_t scancode;
		} map;
	uint32_t code;
};

struct ir_map_tab {
	char custom_name[CUSTOM_NAME_LEN];
	struct cursor_codemap cursor_code;
	uint16_t map_size;
	uint32_t custom_code;
	uint32_t release_delay;
	uint32_t vendor;
	uint32_t product;
	uint32_t version;
	union _codemap codemap[0];
};
#endif

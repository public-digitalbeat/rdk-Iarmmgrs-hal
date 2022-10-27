#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <dlfcn.h>
#include <sys/queue.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <linux/input.h>
#include <memory.h>
#include <string.h>

#include "comcastIrKeyCodes.h"
#include "amremote.h"
#include "plat_ir.h"
#include "remote_config.h"

#define DEVICE_NAME                     "/dev/amremote"
#define AMLOGIC_REMOTE_MAX_NUM_KEYS 72
#define KEY_CONF "/etc/keymap.conf"
#define KEY_ALTERNATE_OK 0x110

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define SUCCESS 1
#define FAILURE 0

uint32_t key_map[256], repeat_key_map[256], mouse_map[4];
static remote_data_t *remoteAmlData = NULL;

static  IR_Comcast_Key_Map_t KeyMap_table[AMLOGIC_REMOTE_MAX_NUM_KEYS] = {
	{KEY_0, KED_DIGIT0},
	{KEY_1, KED_DIGIT1},
	{KEY_2, KED_DIGIT2},
	{KEY_3, KED_DIGIT3},
	{KEY_4, KED_DIGIT4},
	{KEY_5, KED_DIGIT5},
	{KEY_6, KED_DIGIT6},
	{KEY_7, KED_DIGIT7},
	{KEY_8, KED_DIGIT8},
	{KEY_9, KED_DIGIT9},
	{KEY_SEARCH, KED_SEARCH},
	{KEY_SETUP, KED_SETUP},
	{KEY_LANGUAGE, KED_LANGUAGE},
	{KEY_POWER, KED_POWER},
	{KEY_UP, KED_ARROWUP},
	{KEY_LEFT, KED_ARROWLEFT},
	{KEY_RIGHT, KED_ARROWRIGHT},
	{KEY_DOWN, KED_ARROWDOWN},
	{KEY_SELECT, KED_SELECT},
	{KEY_ENTER, KED_ENTER},
	{KEY_EXIT, KED_EXIT},
	{KEY_CHANNELUP,KED_CHANNELUP},
	{KEY_CHANNELDOWN,KED_CHANNELDOWN},
	{KEY_VOLUMEDOWN, KED_VOLUMEDOWN},
	{KEY_VOLUMEUP, KED_VOLUMEUP},
	{KEY_MUTE, KED_MUTE},
	{KEY_INFO, KED_INFO},
	{KEY_PAGEUP, KED_PAGEUP},
	{KEY_PAGEDOWN, KED_PAGEDOWN},
	{KEY_YELLOW, KED_KEYA},
	{KEY_BLUE, KED_KEYB},
	{KEY_RED, KED_KEYC},
	{KEY_GREEN, KED_KEYD},
	{KEY_LAST, KED_LAST},
	{KEY_FAVORITES, KED_FAVORITE},
	{KEY_REWIND, KED_REWIND},
	{KEY_FORWARD, KED_FASTFORWARD},
	{KEY_PLAY, KED_PLAY},
	{KEY_STOP, KED_STOP},
	{KEY_PAUSE, KED_PAUSE},	
	{KEY_RECORD, KED_RECORD},
	{KEY_MEDIA_REPEAT, KED_REPLAY},
	{KEY_HELP, KED_HELP},
	{KEY_CLEAR, KED_CLEAR},
	{KEY_DELETE, KED_DELETE},
	{KEY_NUMERIC_POUND, KED_POUND},
	{KEY_OK, KED_OK},
	{KEY_NUMERIC_STAR, KED_STAR},
	{KEY_PREVIOUS, KED_PREVIOUS},
	{KEY_NEXT, KED_NEXT},
	{KEY_HOME, KED_MENU},
	{KEY_AUDIO, KED_AUDIO},
	{KEY_LIST, KED_LIST},
	{KEY_BACKSPACE, KED_BACK},
	{KEY_F1, KED_UNDEFINEDKEY},
	{KEY_F2, KED_UNDEFINEDKEY},
	{KEY_F3, KED_UNDEFINEDKEY},
	{KEY_F4, KED_UNDEFINEDKEY},
	{KEY_F5, KED_UNDEFINEDKEY},
	{KEY_F6, KED_UNDEFINEDKEY},
	{KEY_F7, KED_UNDEFINEDKEY},
	{KEY_F8, KED_UNDEFINEDKEY},
	{KEY_F9, KED_UNDEFINEDKEY},
	{KEY_F10, KED_UNDEFINEDKEY},
	{KEY_F11, KED_UNDEFINEDKEY},
	{KEY_TAB, KED_UNDEFINEDKEY},
	{KEY_ZOOMIN, KED_UNDEFINEDKEY},
	{KEY_ZOOMOUT, KED_UNDEFINEDKEY},
	{KEY_PROPS, KED_UNDEFINEDKEY}
};
unsigned short default_mouse_map[4] = {
 /* 0x10, 0x11, 0x0b, 0x0e */
 0xffff, 0xffff, 0xffff, 0xffff
};

unsigned short adc_map[2] ={0xffff, 0xffff}; /*left,right*/
unsigned int adc_move_enable = 0;

/**
 * @brief Initialize the underlying IR module.
 *
 * This function must initialize all the IR specific user input device modules.
 *
 * @param     None.
 * @return    Return Code.
 * @retval    0 if successful.
 */
int PLAT_API_INIT(void)
{
    remote_config_t *remote = NULL;
    FILE *fp = NULL;
    int ret = 0, i = 0, j = 0;
    int device_fd = -1;
    struct ir_map_tab *ir_table = 0;
    uint32_t nkeys = 256;

    printf("Inside %s :%d\n",__FUNCTION__,__LINE__);
    remoteAmlData = (remote_data_t *) malloc (sizeof(remote_data_t) );
    if(!remoteAmlData)
    {
        printf("%s: Error: Failed to Allocate "
                "Memory for remoteAmldata\n",__FUNCTION__);
        return -1;
    }

    memset(remoteAmlData, 0, sizeof(remote_data_t) );
    remoteAmlData->hash_table = g_hash_table_new(g_int_hash, g_int_equal);
    if(!remoteAmlData->hash_table)
    {
        printf("Hash Table Creation Failed %s \n",__FUNCTION__);
        return -1;
    }
    for(i=0;
            i < (sizeof(KeyMap_table)/sizeof(IR_Comcast_Key_Map_t)); 
            i++)
    {
        g_hash_table_insert(remoteAmlData->hash_table, 
                &KeyMap_table[i].Amkey, 
                &KeyMap_table[i].comcastkey);
        printf("****KeyMap_table[%d].Amkey is:::%u\n",i,KeyMap_table[i].Amkey);
        printf("***KeyMap_table[%d].comcastkey::%lu\n",i,KeyMap_table[i].comcastkey);
    }
    for(i =0; i < 256; i++)
        key_map[i] = KEY_RESERVED;
    for(i =0; i < 256; i++)
        repeat_key_map[i] = KEY_RESERVED;
    for(i =0; i < 4; i++)
        mouse_map[i] = 0xffff;

    remote = (remote_config_t *)malloc(sizeof(remote_config_t));
    if(!remote){
        printf("out of memory !\n");
        return -1;
    }

    memset((unsigned char*)remote, 0xff, sizeof(remote_config_t));
    remote->key_map = key_map;
    remote->repeat_key_map = repeat_key_map;
    remote->mouse_map = mouse_map;

    device_fd = open(DEVICE_NAME, O_RDONLY);
    if(device_fd < 0)
    {
        printf("Can't open %s .\n", DEVICE_NAME);
        return -2;
    }

    fp = fopen(REMOTE_CONFIG_FILE, "r");
    if(!fp) {
        printf("Open file %s is failed!!!\n", REMOTE_CONFIG_FILE);
        return -4;
    }
    nkeys = get_config_from_file(fp, remote);
    fclose(fp);

    remote->factory_code >>= 16;

    ir_table  = calloc(sizeof(struct ir_map_tab) +
            nkeys * sizeof(union _codemap), sizeof(char));
    if (!ir_table) {
        close(device_fd);
        return FAILURE;
    }

    strncpy(ir_table->custom_name, "amlogic", CUSTOM_NAME_LEN - 1);
    ir_table->map_size = nkeys;
    ir_table->cursor_code.fn_key_scancode = remote->fn_key_scancode;
    ir_table->cursor_code.cursor_left_scancode = remote->left_key_scancode;
    ir_table->cursor_code.cursor_right_scancode = remote->right_key_scancode;
    ir_table->cursor_code.cursor_up_scancode = remote->up_key_scancode;
    ir_table->cursor_code.cursor_down_scancode = remote->down_key_scancode;
    ir_table->cursor_code.cursor_ok_scancode = remote->ok_key_scancode;

    ir_table->custom_code = remote->factory_code;
    ir_table->release_delay = remote->release_delay;

    for(i = 0; i < 256; i++) {
        if(key_map[i] != KEY_RESERVED){
            ir_table->codemap[j].map.keycode = key_map[i];
            ir_table->codemap[j].map.scancode = i;
            j++;
        }
    }

    ioctl(device_fd, REMOTE_IOC_SET_KEY_NUMBER, &nkeys);
    ioctl(device_fd, REMOTE_IOC_SET_KEY_MAPPING_TAB, ir_table);

    free(ir_table);
    close(device_fd);

    return SUCCESS;
}

/**
* @brief Register callback function to which IR Key events should be posted.
*
* This function registers the calling applications callback function.  The application
* will then be notified of IR Key events via this callback function.
*
* @param [in]  func    Function reference of the callback function to be registered.
* @return None.
*/
void PLAT_API_RegisterIRKeyCallback(PLAT_IrKeyCallback_t func)
{
	printf("Inside %s :%d\n",__FUNCTION__,__LINE__);
	if( !func || !remoteAmlData )
	{
		printf("IRKey callback register failed\n");
		return;
	}
	else
	{
		printf("IRKey callback register success\n");
		remoteAmlData->irkey_handler = func;
	}
}


void process_amlogic_key(unsigned short code, unsigned short key_type)
{
	printf("Inside %s :%d\n",__FUNCTION__,__LINE__);
	int *original_key = 0, *val = 0;
	
	if (g_hash_table_lookup_extended(remoteAmlData->hash_table,
				&code,(void**)&original_key,(void**)&val)){
		printf("VALUE IS:::%d\n",*val);
		printf("ORIGINAL KEY:::%d\n",*original_key);
		/* Call back function when key is received */
		remoteAmlData->irkey_handler(key_type, *val);
	}
	else {
		printf("###########KEY NOT FOUND###################\n");
	}
}

/**
 * @brief Execute key event loop.
 *
 * This function executes the platform-specific key event loop. This will generally
 * translate between platform-specific key codes and Comcast standard keycode definitions.
 *
 * @param None.
 * @return None.
 */
void PLAT_API_LOOP()
{
	printf("Inside %s :%d\n",__FUNCTION__,__LINE__);
	fd_set readfds;
	struct input_event inputevt[AMLOGIC_REMOTE_MAX_NUM_KEYS] = {0};
	int flag=0,retval=0;
	unsigned int index = 0,i = 0,readlen = 0;
	char *device = "/dev/input/event0";
	char name[256] = "Unknown";
	FILE *fp=NULL;
	char buf[16];

	/* To open event device 0 */
	remoteAmlData->input_device_fd = open(device,O_RDONLY);

	if(remoteAmlData->input_device_fd == -1) {
		printf("Failed to open event device.\n");
		perror("open");
		exit(1);
	}
	else {
		printf("open input event success\n");
	}

	ioctl(remoteAmlData->input_device_fd, EVIOCGNAME(sizeof(name)), name);
	printf ("Reading From : %s (%s)\n", device, name);

	for(index = 0 ; index < AMLOGIC_REMOTE_MAX_NUM_KEYS ; index ++ ) {
		memset(&inputevt[index], 0, sizeof(struct input_event));
	}
	while(!flag)
	{
		printf("While Inside #######\n");
		/* Initialize the file descriptor set. */
		FD_ZERO(&readfds);
		FD_SET(remoteAmlData->input_device_fd, &readfds);

		/* select returns 0 if timeout, 1 if input available, -1 if error. */
		retval = select((remoteAmlData->input_device_fd)+1,&readfds, NULL, NULL, NULL);
		printf("retval is :::%d\n",retval);
		if(retval == -1)
		{
			perror("select()");
			flag = 1;
		}
		else if(retval)
		{
			/* To check Data is available now. */
			if(FD_ISSET(remoteAmlData->input_device_fd,&readfds))
			{
				/* To read the data from input_device_fd and store the same data into inputeve buffer */
				readlen = read(remoteAmlData->input_device_fd,inputevt,sizeof(inputevt));
				printf("readlen#########%d\n",readlen);
				/* Process the key pressed */
				for (i=0; i<readlen / sizeof(inputevt[0]); i++) {
					printf("type := %d, code := 0x%x, value := %d\n",
							inputevt[i].type, inputevt[i].code, inputevt[i].value);
					printf("CODE:::%d\n",inputevt[i].code);
					if(inputevt[i].code == KEY_ALTERNATE_OK){
							printf("############code := 0x%x\n",inputevt[i].code);
							printf("############code := %d\n",inputevt[i].code);
                                                        inputevt[i].code = KEY_OK;
                                                }
					fp = fopen(KEY_CONF,"r");
					if(fp)
					{
						fgets(buf, sizeof(buf), fp);
						if((strstr(buf, "keydownDisabled")) && inputevt[i].code != KEY_OK && inputevt[i].code != KEY_HOME && inputevt[i].code != KEY_NUMERIC_STAR)
						{
							/* Code to identify the KEY RELEASE */
							if(inputevt[i].type == 1 && inputevt[i].value == 0 && inputevt[i].value != ' ')
							{
								printf("########keydownDisabled#################\n");
								process_amlogic_key(inputevt[i].code,
										KET_KEYUP);
							}
						}
						else{
							/* Code to identify the KEY PRESS */
							if(inputevt[i].type == 1 && inputevt[i].value == 1 && inputevt[i].value != ' ')
							{
								process_amlogic_key(inputevt[i].code,
										KET_KEYDOWN);
							}

							/* Code to identify the KEY RELEASE */
							if(inputevt[i].type == 1 && inputevt[i].value == 0 && inputevt[i].value != ' ')
							{
								process_amlogic_key(inputevt[i].code,
										KET_KEYUP);
							}
						}
						fclose(fp);
					}

				}
			}
		}
		else{
			printf("No data Found...\n");
		}
	}

	
}
/**
 * @brief Close the IR device module.
 *
 * This function must terminate all the IR specific user input device modules. It must
 * reset any data structures used within IR module and release any IR specific handles
 * and resources.
 *
 * @param None.
 * @return None.
 */
void PLAT_API_TERM()
{
	/* To close the input_device_fd */
	printf("Inside %s :%d\n",__FUNCTION__,__LINE__);
	close(remoteAmlData->input_device_fd);
}


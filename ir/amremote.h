/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __KEYMAP_H_
#define __KEYMAP_H_

#ifdef __cplusplus 
extern "C" {
#endif

#include <glib.h>
#include "plat_ir.h"

#define REMOTE_CONFIG_FILE "/etc/remote.conf"

struct remote_data {
	unsigned int input_device_fd;
	char *device;
	PLAT_IrKeyCallback_t irkey_handler;
	GHashTable *hash_table;
};

typedef struct remote_data remote_data_t;

struct ir_comcast_key_map_t {
	unsigned int Amkey;
	unsigned long comcastkey;
};

typedef struct ir_comcast_key_map_t IR_Comcast_Key_Map_t;

#ifdef __cplusplus 
}
#endif /* __cplusplus */
#endif


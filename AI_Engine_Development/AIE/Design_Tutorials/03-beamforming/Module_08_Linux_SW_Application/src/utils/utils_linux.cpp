
/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/

#include "utils_linux.h"

int extractIQ(unsigned int dataHex, int lsb) {
	//printf("[extractIQ]---------------\n");
	int data;
	if (lsb==0)	data = int(dataHex & 0xffff);
	else		data = int((dataHex & 0xffff0000)>>16 );
	//printf("dataHex:0x%x data:0x%x data:%d\n", dataHex, data, data);
	if (data>=0x8000)
		data = -(0x10000 - data);
	return data;
}




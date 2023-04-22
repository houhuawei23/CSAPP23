///////////////////////////////////////////////////////////////////////
////  Copyright 2020 by mars.                                        //
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>

#include "common.h"

extern struct BT9_struct BT9;

int main(int argc, char* argv[])
{
	int ret_val = -1;
	
	printf("Branch Predictor Framework by mars, 2020\n");
	if (argc != 2)
	{
		printf("Usage: %s <trace>\n", argv[0]);
		exit(-1);
	}
	
	ret_val = parse_bt9_file(argv[1]);
	if (ret_val != 0)
	{
		printf("[%s] Fail decompress file %s\n",__func__, argv[1]);
		return -1;
	}
	printf("  TRACE                       \t : %s\n", argv[1]);
	SimBT9(&BT9);
	FreeBT9();
	return 0;
}

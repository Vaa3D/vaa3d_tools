/*
 * convert2bitrawto4bitraw.c
 *
 *  Created on: 2011-11-17
 *      Author: feng
 */

#include <utilities.h>
#include "tz_image_io.h"
#include <stdlib.h>

int main(int argc, char **argv) {
	static char *Spec[] = {"<input:string>", "[-output <string>]", NULL};
	Process_Arguments(argc, argv, Spec, 1);
	printf("start reading\n");
	Mc_Stack* mcstack = Read_Mc_Stack(Get_String_Arg("input"), -1);
	printf("read done\n");
	Write_Mc_Stack(Get_String_Arg("-output"), mcstack, NULL);
	Kill_Mc_Stack(mcstack);
	printf("done\n");
}

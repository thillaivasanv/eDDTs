/* cpu_core_test.c

 * This verifies the number of cpu cores present in beagle bone evm
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (c) HCL Technologies Limited, 2020
 
 * Author:Arjun Kumar Rath <arjun-r@hcl.com>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*TODO : Verify each cpu core*/


#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/rtc.h>
#include <errno.h>
#include <time.h>
#include <fnmatch.h>

#include "test.h"
#include "safe_macros.h"

#define LINUX_SYS_CPU_DIRECTORY "/sys/devices/system/cpu"

char *TCID = "cpu_core_test";
int TST_TOTAL = 1;

static const option_t options[] = {
	{NULL, NULL, NULL},
	{NULL, NULL, NULL}
};

static void help(void)
{


}

int main(int argc, char *argv[])
{
	int cpu_count = 0;

	tst_parse_opts(argc, argv, options, help);

	tst_require_root();

	tst_resm(TINFO, "CPU Core Test!");

	DIR *sys_cpu_dir = opendir(LINUX_SYS_CPU_DIRECTORY);
   
	if (sys_cpu_dir == NULL) 
	{
		tst_brkm(TCONF,NULL,"Cannot open %s directory",LINUX_SYS_CPU_DIRECTORY);
   	}
   
	const struct dirent *cpu_dir;
   	
	while((cpu_dir = readdir(sys_cpu_dir)) != NULL) 
	{
       		if (fnmatch("cpu[0-9]*", cpu_dir->d_name, 0) != 0)
       		{
          		/* Skip the file which does not represent a CPU */
          		continue;
       		}
       		
		cpu_count++;
   	}
   
	tst_resm(TINFO,"CPU count: %d\n",cpu_count);

	if(cpu_count == 1)
	{	
		tst_resm(TINFO, "CPU Core Test Passed!");
	}else
	{
		tst_resm(TINFO, "CPU Core Test Failed!");
	}


	tst_resm(TINFO, "CPU Core Test Done!");
	tst_exit();

}

/* usb_test.c

 * This test verifies whether a USB device is mounted or not
 * by checking the device logs
 * 
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
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/rtc.h>
#include <errno.h>
#include <time.h>
#include <linux/watchdog.h>

#include "test.h"
#include "safe_macros.h"

char *TCID = "usb_test";
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
	int ret =0; 

	tst_parse_opts(argc, argv, options, help);

	tst_require_root();

	tst_resm(TINFO, "USB Mount Test!");

	/* launch a command and gets its output */
    
	FILE *f = popen("mount | grep /dev/sda1", "r");
    	if (NULL != f)
    	{
        	/* test if something has been outputed by 
           	the command */
        	if (EOF == fgetc(f))
        	{
			tst_resm(TFAIL, "USB Device not mounted");

        	}
        	else
        	{
			tst_resm(TPASS, "USB Device mounted!");
        	}        
        
		/* close the command file */
        	pclose(f);        
    	} 

	tst_resm(TINFO, "USB Mount Test Done!");
	tst_exit();

}

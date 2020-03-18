/* audio_playback_test.c

 * This tests includes playing a wav file using alsa utility
 * This needs alsa-utils to be present in the filesystem
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

#include <stdint.h>
#include <limits.h>
#include <sys/syscall.h>
#include <linux/futex.h>

#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>

#include "test.h"
#include "safe_macros.h"

char *TCID = "audio_playback_test";
int TST_TOTAL = 1;

static const option_t options[] = {
        {NULL, NULL, NULL},
        {NULL, NULL, NULL}
};

static void help(void)
{
}

void test_play_file(char *argv[])
{
	int retVal = 0;
	char *alsacmd = argv[1];
        char *path_to_file = argv[2];
        char cmd[1024];      	

	retVal = sprintf(cmd,"%s %s",alsacmd,path_to_file);
	if(retVal < 0)
	{
		tst_brkm(TFAIL, NULL, "Command Building Failed\n");	
	}
        
	printf("%s\n",cmd);
        retVal = system(cmd);
	if(retVal == -1)
	{
		tst_brkm(TFAIL, NULL, "Command Execution Failed\n");
	}

	tst_resm(TPASS, "Audio Playback Test Pass!");

}

void test_init(void)
{
	FILE *fd = popen("busybox lsmod | grep snd_bcm2835", "r");

        char buf[16];
        if (fread (buf, 1, sizeof (buf), fd) > 0) // if there is some result the module must be loaded
        {
		tst_resm(TINFO, "Audio Module Loaded!");
	}
        else
	{
		tst_brkm(TFAIL, NULL, "Audio Module snd_bcm2835 not loaded\n");	
	}

	fclose(fd);

}

int main(int argc, char *argv[])
{   
      	tst_parse_opts(argc, argv, options, help);

	tst_require_root();

      	test_init();

      	test_play_file(argv);	

      tst_resm(TINFO, "Audio Playback Tests Done!");

      
      tst_exit();
}

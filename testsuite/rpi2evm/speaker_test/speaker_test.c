/* speaker_test.c

 * This tests includes playing a sine wav tone using alsa utility
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

#define true 1
#define false 0

char *TCID = "speaker_test";

static const option_t options[] = {
        {NULL, NULL, NULL},
        {NULL, NULL, NULL}
};

static void help(void)
{
}


void test_speaker(int argc, char *argv[])
{
	int retVal = 0;
        char cmd[1024];
        char *speakertestcmd = "speaker-test";
	char *speakertestparam_1 = "-t";
	char *speakertestparam_2 = argv[1];
	char *speakertestparam_3 = "-f";
	char *speakertestparam_4 = "-l";
	char *speakertestparam_5 = "-c";
	char *speakertestparam_6 = argv[2];
	char *sinewavefreq = "2600";
	char *numofstreams = "6";
	unsigned char success = false;

	if((argc > 1) && (argc < 4))
	{
		if(strcmp(speakertestparam_2,"sine") == 0)
		{
			if(argc > 2)
			{
				retVal = sprintf(cmd , "%s %s %s %s %s %s %s",speakertestcmd , speakertestparam_1 , speakertestparam_2, speakertestparam_3,sinewavefreq , speakertestparam_4,speakertestparam_6);
				if(retVal < 0)
				{
					tst_brkm(TFAIL, NULL,"Command build failed\n");
				}else
				{
					success = true;
				}
			}else
			{
				tst_brkm(TFAIL, NULL,"Invalid Command1\n");
			}
		}else if(strcmp(speakertestparam_2,"pink") == 0)
		{
			if(argc > 2)
                        {
                                retVal = sprintf(cmd , "%s %s %s %s %s",speakertestcmd , speakertestparam_1 ,speakertestparam_2,speakertestparam_4,speakertestparam_6);
				if(retVal < 0)
                                {
                                        tst_brkm(TFAIL, NULL,"Command build failed\n");
                                }else
                                {
                                        success = true;
                                }

                        }else
                        {
                                tst_brkm(TFAIL, NULL,"Invalid Command\n");
                        }

		}else if(strcmp(speakertestparam_2,"wav") == 0)
		{
			if(argc > 2)
                        {
                                retVal = sprintf(cmd , "%s %s %s %s %s %s %s",speakertestcmd , speakertestparam_1 ,speakertestparam_2,speakertestparam_5,numofstreams,speakertestparam_4,speakertestparam_6);
				if(retVal < 0)
                                {
                                        tst_brkm(TFAIL, NULL,"Command build failed\n");
                                }else
                                {
                                        success = true;
                                }

                        }else
                        {
                                tst_brkm(TFAIL, NULL,"Invalid Command\n");
                        }

		}else
		{
			tst_brkm(TFAIL, NULL,"Invalid Command\n");
		}

	}else	
	{
		tst_brkm(TFAIL, NULL,"Invalid Command\n");
	}

	if(success)
	{
		retVal = system(cmd);
		if(retVal == -1)
		{
			tst_brkm(TFAIL, NULL,"Command Execution Failed\n");
		}

		tst_resm(TPASS, "Speaker Tests Pass!");
	}
}

void test_init(void)
{
	FILE *fd = popen("busybox lsmod | grep snd_bcm2835", "r");

        char buf[16];
        if (fread (buf, 1, sizeof (buf), fd) > 0) // if there is some result the module must be loaded
        {
		printf("\n* ========== Audio Module Loaded========== *\n");
	}
        else
	{
                printf ("module is not loaded\n");
		tst_brkm(TFAIL, NULL, "Audio Module snd_bcm2835 not loaded\n");	
	}

	fclose(fd);

}

int main(int argc, char *argv[])
{   
      tst_parse_opts(argc, argv, options, help);

	tst_require_root();

      test_init();

      test_speaker(argc,argv);	

      tst_resm(TINFO, "Speaker Tests Done!");

      
      tst_exit();
}

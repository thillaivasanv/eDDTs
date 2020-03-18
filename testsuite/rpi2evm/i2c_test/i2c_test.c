/* i2c_test.c

 * This test scans the i2c bus to check whether any
 * known slave is connected. The slave address can 
 * be passed as a command line parameter
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

#include <linux/i2c-dev.h>

/* I2C COMMANDS */

/* Test Data to I2C */

/* I2C Registers Specific to Temperature Sensor(Sense HAT) */

#define READ_BUFF 	0x2F
#define CTRL_REG1 	0x20
#define CTRL_REG2 	0x21
#define TEMP_OUT_L 	0x2A
#define TEMP_OUT_H 	0x2B
#define CNTL_REG4	0x1E
#define CNTL_REG5	0x1F
#define WHO_AM_I	0x0F

char *TCID = "i2c_test";

int i2c_fd = -1;

static char *i2c_dev ;
static char slave_addr;


static const option_t options[] = {
        {"d:", NULL, &i2c_dev},
        {NULL, NULL, NULL}
};

static void help(void)
{
        printf("  -d x    device node, default is %s\n",i2c_dev);
}

void test_init(void)
{
        if (access(i2c_dev, F_OK) == -1)
	{
                tst_brkm(TFAIL, NULL, "couldn't find i2c device");
	}

        i2c_fd = SAFE_OPEN(NULL,i2c_dev,O_RDWR);
 
        if(i2c_fd < 0 && errno != ENOENT)
        {
		tst_brkm(TFAIL, NULL, "i2c open failed");
		return;
        }

	if (SAFE_IOCTL(NULL,i2c_fd, I2C_SLAVE, slave_addr) < 0) {
    		 tst_brkm(TFAIL, NULL, "Failed to acquire bus access and/or talk to slave.\n");
	}

	tst_resm(TPASS, "I2C SCAN TEST PASSED\n\n");
}

void test_cleanup(void)
{

       if(i2c_fd > 0 && close(i2c_fd))
		tst_resm(TWARN | TERRNO,"close(i2c_fd)");
   
       close(i2c_fd);
       tst_resm(TINFO, "I2C Scan Test Done!\n\n");
}


int main(int argc, char *argv[])
{
        
      tst_parse_opts(argc, argv, options, help);

      tst_require_root();
     
      i2c_dev 	 = argv[1];
      slave_addr = strtol(argv[2],NULL,16);	

      test_init();
 
      test_cleanup(); 
      
      tst_exit();
}

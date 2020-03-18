/* eeprom_test.c

 * This test Executes read and write commands and also 
 * verifies device/interface integity by comparing
 * tests data used to write in the EEPROM with
 * data read back from the same location. This
 * preserves the existing data.

 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (c) HCL Technologies Limited, 2020
 
 * Author:titus.dhanasingh@hcl.com
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
#include <errno.h>
#include <time.h>

#include "test.h"
#include "safe_macros.h"

char *TCID = "eeprom_test";
int TST_TOTAL = 1;

static char *eeprom_dev = "/sys/bus/i2c/devices/1-0057/at24-1/nvmem";
static int dflag;
static const option_t options[] = {
	{" ", &dflag, &eeprom_dev},
	{NULL, NULL, NULL}
};

#define EEPROM_TEST_SIZE     256 
#define EEPROM_RDWR_LEN      32 

static void help(void)
{
	printf("  -d x    eeprom device node, default is %s\n",
		eeprom_dev);
}

void eeprom_read_write_test(void)
{
    char *eeprom_data_buffer1;
    char *eeprom_data_buffer2;
    int len;
    int rd_wr_len;

    /* Open MTD device file */
    int eeprom_fd = SAFE_OPEN(NULL, eeprom_dev, O_RDWR);

    if (eeprom_fd < 0) {
        tst_resm(TFAIL | TERRNO, "MTD device File open failed %s", eeprom_dev);
        return;
    }

    eeprom_data_buffer1 = (char *)malloc(EEPROM_TEST_SIZE);

    if (!eeprom_data_buffer1) {
       tst_resm(TFAIL , "Read Buffer Memory Allocation failed ...");
       goto resource_free;
    }

    eeprom_data_buffer2 = (char *)malloc(EEPROM_TEST_SIZE);

    if(!eeprom_data_buffer2) {
       tst_resm(TFAIL , "Read Buffer Memory Allocation failed ...");
       goto resource_free;
    }
	
    tst_resm(TINFO, "EEPROM READ-WRITE-READ TEST:");

    tst_resm(TINFO, "Read EEPROM Data %x ", EEPROM_TEST_SIZE);
    rd_wr_len = 0;
    while(rd_wr_len < EEPROM_TEST_SIZE) {

       len = read(eeprom_fd, eeprom_data_buffer1 + rd_wr_len, EEPROM_RDWR_LEN);

       if(len < EEPROM_RDWR_LEN) {
           printf("Len  %d @ %d\n",len, rd_wr_len);
           tst_resm(TFAIL , "Read EEPROM failed ...");
           goto resource_free;
       }
       rd_wr_len += EEPROM_RDWR_LEN;
    }


    tst_resm(TINFO, "Write EEPROM Data %x ", EEPROM_TEST_SIZE);

    /* Rewind to start position */
    lseek(eeprom_fd, 0, SEEK_SET);

    rd_wr_len = 0;
    while(rd_wr_len < EEPROM_TEST_SIZE) {
    
       len = write(eeprom_fd, eeprom_data_buffer1 + rd_wr_len, EEPROM_RDWR_LEN);

       if(len < EEPROM_RDWR_LEN) {
           printf("Len  %d\n",len);
           tst_resm(TFAIL , "Write EEPROM failed ...");
           goto resource_free;
       }
       rd_wr_len += EEPROM_RDWR_LEN;
    }

    /* Rewind to start position */
    lseek(eeprom_fd, 0, SEEK_SET);

    tst_resm(TINFO, "Read back EEPROM Data %x ", EEPROM_TEST_SIZE);
    rd_wr_len = 0;
    while(rd_wr_len < EEPROM_TEST_SIZE) {

       len = read(eeprom_fd, eeprom_data_buffer2 + rd_wr_len, EEPROM_RDWR_LEN);

       if(len < EEPROM_RDWR_LEN) {
           printf("Len  %d\n",len);
           tst_resm(TFAIL , "Read EEPROM failed ...");
           goto resource_free;
       }
       rd_wr_len += EEPROM_RDWR_LEN;
    }

    tst_resm(TINFO, "Compare EEPROM Data %x ", EEPROM_TEST_SIZE);
    if( memcmp(eeprom_data_buffer1,
               eeprom_data_buffer2, EEPROM_TEST_SIZE ) ) {
        tst_resm(TFAIL , "Read back data mismatch Block");
        goto resource_free;
    } 

    tst_resm(TPASS, "EEPROM READ TEST Passed");

resource_free:
             if(eeprom_data_buffer1)
                 free(eeprom_data_buffer1);
             if(eeprom_data_buffer2)
                 free(eeprom_data_buffer2);
             if(eeprom_fd > 0)
                 close(eeprom_fd);
}

int main(int argc, char *argv[])
{
	tst_parse_opts(argc, argv, options, help);

	tst_require_root();

	if (access(eeprom_dev, F_OK) == -1)
		tst_brkm(TCONF, NULL, "couldn't find eeprom device '%s'", eeprom_dev);

	/*Read Write test*/
	eeprom_read_write_test();


	tst_resm(TINFO, "EEPROM Tests Done!");
	tst_exit();
}

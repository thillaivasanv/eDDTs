/* norflash_test.c

 * This test Executes Erase, Read and Write operations 
 * over MTD partitions. Verifies device/interface
 * integrity by comparing tests data writtern to the device
 * with data read back from the same location. 
 * This tests preserves the existing data in the device.
 * Winbond 4M chip is used for the testing
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

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include "test.h"
#include "safe_macros.h"
#include <mtd/mtd-user.h>

#define SPIMTD_TEST_SIZE 4
#define SPIMTD_DEV_FILE  "/dev/mtd0"

char *TCID = "norflash_test";
int TST_TOTAL = 1;

static const option_t options[] = {
        {NULL, NULL, NULL},
        {NULL, NULL, NULL}
};


void spidev_mtd_test(char *dev_name)
{
    mtd_info_t mtd_info;           // the MTD structure
    erase_info_t ei;               // the erase block structure
    int ret;
    char *test_buffer_data1 = NULL;
    char *test_buffer_data2 = NULL;

    tst_resm(TINFO, "Initiating Erase, Write and Read test on %s", dev_name);

    /* Open MTD device file */
    int fd = SAFE_OPEN(NULL, dev_name, O_RDWR);

    if (fd < 0) {
        tst_resm(TFAIL | TERRNO, "MTD device File open failed %s",dev_name);
        return; 
    }

    /* Get the MTD partition Info */ 
    ret = ioctl(fd, MEMGETINFO, &mtd_info); 
    if (ret) {
        tst_resm(TFAIL | TERRNO, "IOCTL - MEMGETINFO failed for %s", dev_name);
        goto resource_free; 
    }

    test_buffer_data1 = (char *)malloc(mtd_info.erasesize * SPIMTD_TEST_SIZE);
  
    if (!test_buffer_data1) {
       tst_resm(TFAIL , "Write Buffer Memory Allocation failed ...");
       goto resource_free;
    }
   
    test_buffer_data2 = (char *)malloc(mtd_info.erasesize * SPIMTD_TEST_SIZE);
    
    if(!test_buffer_data2) {
       tst_resm(TFAIL , "Read Buffer Memory Allocation failed ...");
       goto resource_free;
    }

    /* Get erase block length */
    ei.length = mtd_info.erasesize; 

    lseek(fd, 0, SEEK_SET); /* Set to start position */

    /* Read data from the designated test bloaks */
    for (ei.start = 0; ei.start < (SPIMTD_TEST_SIZE * mtd_info.erasesize); ei.start += ei.length) {

        ret = read(fd, (test_buffer_data1 + ei.start), ei.length); /* Read back Test data */
        if (ret < ei.length) {
           tst_resm(TFAIL | TERRNO, "Read failed at offset %d", ei.start);
           goto resource_free;
        }

    }

    /* Erase test blocks */
    for(ei.start = 0; ei.start < (SPIMTD_TEST_SIZE * mtd_info.erasesize); ei.start += ei.length) {

        ret = ioctl(fd, MEMERASE, &ei);
	if (ret) {
           tst_resm(TFAIL | TERRNO, "IOCTL - MEMERASE failed st offset %d", ei.start);
	   goto resource_free;
        }
    }

    lseek(fd, 0, SEEK_SET); /* Rewind to start */

    memset(test_buffer_data1, 0xAA, (SPIMTD_TEST_SIZE * mtd_info.erasesize));

    for (ei.start = 0; ei.start < (SPIMTD_TEST_SIZE * mtd_info.erasesize); ei.start += ei.length) {

        ret = write(fd, (test_buffer_data1 + ei.start) , ei.length); /* Write Test data */
        if (ret < ei.length) {
           tst_resm(TFAIL | TERRNO, "Write failed at offset %d", ei.start);
	   goto resource_free;
        }
    }

    lseek(fd, 0, SEEK_SET); /* Rewind to start position */ 

    for (ei.start = 0; ei.start < (SPIMTD_TEST_SIZE * mtd_info.erasesize); ei.start += ei.length) {

        ret = read(fd, (test_buffer_data2 + ei.start), ei.length); /* Read back Test data */
        if (ret < ei.length) {
           tst_resm(TFAIL | TERRNO, "Read failed at offset %d", ei.start);
	   goto resource_free;
        }
    
    }

    /* compare test data read from each block */
    for (ei.start = 0; ei.start < (SPIMTD_TEST_SIZE * mtd_info.erasesize); ei.start += ei.length) {

        if( memcmp((test_buffer_data1 + mtd_info.erasesize),
                   (test_buffer_data2 + mtd_info.erasesize), mtd_info.erasesize) ) {
           tst_resm(TFAIL , "Read back data mismatch Block %d", (ei.start / mtd_info.erasesize));
	   goto resource_free;
        }
    }

    tst_resm(TPASS, "SPI MTD Erase, Write and Read Test passed .......!");

resource_free:
              if(test_buffer_data1)
		     free(test_buffer_data1);
	      if(test_buffer_data2)
                     free(test_buffer_data2);
	      if(fd > 0)
		      close(fd);

    return;
}

int main(int argc, char *argv[])
{
	tst_require_root();

	if (access(SPIMTD_DEV_FILE, F_OK) == -1)
		tst_brkm(TFAIL, NULL, "couldn't find spi device '%s'", SPIMTD_DEV_FILE);

	spidev_mtd_test(SPIMTD_DEV_FILE);

	tst_resm(TINFO, "SPI MTD DEV Tests Done-----------!");
	
	tst_exit();
}

/* spidev_test.c

 * This verifies SPI functionality by loopbacking
 * MISO and MOSI.(using spidev driver)

 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com> 
 * 
 * Modified by  :Arjun Kumar Rath <arjun-r@hcl.com>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

/*referred from spidev utuility*/

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "test.h"
#include "safe_macros.h"

int spi_fd = -1;
char *TCID = "spidev_test";
int TST_TOTAL = 1;

static char *spi_dev = "/dev/spidev1.0";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 500000;
static uint16_t delay;
static uint16_t words = 100; 

static int dflag;
static const option_t options[] = {
	{"d:", &dflag, &spi_dev},
	{NULL, NULL, NULL}
};

static void help(void)
{
	printf("  -d x    spi device node, default is %s\n",
		spi_dev);
}

void loopback_test(void)
{
	int ret;
  	uint8_t *tx;
  	uint8_t *rx;

	tst_resm(TINFO, "SPI DEV LOOPBACK TEST:");


  	tx = (uint8_t*)malloc(sizeof(uint8_t) * words);
  	rx = (uint8_t*)malloc(sizeof(uint8_t) * words);

  	if ((NULL == tx) || (NULL == rx)) 
	{
		tst_resm(TFAIL | TERRNO, "Memory allocation failed");
                return;
  	}
  
	struct spi_ioc_transfer tr = {
    		.tx_buf = (unsigned long)tx,
    		.rx_buf = (unsigned long)rx,
		.len = words,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
		};

	//write data
  	for (ret = 0; ret < words; ret++) 
	{
    		tx[ret] = ret % 256;
  	}

	tst_resm(TINFO, "Transmit Data");
  	
	for (ret = 0; ret < words; ret++) 
	{
    		if (!(ret % 6))
      			puts("");
//		tst_resm(TINFO,"%.2X ",tx[ret]);
		printf("%.2X ",tx[ret]);
  	}
  
	puts("");

	ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
	{
		tst_resm(TFAIL | TERRNO, "SPI transmit error");
                return;
	}

	tst_resm(TINFO, "Recieve Data");
	for (ret = 0; ret < words; ret++) 
	{
		if (!(ret % 6))
			puts("");
//		tst_resm(TINFO,"%.2X ",rx[ret]);
		printf("%.2X ",rx[ret]);

	}
	
	puts("");

	for (ret = 0; ret < words; ret++)
	{
		if(tx[ret] != rx[ret])
		{
			tst_brkm(TFAIL, NULL, "Loopback test failed for '%s'", spi_dev);
		}
	}

  	//free the memory allocated
  	free(tx);
  	free(rx);

	tst_resm(TPASS, "SPI DEV LOOPBACK TEST Passed");

}

void set_get_spi_params(void)
{
	int ret;

	tst_resm(TINFO, "SPI DEV SET PARAMS TEST:");
	/*
         * spi mode
         */
        ret = ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);
        if (ret == -1)
        {
                tst_resm(TFAIL | TERRNO, "SET SPI MODE ERROR");
                return;
        }

        ret = ioctl(spi_fd, SPI_IOC_RD_MODE, &mode);
        if (ret == -1)
        {
                tst_resm(TFAIL | TERRNO, "GET SPI MODE ERROR");
                return;

        }

        /*
         * bits per word
         */
        ret = ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
        if (ret == -1)
        {
                tst_resm(TFAIL | TERRNO, "SET SPI BITS PER WORD ERROR");
                return;
        }

        ret = ioctl(spi_fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
        if (ret == -1)
        {
                tst_resm(TFAIL | TERRNO, "GET SPI BITS PER WORD ERROR");
                return;
        }

	/*
         * max speed hz
         */
        ret = ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
        if (ret == -1)
        {
                tst_resm(TFAIL | TERRNO, "SET SPI MAX SPEED ERROR");
                return;
        }

        ret = ioctl(spi_fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
        if (ret == -1)
        {
                tst_resm(TFAIL | TERRNO, "GET SPI MAX SPEED ERROR");
                return;
        }

        tst_resm(TINFO,"spi mode: %d", mode);
        tst_resm(TINFO,"bits per word: %d", bits);
        tst_resm(TINFO,"max speed: %d Hz (%d KHz)", speed, speed/1000);
        tst_resm(TINFO,"number of words to be transfered: %d", words);

	tst_resm(TPASS, "SPI DEV SET PARAMS*********** TEST Passed");
}

int main(int argc, char *argv[])
{
	tst_parse_opts(argc, argv, options, help);

	tst_require_root();

	if (access(spi_dev, F_OK) == -1)
		tst_brkm(TFAIL, NULL, "couldn't find spi device '%s'", spi_dev);

	spi_fd = SAFE_OPEN(NULL, spi_dev, O_RDWR);

	set_get_spi_params();

	loopback_test();

	close(spi_fd);

	tst_resm(TINFO, "SPIDEV Tests Done-----------!");
	
	tst_exit();
}

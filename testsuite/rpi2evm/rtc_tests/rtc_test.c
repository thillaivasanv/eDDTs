/* rtc_test.c

 * This test checks the RTC functionality and compares the system time and RTC time .
 * This test was verified using DS2321 RTC chip . The chip does not support alarm 
 * functionality
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

#include "test.h"
#include "safe_macros.h"

int rtc_fd = -1;
char *TCID = "rtc_test";
int TST_TOTAL = 1;

static char *rtc_dev = "/dev/rtc0";
static int dflag;
static const option_t options[] = {
	{"d:", &dflag, &rtc_dev},
	{NULL, NULL, NULL}
};

static void help(void)
{
	printf("  -d x    rtc device node, default is %s\n",
		rtc_dev);
}

void rtc_read_test(void)
{
        struct rtc_time rtc_tm;
        int ret;

        tst_resm(TINFO, "RTC READ TEST:");

        /*Read RTC Time */
        ret = ioctl(rtc_fd, RTC_RD_TIME, &rtc_tm);
        if (ret == -1) {
                tst_resm(TFAIL | TERRNO, "RTC_RD_TIME ioctl failed");
                return;
        }

        tst_resm(TINFO, "Current RTC date/time is %d-%d-%d, %02d:%02d:%02d.",
                 rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year + 1900,
                 rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);

	tst_resm(TPASS, "RTC READ TEST Passed");

}

void rtc_system_time_compare_test(void)
{
	tst_resm(TINFO, "RTC & SYSTEM TIME COMPARE TEST:");

	//Read RTC time
	struct rtc_time rtc_tm;
        int ret;
	ret = ioctl(rtc_fd, RTC_RD_TIME, &rtc_tm);
        if (ret == -1) {
                tst_resm(TFAIL | TERRNO, "RTC_RD_TIME ioctl failed");
                return;
        }

	tst_resm(TINFO,"RTC time is %02d:%02d:%02d",rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);

	//Read system time
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
    	printf("system time %s", asctime(tm));
	tst_resm(TINFO,"System time is %02d:%02d:%02d",tm->tm_hour, tm->tm_min, tm->tm_sec);

	if((rtc_tm.tm_hour == tm->tm_hour) && 
		(rtc_tm.tm_min == tm->tm_min) && 
		((tm->tm_sec >= (rtc_tm.tm_sec-10)) && (tm->tm_sec <= rtc_tm.tm_sec + 10)))	
	{
		tst_resm(TPASS, "RTC & SYSTEM COMPARE TEST Passed");
	}else
	{
		tst_brkm(TCONF, NULL,"RTC & SYSTEM COMPARE TEST Failed");
		return;
	}

}

void rtc_alarm_test(void)
{
	struct rtc_time rtc_tm;
        int ret;
        unsigned long data;
        fd_set rfds;
        struct timeval tv;


	tst_resm(TINFO, "RTC ALARM TEST :");

	/* Enable alarm interrupts */
        ret = ioctl(rtc_fd, RTC_AIE_ON, 0);
        if (ret == -1) {
                tst_resm(TINFO | TERRNO, "ALARM NOT SUPPORTED");
                return;
        }else
	{
		/* Disable alarm interrupts */
        	ret = ioctl(rtc_fd, RTC_AIE_OFF, 0);
        	if (ret == -1) {
                	tst_resm(TFAIL | TERRNO, "RTC_AIE_OFF ioctl failed");
                	return;
        	}


        	/*set Alarm to 5 Seconds */
	        rtc_tm.tm_sec += 5;
        	if (rtc_tm.tm_sec >= 60) {
                	rtc_tm.tm_sec %= 60;
	                rtc_tm.tm_min++;
        	}

        	if (rtc_tm.tm_min == 60) {
                	rtc_tm.tm_min = 0;
                	rtc_tm.tm_hour++;
        	}

        	if (rtc_tm.tm_hour == 24)
                	rtc_tm.tm_hour = 0;

        	ret = ioctl(rtc_fd, RTC_ALM_SET, &rtc_tm);
        	if (ret == -1) {
                	if (errno == EINVAL)
                        	tst_resm(TCONF | TERRNO, "RTC_ALM_SET not supported");
                	else
                        	tst_resm(TFAIL | TERRNO, "RTC_ALM_SET ioctl failed");
                	return;
        	}	

		/*Read current alarm time */
        	ret = ioctl(rtc_fd, RTC_ALM_READ, &rtc_tm);
        	if (ret == -1) {
                	if (errno == EINVAL) {
                        	tst_resm(TCONF | TERRNO, "RTC_ALM_READ not suported");
                	} else {
                        	tst_resm(TFAIL | TERRNO, "RTC_ALM_READ ioctl failed");
                        	return;
                	}
        	} else {
                	tst_resm(TINFO, "Alarm time set to %02d:%02d:%02d.",
                         	rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);
        	}

        	/* Enable alarm interrupts */
        	ret = ioctl(rtc_fd, RTC_AIE_ON, 0);
        	if (ret == -1) {
                	tst_resm(TINFO | TERRNO, "RTC_AIE_ON ioctl failed");
                	return;
        	}

        	tst_resm(TINFO, "Waiting 5 seconds for the alarm...");

        	tv.tv_sec = 6;          /*set 6 seconds as the time out */
        	tv.tv_usec = 0;

        	FD_ZERO(&rfds);
        	FD_SET(rtc_fd, &rfds);

        	ret = select(rtc_fd + 1, &rfds, NULL, NULL, &tv);       /*wait for alarm */

		if (ret == -1) {
                	tst_resm(TFAIL | TERRNO, "select failed");
                	return;
        	} else if (ret) {
                	ret = read(rtc_fd, &data, sizeof(unsigned long));
                	if (ret == -1) {
                        	tst_resm(TFAIL | TERRNO, "read failed");
                        	return;
                	}
                	tst_resm(TINFO, "Alarm rang.");
        	} else {
                	tst_resm(TFAIL, "Timed out waiting for the alarm");
                	return;
        	}

        	/* Disable alarm interrupts */
        	ret = ioctl(rtc_fd, RTC_AIE_OFF, 0);
        	if (ret == -1) {
                	tst_resm(TFAIL | TERRNO, "RTC_AIE_OFF ioctl failed");
                	return;
        	}
        	tst_resm(TPASS, "RTC ALARM TEST Passed");
	}
}

int main(int argc, char *argv[])
{
	tst_parse_opts(argc, argv, options, help);

	tst_require_root();

	if (access(rtc_dev, F_OK) == -1)
		tst_brkm(TCONF, NULL, "couldn't find rtc device '%s'", rtc_dev);

	rtc_fd = SAFE_OPEN(NULL, rtc_dev, O_RDONLY);

	/*Read test*/
	rtc_read_test();

	/*Compare test*/
	rtc_system_time_compare_test();

	/*alarm set*/
	rtc_alarm_test();

	close(rtc_fd);

	tst_resm(TINFO, "RTC Tests Done!");
	tst_exit();
}

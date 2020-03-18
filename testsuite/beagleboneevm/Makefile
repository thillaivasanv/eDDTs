#
#    Copyright (C) 2009, Cisco Systems Inc.
#    Copyright (C) 2010, Linux Test Project.
#    Copyright (C) 2020 , HCL TECHNOLOGIES LIMITED

#    Author : Arjun Kumar Rath (arjun-r@hcl.com)
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along
#    with this program; if not, write to the Free Software Foundation, Inc.,
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#

top_srcdir		?= ../../ltp

include $(top_srcdir)/include/mk/env_pre.mk

SUBDIRS                 := cpu_core_test \
			   eeprom_test \
                           eth_test \
                           gpio_test \
                           i2c_test \
			   norflash_test \
                           rtc_tests \
                           spidev_test \
                           temp_sensor_test \
                           usb_test \
                           wdt_test

all:
	for dir in $(SUBDIRS); do \
                cd $$dir; $(MAKE); cd ..;\
        done

install:
	for dir in $(SUBDIRS); do \
                cd $$dir; $(MAKE) install; cd ..;\
	cp -f bbb_test_cmdline $(DESTDIR)/$(prefix)/runtest/;   \
        done

clean:
	for dir in $(SUBDIRS); do \
                cd $$dir; $(MAKE) clean; cd ..;\
        done


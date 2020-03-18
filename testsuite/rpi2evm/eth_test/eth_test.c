/* eth_test.c

 * This verifies and displays the hostname,ip address
 * gateway ip address , mac address and tests connectivity 
 * to a known IP adress. This is tested in rpi 2 evm
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <net/if.h>


#include "test.h"
#include "safe_macros.h"

#define BUFFER_SIZE 4096
#define PORT	8080
#define SERVER_IP "10.144.172.135"

char *TCID = "eth_test";
int TST_TOTAL = 1;

static const option_t options[] = {
	{NULL, NULL, NULL},
	{NULL, NULL, NULL}
};

static void help(void)
{
}

void server_connection_test(void)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(sockfd < 0)
        {
                tst_brkm(TFAIL, NULL,"server_connection_test-> Socket open failed");
        }

	struct sockaddr_in addr = { AF_INET, htons(PORT), inet_addr(SERVER_IP) };
        if (connect(sockfd, (struct sockaddr *) &addr, sizeof(addr)) != 0) 
	{
		tst_brkm(TFAIL, NULL,"Connect to server Failed");	
	}
        
	close(sockfd);

}

int getgatewayandiface(in_addr_t * addr, char *interface)
{
	long destination, gateway;
    	char iface[IF_NAMESIZE];
    	char buf[BUFFER_SIZE];
    	FILE * file;
    
    	memset(iface, 0, sizeof(iface));
    	memset(buf, 0, sizeof(buf));
    
    	file = fopen("/proc/net/route", "r");
    	if (!file)
	{
		tst_brkm(TFAIL, NULL,"/proc/net/route open failed" );
	} 

    	while (fgets(buf, sizeof(buf), file)) 
	{
        	if (sscanf(buf, "%s %lx %lx", iface, &destination, &gateway) == 3) 
		{
            		if (destination == 0) 
			{ /* default */
                		*addr = gateway;
                		strcpy(interface, iface);
                		fclose(file);
                		return 0;
            		}
        	}
    	}
    
    	/* default route not found */
    	if (file)
        	fclose(file);
    
	tst_brkm(TFAIL, NULL,"Gateway test Failed");
}


void gatewayaddr_test(void)
{
	in_addr_t addr = 0;
    	char iface[IF_NAMESIZE];

    	memset(iface, 0, sizeof(iface));

    	getgatewayandiface(&addr, iface);

	tst_resm(TPASS,"Interface:%s Gateway address: %s",iface, inet_ntoa(*(struct in_addr *) &addr));
}

void macaddr_test(void)
{
	struct ifreq s;

	char macStr[18];
  	
	int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	
	if(fd < 0)
	{
		tst_brkm(TFAIL, NULL,"macaddr_test -> Socket open failed");
	}

  	strcpy(s.ifr_name, "eth0");
  	if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) 
	{
			snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
         					(unsigned char) s.ifr_addr.sa_data[0], 
						(unsigned char) s.ifr_addr.sa_data[1], 
						(unsigned char) s.ifr_addr.sa_data[2], 
						(unsigned char) s.ifr_addr.sa_data[3],
						(unsigned char) s.ifr_addr.sa_data[4], 
						(unsigned char) s.ifr_addr.sa_data[5]); 
			
			tst_resm(TPASS,"Mac Addr is %s",macStr);
  	}else
	{
		tst_brkm(TFAIL, NULL,"macaddr_test -> IOCTL Failed");
	}

}

void ipaddress_test(void)
{
	int fd;

	int ret = 0;
    	
	struct ifreq ifr;

    	fd = socket(AF_INET, SOCK_DGRAM, 0);
	
	if(fd < 0)
	{
		tst_brkm(TFAIL, NULL,"ipaddress_test -> Socket open failed");
	}

    	/* I want to get an IPv4 IP address */
    	ifr.ifr_addr.sa_family = AF_INET;

    	/* I want an IP address attached to "eth0" */
    	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

    	ret = ioctl(fd, SIOCGIFADDR, &ifr);

	if(ret == -1)
	{
		tst_brkm(TFAIL, NULL,"ipaddress_test -> IOCTL Failed");
	}
	
    	close(fd);

	tst_resm(TPASS, "IPADDR : %s",inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

    	/* Display result */
}
void hostname_test(void)
{
	char hostbuffer[256];
    	struct hostent *host_entry;
    	int hostname;

    	// To retrieve hostname 
    	hostname = gethostname(hostbuffer, sizeof(hostbuffer));
	if (hostname == -1)
    	{
		tst_brkm(TCONF, NULL,"Get Host Name Failed");
    	}

	// To retrieve host information 
        host_entry = gethostbyname(hostbuffer);
	if(host_entry == NULL)
	{
		tst_brkm(TCONF, NULL,"Get Host by Name Failed");
	}

	tst_resm(TPASS, "Hostname : %s",hostbuffer);
}

int main(int argc, char *argv[])
{
	tst_parse_opts(argc, argv, options, help);

	tst_require_root();

	//Test Hostname
	hostname_test();

	//Test IP address
	ipaddress_test();

	//Mac address test
	macaddr_test();

	//Gateway address test
	gatewayaddr_test();	

	//Check server connectivity	
	server_connection_test();

	tst_resm(TPASS, "Ethernet Tests Pass!");

	tst_resm(TINFO, "Ethernet Tests Done!");
	tst_exit();
}

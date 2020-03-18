/* eth_test.c

 * This test Executes series test suites verifiying ethernet interface's link 
 *  and IP connectivity.This is tested in beagle bone black evm
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (c) HCL Technologies Limited, 2020
 
 * Author:Arjun Kumar Rath <arjun-r@hcl.com>
 * Modified By:titus.dhanasingh@hcl.com 
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
#include <linux/types.h>
#include <linux/sockios.h>
#include <linux/mii.h>

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

/* This data structure is used for all the MII ioctl's */
struct mii_data {
    __u16	phy_id;
    __u16	reg_num;
    __u16	val_in;
    __u16	val_out;
};

#define ETH_INTF_NAME0 "eth0"
static int skfd = -1;		/* AF_INET socket for ioctl() calls. */
static int nway_advertise = 0;

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

static struct ifreq ifr;

#define MII_ANAR   0x04
#define MII_BMCR   0x00
#define MII_BMSR   0x01

#define ANEG_ENABLED     1
#define ANADVERT_ENABLED 1

#define SPEED_10 10
#define SPEED_100 100
#define SPEED_1000 1000
#define SPEED_10000 10000

#define ADVERTISED_10baseT_Half         (1 << 0)
#define ADVERTISED_10baseT_Full         (1 << 1)
#define ADVERTISED_100baseT_Half        (1 << 2)
#define ADVERTISED_100baseT_Full        (1 << 3)


int eth_set_and_test_speed_duplex(uint32_t speed, uint32_t duplex, char *intf_name)
{
    int32_t fd;
    struct ifreq eth_ifr;
    int32_t ret = 0;
    struct ethtool_cmd ecmd;
    int32_t reg_speed_duplex = 0;

    /* Open a basic socket. */
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        fprintf(stdout,"Socket open failed ");
        return -1;
    }

    memset(&eth_ifr, 0, sizeof(eth_ifr));
    strcpy(eth_ifr.ifr_name, intf_name);

    memset(&ecmd, 0, sizeof(struct ethtool_cmd)); 
    /* Pass the "get info" command to eth tool driver	*/
    ecmd.cmd = ETHTOOL_GSET;
    eth_ifr.ifr_data = (caddr_t)&ecmd;
    ret = ioctl(fd, SIOCETHTOOL, &eth_ifr);

    /* ioctl failed: */
    if (ret != 0) {
        fprintf(stdout,"111-Cannot get device settings");
        return -1;
    }

    if (speed == SPEED_10 && duplex == DUPLEX_HALF) {
        ecmd.advertising = ADVERTISED_10baseT_Half;
    } else if (speed == SPEED_10 && duplex == DUPLEX_FULL) {
        ecmd.advertising = ADVERTISED_10baseT_Full;
    } else if (speed == SPEED_100 && duplex == DUPLEX_HALF) {
        ecmd.advertising = ADVERTISED_100baseT_Half;
    } else if (speed == SPEED_100 && duplex == DUPLEX_FULL) {
        ecmd.advertising = ADVERTISED_100baseT_Full;
    } else {
	   fprintf(stdout,"Unsupported Speed Duplex ");
           return -1;
    }

    /*  Pass the "set info" command to eth tool driver  */
    ecmd.cmd = ETHTOOL_SSET;
    eth_ifr.ifr_data = (caddr_t)&ecmd;
    ret = ioctl(fd, SIOCETHTOOL, &eth_ifr);

    /* ioctl failed: */
    if (ret != 0) {
        fprintf(stdout,"222-Cannot get device settings");
        return -1;
    }

    sleep(5);

    /* Pass the "get info" command to eth tool driver   */
    ecmd.cmd = ETHTOOL_GSET;
    eth_ifr.ifr_data = (caddr_t)&ecmd;
    ret = ioctl(fd, SIOCETHTOOL, &eth_ifr);

    /* ioctl failed: */
    if (ret != 0) {
        fprintf(stdout,"Cannot get device settings");
        return -1;
    }

    if(ecmd.speed != speed || ecmd.duplex != duplex) {
       fprintf(stdout,"Speed/Duplex Mismatch");
       return -1;
    }

    return 0;
}


void ethernet_speed_duplex_AN_test()
{

    if(eth_set_and_test_speed_duplex(SPEED_10, DUPLEX_HALF, ETH_INTF_NAME0) < 0) {
       tst_brkm(TFAIL, NULL,"Speed-10 Duplex-HALF test failed");  
    }

    if(eth_set_and_test_speed_duplex(SPEED_10, DUPLEX_FULL, ETH_INTF_NAME0) < 0) {
       tst_brkm(TFAIL, NULL,"Speed-10 Duplex-FULL test failed");
    }

    if(eth_set_and_test_speed_duplex(SPEED_100, DUPLEX_HALF, ETH_INTF_NAME0) < 0) {
       tst_brkm(TFAIL, NULL,"Speed-100 Duplex-HALF test failed");
    }

    if(eth_set_and_test_speed_duplex(SPEED_100, DUPLEX_FULL, ETH_INTF_NAME0) < 0) {
       tst_brkm(TFAIL, NULL,"Speed-100 Duplex-FULL test failed");
    }

    tst_resm(TPASS, "Speed Test passed");

    return;
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

	// Speed, duplex and AN  test

	ethernet_speed_duplex_AN_test();

	tst_resm(TPASS, "Ethernet Tests Pass!");

	tst_resm(TINFO, "Ethernet Tests Done!");
	tst_exit();
}

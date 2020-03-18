/* dma_test.c

 * This test verifies copying memory from one region to another using DMA in user space
 * Copies the string "hello world" from one place in memory to another through the use 
 * of the Raspberry Pi's DMA peripheral.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * https://github.com/Wallacoloo/Raspberry-Pi-DMA-Example : DMA Raspberry Pi Examples
 * Author: Colin Wallace
 * Modified By:Arjun Kumar Rath <arjun-r@hcl.com>
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

#include <sys/mman.h> //for mmap
#include <unistd.h> //for NULL
#include <stdio.h> //for printf
#include <stdlib.h> //for exit
#include <fcntl.h> //for file opening
#include <stdint.h> //for uint32_t
#include <string.h> //for memset

#include "test.h"
#include "safe_macros.h"

#include "hw-addresses.h" // for DMA addresses, etc.

#define PAGE_SIZE 4096 //mmap maps pages of memory, so we must give it multiples of this size


//-------- Relative offsets for DMA registers
//DMA Channel register sets (format of these registers is found in DmaChannelHeader struct):
#define DMACH(n) (0x100*(n))

//Each DMA channel has some associated registers, but only CS (control and status), CONBLK_AD (control block address), and DEBUG are writeable
//DMA is started by writing address of the first Control Block to the DMA channel's CONBLK_AD register and then setting the ACTIVE bit inside the CS register (bit 0)
//Note: DMA channels are connected directly to peripherals, so physical addresses should be used (affects control block's SOURCE, DEST and NEXTCONBK addresses).

#define DMAENABLE 0x00000ff0 //bit 0 should be set to 1 to enable channel 0. bit 1 enables channel 1, etc.

//flags used in the DmaChannelHeader struct:
#define DMA_CS_RESET (1<<31)
#define DMA_CS_ACTIVE (1<<0)

#define DMA_DEBUG_READ_ERROR (1<<2)
#define DMA_DEBUG_FIFO_ERROR (1<<1)
#define DMA_DEBUG_READ_LAST_NOT_SET_ERROR (1<<0)

//flags used in the DmaControlBlock struct:
#define DMA_CB_TI_DEST_INC (1<<4)
#define DMA_CB_TI_SRC_INC (1<<8)

int memfd = -1;
char *TCID = "dma_test";
int TST_TOTAL = 1;

static char *mem_dev = "/dev/mem";
static int dflag;
static const option_t options[] = {
	{"d:", &dflag, &mem_dev},
	{NULL, NULL, NULL}
};

static void help(void)
{
	printf("  -d x    mem device node, default is %s\n",
		mem_dev);
}


//set bits designated by (mask) at the address (dest) to (value), without affecting the other bits
//eg if x = 0b11001100
//  writeBitmasked(&x, 0b00000110, 0b11110011),
//  then x now = 0b11001110
void writeBitmasked(volatile uint32_t *dest, uint32_t mask, uint32_t value) 
{
	uint32_t cur = *dest;
    	uint32_t new = (cur & (~mask)) | (value & mask);
    	*dest = new;
    	*dest = new; //added safety for when crossing memory barriers.
}

struct DmaChannelHeader 
{
    	uint32_t CS; //Control and Status
        	//31    RESET; set to 1 to reset DMA
        	//30    ABORT; set to 1 to abort current DMA control block (next one will be loaded & continue)
        	//29    DISDEBUG; set to 1 and DMA won't be paused when debug signal is sent
        	//28    WAIT_FOR_OUTSTANDING_WRITES; set to 1 and DMA will wait until peripheral says all writes have gone through before loading next CB
        	//24-27 reserved
        	//20-23 PANIC_PRIORITY; 0 is lowest priority
        	//16-19 PRIORITY; bus scheduling priority. 0 is lowest
        	//9-15  reserved
        	//8     ERROR; read as 1 when error is encountered. error can be found in DEBUG register.
        	//7     reserved
        	//6     WAITING_FOR_OUTSTANDING_WRITES; read as 1 when waiting for outstanding writes
        	//5     DREQ_STOPS_DMA; read as 1 if DREQ is currently preventing DMA
        	//4     PAUSED; read as 1 if DMA is paused
        	//3     DREQ; copy of the data request signal from the peripheral, if DREQ is enabled. reads as 1 if data is being requested, else 0
        	//2     INT; set when current CB ends and its INTEN=1. Write a 1 to this register to clear it
        	//1     END; set when the transfer defined by current CB is complete. Write 1 to clear.
        	//0     ACTIVE; write 1 to activate DMA (load the CB before hand)
    	uint32_t CONBLK_AD; //Control Block Address
    	uint32_t TI; //transfer information; see DmaControlBlock.TI for description
    	uint32_t SOURCE_AD; //Source address
    	uint32_t DEST_AD; //Destination address
    	uint32_t TXFR_LEN; //transfer length.
    	uint32_t STRIDE; //2D Mode Stride. Only used if TI.TDMODE = 1
    	uint32_t NEXTCONBK; //Next control block. Must be 256-bit aligned (32 bytes; 8 words)
    	uint32_t DEBUG; //controls debug settings
};

struct DmaControlBlock 
{
	uint32_t TI; //transfer information
        	//31:27 unused
        	//26    NO_WIDE_BURSTS
        	//21:25 WAITS; number of cycles to wait between each DMA read/write operation
        	//16:20 PERMAP; peripheral number to be used for DREQ signal (pacing). set to 0 for unpaced DMA.
        	//12:15 BURST_LENGTH
        	//11    SRC_IGNORE; set to 1 to not perform reads. Used to manually fill caches
        	//10    SRC_DREQ; set to 1 to have the DREQ from PERMAP gate requests.
        	//9     SRC_WIDTH; set to 1 for 128-bit moves, 0 for 32-bit moves
        	//8     SRC_INC;   set to 1 to automatically increment the source address after each read (you'll want this if you're copying a range of memory)
        	//7     DEST_IGNORE; set to 1 to not perform writes.
        	//6     DEST_DREG; set to 1 to have the DREQ from PERMAP gate *writes*
        	//5     DEST_WIDTH; set to 1 for 128-bit moves, 0 for 32-bit moves
        	//4     DEST_INC;   set to 1 to automatically increment the destination address after each read (Tyou'll want this if you're copying a range of memory)
        	//3     WAIT_RESP; make DMA wait for a response from the peripheral during each write. Ensures multiple writes don't get stacked in the pipeline
        	//2     unused (0)
        	//1     TDMODE; set to 1 to enable 2D mode
        	//0     INTEN;  set to 1 to generate an interrupt upon completion
    	uint32_t SOURCE_AD; //Source address
    	uint32_t DEST_AD; //Destination address
    	uint32_t TXFR_LEN; //transfer length.
    	uint32_t STRIDE; //2D Mode Stride. Only used if TI.TDMODE = 1
    	uint32_t NEXTCONBK; //Next control block. Must be 256-bit aligned (32 bytes; 8 words)
    	uint32_t _reserved[2];
};

//allocate a page & simultaneously determine its physical address.
//virtAddr and physAddr are essentially passed by-reference.
//this allows for:
//void *virt, *phys;
//makeVirtPhysPage(&virt, &phys)
//now, virt[N] exists for 0 <= N < PAGE_SIZE,
//  and phys+N is the physical address for virt[N]
//based on http://www.raspians.com/turning-the-raspberry-pi-into-an-fm-transmitter/
void makeVirtPhysPage(void** virtAddr, void** physAddr) 
{
	int numofbytes;
	
    	*virtAddr = valloc(PAGE_SIZE); //allocate one page of RAM

    	//force page into RAM and then lock it there:
    	((int*)*virtAddr)[0] = 1;
    
	mlock(*virtAddr, PAGE_SIZE);
    	
	memset(*virtAddr, 0, PAGE_SIZE); //zero-fill the page for convenience

    
	//Magic to determine the physical address for this page:
    	uint64_t pageInfo;
    
	int file = open("/proc/self/pagemap", 'r');

	if(file < 0)
	{
		tst_brkm(TCONF, NULL,"Failed to open /proc/self/pagemap\n");
	}

    	lseek(file, ((size_t)*virtAddr)/PAGE_SIZE*8, SEEK_SET);
    
	numofbytes = read(file, &pageInfo, 8);

	if(numofbytes != 8)
	{
		tst_brkm(TCONF, NULL,"Pagemap read failed\n");
	}

    
	*physAddr = (void*)(size_t)(pageInfo*PAGE_SIZE);
//    printf("makeVirtPhysPage virtual to phys: %p -> %p\n", *virtAddr, *physAddr);
	
	tst_resm(TINFO,"makeVirtPhysPage virtual to phys: %p -> %p\n",*virtAddr, *physAddr);
	
}

//call with virtual address to deallocate a page allocated with makeVirtPhysPage
void freeVirtPhysPage(void* virtAddr) 
{
	munlock(virtAddr, PAGE_SIZE);
    	free(virtAddr);
}

//map a physical address into our virtual address space. memfd is the file descriptor for /dev/mem
volatile uint32_t* mapPeripheral(int memfd, int addr) 
{
    	///dev/mem behaves as a file. We need to map that file into memory:
    	void *mapped = mmap(NULL, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, memfd, addr);
    
	//now, *mapped = memory at physical address of addr.
    	if (mapped == MAP_FAILED) 
	{
        //	printf("failed to map memory (did you remember to run as root?)\n");
        //	exit(1);
		tst_brkm(TCONF, NULL,"Failed to map memory\n");
    	} else 
	{
//        	printf("mapped: %p\n", mapped);
		tst_resm(TINFO,"Memory mapped to %p",mapped);
    	}
    
	return (volatile uint32_t*)mapped;
}

int main(int argc, char *argv[])
{
        tst_parse_opts(argc, argv, options, help);

        tst_require_root();

	tst_resm(TINFO, "DMA Memory Copy Tests!");

    	//cat /sys/module/dma/parameters/dmachans gives a bitmask of DMA channels that are not used by GPU. Results: ch 1, 3, 6, 7 are reserved.
    	//dmesg | grep "DMA"; results: Ch 2 is used by SDHC host
    	//ch 0 is known to be used for graphics acceleration
    	//Thus, applications can use ch 4, 5, or the LITE channels @ 8 and beyond.
    //	int dmaChNum = 5;

	int dmaChNum = atoi(argv[2]);

    	//First, open the linux device, /dev/mem
    	//dev/mem provides access to the physical memory of the entire processor+ram
    	//This is needed because Linux uses virtual memory, thus the process's memory at 0x00000000 will NOT have the same contents as the physical    //memory at 0x00000000
//    	int memfd = open("/dev/mem", O_RDWR | O_SYNC);
//	memfd = open("/dev/mem", O_RDWR | O_SYNC);
    
//	if (memfd < 0) 
  //  	{
    //    	printf("Failed to open /dev/mem (did you remember to run as root?)\n");
    //    	exit(1);
   // 	}

	if (access(mem_dev, F_OK) == -1)
		tst_brkm(TCONF, NULL, "couldn't find mem device '%s'", mem_dev);

	memfd = SAFE_OPEN(NULL, mem_dev, O_RDWR | O_SYNC);
    
	//now map /dev/mem into memory, but only map specific peripheral sections:
    	volatile uint32_t *dmaBaseMem = mapPeripheral(memfd, DMA_BASE);
    
    	//configure DMA:
    	//allocate 1 page for the source and 1 page for the destination:
    	void *virtSrcPage, *physSrcPage;
    	
	makeVirtPhysPage(&virtSrcPage, &physSrcPage);
    
	void *virtDestPage, *physDestPage;
    
	makeVirtPhysPage(&virtDestPage, &physDestPage);
    
    	//write a few bytes to the source page:
    	char *srcArray = (char*)virtSrcPage;
    	srcArray[0]  = 'h';
    	srcArray[1]  = 'e';
    	srcArray[2]  = 'l';
    	srcArray[3]  = 'l';
    	srcArray[4]  = 'o';
    	srcArray[5]  = ' ';
    	srcArray[6]  = 'w';
    	srcArray[7]  = 'o';
    	srcArray[8]  = 'r';
    	srcArray[9]  = 'l';
    	srcArray[10] = 'd';
    	srcArray[11] =  0; //null terminator used for printf call.
    
    	//allocate 1 page for the control blocks
    	void *virtCbPage, *physCbPage;
    	makeVirtPhysPage(&virtCbPage, &physCbPage);
    
    	//dedicate the first 8 words of this page to holding the cb.
    	struct DmaControlBlock *cb1 = (struct DmaControlBlock*)virtCbPage;
    
    	//fill the control block:
    	cb1->TI = DMA_CB_TI_SRC_INC | DMA_CB_TI_DEST_INC; //after each byte copied, we want to increment the source and destination address of the copy, otherwise we'll be copying to the same address.
    	cb1->SOURCE_AD = (uint32_t)physSrcPage; //set source and destination DMA address
    	cb1->DEST_AD = (uint32_t)physDestPage;
    	cb1->TXFR_LEN = 12; //transfer 12 bytes
    	cb1->STRIDE = 0; //no 2D stride
    	cb1->NEXTCONBK = 0; //no next control block
    
//    	printf("destination was initially: '%s'\n", (char*)virtDestPage);
	tst_resm(TINFO,"Initial Destination Read %s\n",(char*)virtDestPage);
    
    	//enable DMA channel (it's probably already enabled, but we want to be sure):
    	writeBitmasked(dmaBaseMem + DMAENABLE/4, 1 << dmaChNum, 1 << dmaChNum);
    
    	//configure the DMA header to point to our control block:
    	volatile struct DmaChannelHeader *dmaHeader = 
				(volatile struct DmaChannelHeader*)(dmaBaseMem + (DMACH(dmaChNum))/4); //dmaBaseMem is a uint32_t ptr, so divide by 4 before adding byte offset
    
	dmaHeader->CS = DMA_CS_RESET; //make sure to disable dma first.
    	sleep(1); //give time for the reset command to be handled.
    	dmaHeader->DEBUG = DMA_DEBUG_READ_ERROR | DMA_DEBUG_FIFO_ERROR | DMA_DEBUG_READ_LAST_NOT_SET_ERROR; // clear debug error flags
    	dmaHeader->CONBLK_AD = (uint32_t)physCbPage; //we have to point it to the PHYSICAL address of the control block (cb1)
    	dmaHeader->CS = DMA_CS_ACTIVE; //set active bit, but everything else is 0.
    
    	sleep(1); //give time for copy to happen
    
//    	printf("destination reads: '%s'\n", (char*)virtDestPage);
	tst_resm(TINFO,"Now Destination Reads %s\n",(char*)virtDestPage);

	if(strcmp((char*)virtDestPage , "hello world") == 0)
	{
		tst_resm(TFAIL, "DMA TEST Failed");
	}else
	{
		tst_resm(TPASS, "DMA TEST Passed");
	}
    
    	//cleanup
    	freeVirtPhysPage(virtCbPage);
    	freeVirtPhysPage(virtDestPage);
    	freeVirtPhysPage(virtSrcPage);

	close(memfd);

	tst_resm(TINFO, "DMA Tests Done!");
	tst_exit();
    
	return 0;
}

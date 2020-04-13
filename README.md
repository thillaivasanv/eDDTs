# eDDTs Objective # (Embedded Device Driver Test Suite)

=================================================================

eDDTs is a test suite specifically designed for embedded Linux
device driver testing.

There are numerous aspects of testing that are still done in an ad-hoc and
company-specific way. Although there are open source test programs
(such as cylictest, LTP, linuxbench, etc.), there are lots of aspects of
Linux testing that are not shared and once such is device drivers testing

The purpose of eDDTs is to provide a test suite for testing device drivers for
multiple platforms covering different device technologies (e.g memory,sensors,
networking etc.)in  embedded Linux, that is distributed and allows individuals
and organizations to easily run the available test cases , create and run their
own tests,and at the same time allows people to share their tests and test results
with each other.

Below are some of the key objectives and milestones listed

* Support for multiple platforms
* Covering device test cases of different technologies (e.g memory,sensors,networking etc.)
* User Interface to execute test
* Test cases reusability across platforms


# eDDTs Overview (Embedded Device Driver Test Suite) #

================================================================

eDDTs is a test framework to test and validate various device drivers ------
based on embedded linux.

It is based on LTP (Linux Test Project)

LTP validates many kernel areas, such as memory management, scheduler 
and system calls. 

eDDTs extends LTP's core Kernel tests with tests to validate different 
device drivers. eDDTs focuses on embedded device driver tests. It contains 
number of tests that validate interface and functionality of device drivers.

eDDTs uses LTP's test infrastructure, such as

* Test execution drivers (PAN)
* Top-level test scripts (i.e. runltp)
* Same Folder Hierarchy and test case definition format
* eDDTs test cases are LTP test cases and vice-versa.

The main additions or 'enhacements' of eDDTs compared to LTP are

* Several device drivers test cases like RTC,DMA,AUDIO,GPIO,ETHERNET,FLASH memories,eMMC
* Support for different SoCs and chipsets
* All eDDTs test cases and test code reside in eDDT/testsuite/<platform-name-evm>/.

# eDDTs Benefits #

================================================================

* Easy to use(filter test cases not applicable for platform)
* Easy to support new platforms
* High Code Reuse across platforms

# Test Suites Details #

================================================================

The following device test cases for a specified part numbers are available . 
SoC represents the test for system on Chip interfaces like SPI,I2C,CPU cores etc.  

Driver Category | Device | Part Number | Test Case |
| ------------- | -------| ----------  | --------- |
| Memory        | EEPROM | AT243C2 | Verifies Data Integrity by writing and reading to the EEPROM | 
|               | NOR FLASH | W25Q32 | 1. Performs Erase,Write and Read |
|               |           |        | 2. Verifies Data Integrity |
| Timer         | RTC | DS2321 | 1.Read RTC Time |
|              	|     |	       | 2.Compare RTC time & system time |
|               |     |        | 3.Checks for Alarms |
|               | WDT | SoC | Performs Watchdog timeout configuration & watchdog Refresh |
| Network       | Ethernet | SoC | 1.Interface Link |
|          	|          |     | 2.IP Address Connectivity |
|          	|          |     | 3.Speed & duplex test |
|          	|          |     | 4.Auto negotiate test |
| Sensors       | Temperature Sensor | TMP101 | Read Temperature value |
| Input/Output  | GPIO | SoC | 1.Test GPIO Output |
|      		|      |     | 2.Test GPIO Input |
| USB		| USB Device | SoC | Checks USB Device Connectivity | 
| Data Transfer | DMA |SoC | Performs memory-to-memory transfer test using a DMA Channel |
| Serial        | I2C | SoC | Scans I2C Bus for connected slaves |
|               | SPI | SoC | Verifies SoC SPI interface by loopback transmit & recieve |
| Audio         | | PWM | 1.Plays Sine tone using also utilities
|       	| |     | 2.Plays Wave file using alsa utilities
| Cores         |CPU Cores | SoC | Determines the number of CPU cores present in the SoC |

# Platform Under Tests Supported #

=================================================================

eDDTs has been used on following platforms or evaluation Boards:

* Beagle Bone Black EVM (AM35XX)
* Raspberry Pi 2 EVM (BCM2835)

# Host Requirements #

=================================================================

* Linux Host (Any distributions) 
* GCC Toolchain for ARM based platforms
* Serial console terminal application
* GIT

# Target Device Requirements #

=================================================================

* Linux Kernel and Filesystem should be Functional as the test runs at user space
* Filesystem should have the following utilities installed to run the existing test cases
	- ALSA utilities
	- Support for SYSFS & PROCS
	- iPerf Utilities
	- SSMTP 
* Device Drivers that needs to be tested

# Build & Installation #

================================================================

Clone the project at
	https://github.com/arjunTest3330/eDDTs.git

Detailed Build & Installation instructions are in the README-eDDTs file
	
# Running Tests #

================================================================

Run eDDTs tests the same way as LTP tests. Use ltprun program and pass to it the test scenario 
file in the runtest directory (option -f) to run. For example in beagle bone black platform:

	./runltp -f beaglebone-evm 

These option select test cases based on the test case TAG specified in the test scenario file.
The runltp script have lot of options. Some useful ones for stress tests are:
* -t DURATION: Define duration of the test in s,m,h,d.
* -x INSTANCES: Run multiple test instances in parallel.
* -c <options>: Run test under additional background CPU load
* -D <options>: Run test under additional background load on Secondary storage
* -m <options>: Run test under additional background load on Main memory
* -i <options>: Run test under additional background load on IO Bus
* -n          : Run test with network traffic in background.

Please refer to README-eDDTs file for more details.

The testsuite assumes a serial console is available for executing the 
test cases

# LICENSE #

=================================================================

The test suite follows Linux Test Project Licensing (GPL2 or later).

Refer LICENSE Document for further details

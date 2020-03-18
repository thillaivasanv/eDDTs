# eDDTs Overview (Embedded Device Driver Test Suite) #

================================================================

eDDTs is a test framework to test and validate various device drivers 
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

The following device test cases are available 

* RTC (DS3231)
* Temperature Sensor (TMP101)
* EEPROM (AT243C2 )
* SPI NOR FLASH
* Direct Memory Access (DMA)
* eMMC
* Audio Playback & Speaker Test
* Ethernet - Speed , autonegotiate , full duplex , Connectivity
* I2C Probe
* SPIDEV
* GPIO - SYSFS
* Watchdog
* Basic USB Test
* Number of CPU core

Device | Test Case |
| --- | --- |
| `DS2321-RTC` | 1.Read RTC Time |     
|              | 2.Compare RTC time & system time |
|              | 3.Checks for Alarms |
| 'TMP101-TEMP SENSOR' | 1.Read Temperature |
| 'I2C PROBE' | 1.Scans I2C Bus for connected slaves |

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

Clone the project
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


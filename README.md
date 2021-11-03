# ESP32-Partition-Test

## Problem description:


The [UpdateFromFS](https://github.com/espressif/arduino-esp32/blob/master/libraries/Update/examples/SD_Update/SD_Update.ino)
example fails if the running binary was compiled in Arduino IDE and the target binary was compiled in platformio.

This also applies to Update.rollBack().

## Requirements

A SDCard module is required (was easier to code than adding networking).

Those files will be *automatically* created on the SDCard after respective flashing, they will be named after the platform they've been compiled on.

    /part_pio_test.bin      << compiled from platformio


    /part_arduino_test.bin  << compiled from arduino IDE


Additionally they will also be copied to OTA1 after being flashed.
The reason for that is to provide neutral grounds to test `Update.rollBack()` with any sketch outside this project.
Since the partition has been copied on OTA1, the next time the ESP is flashed will burn on OTA0, and OTA1 will be available for rollback.


## Usage

1) Open the sketch with Arduino IDE, open the serial console, compile and flash

After flashing, the sketch will copy itself on OTA1 and on the SD Card, then restart.

Expected output:

```log
16:09:33.498 ->
16:09:33.498 -> ************* Welcome to Arduino IDE sketch ****************
16:09:33.498 -> Partition  Type   Subtype    Address   PartSize   ImgSize   Digest
16:09:33.498 -> ---------+------+---------+----------+----------+---------+--------
16:09:33.498 -> nvs        0x01      0x02   0x009000      20480       n/a   n/a
16:09:33.498 -> otadata    0x01      0x00   0x00e000       8192       n/a   n/a
16:09:33.597 -> app0       0x00      0x10   0x010000    1310720    311120   35a874467293e0197ccc13251f0ce6454e810f80d97912a2d0db1b07f0f1a0d0
16:09:33.729 -> app1       0x00      0x11   0x150000    1310720    310928   bb11467ed926c25669d45dc8a3ecefb242aafee6311502d35714813fe0b73744
16:09:33.729 -> spiffs     0x01      0x82   0x290000    1507328       n/a   n/a
16:09:33.729 -> Sketch is currently running on partition OTA0
16:09:33.729 -> Comparing 'app1' and 'app0' partitions
16:09:33.862 -> Partitions differ, will initiate copy
16:09:33.895 -> [SD] Opening /part_arduino_test.bin
16:09:33.895 -> Copying partition 'app0' to 'app1' (flash) and /part_arduino_test.bin (SD)
16:09:39.270 -> Done
16:09:39.303 -> Now Using rollback to switch to 'app1' partition
16:09:39.502 -> ets Jun  8 2016 00:22:57
16:09:39.502 ->
16:09:39.502 -> rst:0xc (SW_CPU_RESET),boot:0x17 (SPI_FAST_FLASH_BOOT)
16:09:39.502 -> configsip: 0, SPIWP:0xee
16:09:39.502 -> clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
16:09:39.502 -> mode:DIO, clock div:1
16:09:39.502 -> load:0x3fff0030,len:1420
16:09:39.502 -> ho 0 tail 12 room 4
16:09:39.502 -> load:0x40078000,len:13540
16:09:39.502 -> load:0x40080400,len:3604
16:09:39.502 -> entry 0x400805f0
16:09:39.635 -> [␀␘␂␂␂��m␑um����2-hal-cpu.c:211] setCpuFrequencyMhz(): PLL: 480 / 2 = 240 Mhz, APB: 80000000 Hz
16:09:40.664 ->
16:09:40.664 -> ************* Welcome to Arduino IDE sketch ****************
16:09:40.664 -> Partition  Type   Subtype    Address   PartSize   ImgSize   Digest
16:09:40.664 -> ---------+------+---------+----------+----------+---------+--------
16:09:40.664 -> nvs        0x01      0x02   0x009000      20480       n/a   n/a
16:09:40.697 -> otadata    0x01      0x00   0x00e000       8192       n/a   n/a
16:09:40.796 -> app0       0x00      0x10   0x010000    1310720    311120   35a874467293e0197ccc13251f0ce6454e810f80d97912a2d0db1b07f0f1a0d0
16:09:40.929 -> app1       0x00      0x11   0x150000    1310720    311120   35a874467293e0197ccc13251f0ce6454e810f80d97912a2d0db1b07f0f1a0d0
16:09:40.929 -> spiffs     0x01      0x82   0x290000    1507328       n/a   n/a

```



2) install platformio, then `cd` to the project folder and issue `pio run -e esp32 --target=upload --target=monitor --upload-port=/dev/ttyUSB0` (edit your usb port if necessary)

After flashing, the sketch will copy itself on OTA1 and on the SD Card, then restart.

Expected output:

```log
************* Welcome to platformio sketch ****************
Partition  Type   Subtype    Address   PartSize   ImgSize   Digest
---------+------+---------+----------+----------+---------+--------
nvs        0x01      0x02   0x009000      20480       n/a   n/a
otadata    0x01      0x00   0x00e000       8192       n/a   n/a
app0       0x00      0x10   0x010000    1310720    310928   bb11467ed926c25669d45dc8a3ecefb242aafee6311502d35714813fe0b73744
app1       0x00      0x11   0x150000    1310720    311120   35a874467293e0197ccc13251f0ce6454e810f80d97912a2d0db1b07f0f1a0d0
spiffs     0x01      0x82   0x290000    1507328       n/a   n/a
Sketch is currently running on partition OTA0
Comparing 'app1' and 'app0' partitions
Partitions differ, will initiate copy
[SD] Opening /part_pio_test.bin
Copying partition 'app0' to 'app1' (flash) and /part_pio_test.bin (SD)
Done
Now Using rollback to switch to 'app1' partition
ets Jun  8 2016 00:22:57

rst:0xc (SW_CPU_RESET),boot:0x17 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:2
load:0x3fff0030,len:1284
load:0x40078000,len:12808
load:0x40080400,len:3032
entry 0x400805e4
[␀␘␂␂␂��m␑um����2-hal-cpu.c:211] setCpuFrequencyMhz(): PLL: 480 / 2 = 240 Mhz, APB: 80000000 Hz

************* Welcome to platformio sketch ****************
Partition  Type   Subtype    Address   PartSize   ImgSize   Digest
---------+------+---------+----------+----------+---------+--------
nvs        0x01      0x02   0x009000      20480       n/a   n/a
otadata    0x01      0x00   0x00e000       8192       n/a   n/a
app0       0x00      0x10   0x010000    1310720    310928   bb11467ed926c25669d45dc8a3ecefb242aafee6311502d35714813fe0b73744
app1       0x00      0x11   0x150000    1310720    310928   bb11467ed926c25669d45dc8a3ecefb242aafee6311502d35714813fe0b73744
spiffs     0x01      0x82   0x290000    1507328       n/a   n/a

```


3) in the serial console, issue the command `sdload-other`

Expected behaviour: the binary from the other platform (Arduino IDE) should be loaded.
Actual behaviour:

```log
Command: sdload-other
File /part_arduino_test.bin exists (311120 bytes)
[ 94550][D][Updater.cpp:133] begin(): OTA Partition: app0
Written : 311120 successfully
OTA done!
Update successfully completed. Rebooting.
ets Jun  8 2016 00:22:57

rst:0xc (SW_CPU_RESET),boot:0x17 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:2
load:0x3fff0030,len:1284
load:0x40078000,len:12808
load:0x40080400,len:3032
entry 0x400805e4
ets Jun  8 2016 00:22:57

rst:0x10 (RTCWDT_RTC_RESET),boot:0x17 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:2
load:0x3fff0030,len:1284
load:0x40078000,len:12808
load:0x40080400,len:3032
entry 0x400805e4
�ets Jun  8 2016 00:22:57

rst:0x10 (RTCWDT_RTC_RESET),boot:0x17 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:2
load:0x3fff0030,len:1284
load:0x40078000,len:12808
load:0x40080400,len:3032
entry 0x400805e4
␀ets Jun  8 2016 00:22:57

rst:0x10 (RTCWDT_RTC_RESET),boot:0x17 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:2
load:0x3fff0030,len:1284
load:0x40078000,len:12808
load:0x40080400,len:3032
entry 0x400805e4

```



## Serial Console Tools

  `propagate` : Copy current sketch on other OTA partition and onto the SD Card

  `rollback`: issue an `Update.rollBack()`

  `copytosd`: Copy self to SD Card

  `sdload-piosketch`: Explicit, flash the ESP with SD binary from platformio

  `sdload-arduinosketch`: Explicit, flash the ESP with SD binary from arduino

  `sdload-this`: Contextual, flash the ESP with SD binary from the **same** platform

  `sdload-other`: Contextual, flash the ESP with SD binary from the **other** platform



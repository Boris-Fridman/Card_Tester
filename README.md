# Card_Tester
### The final project from the course RT-Concepts

The project contains the 5 main folders:
1. UUT_Prog - The folder with the program that is burnt to the STM32F756ZG MCU for receiving data from computer and testing the devices in the board and sending back the test results.
2. Comp_Prog - The folder with the program sending commands for test to the unit under test and receiving the test results.
3. Common_Libs - The folder containint the libraries-files containing the common definitions and functions for the both progrms mentioned above.
4. BootLoader - The folder with the boot-loader project for burning new program-image "UUT_Prog" to the flash by receifing thata "OTA" via ethernet.
5. CTUpdater - The folder with computer program sending the image to the bootloader "OTA" via the ethernet. 

The program uses sql database "SQLite" so for using this program it is needed to do:
1. Install SQLite by the command sudo apt install libsqlite3-dev
2. The header must be included: #include <sqlite3.h>
3. The linking must be done (in case of tcc compiler) with the parameter: -lsqlite3 (e.g., gcc main.c -lsqlite3).


#### Peripheral Descriptions:
The peripherals for testing were selected according 2 conditions:
1. The all the IO-Pins of these peripherals must exist in the original-factory-soldered socket to make the jump wiring easier.
2. The all peripherals must be able to share their DMA cahnnels simultaniousely (because not all combinations permit this).

The selected devices are:

       ┏━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━┓
       ┃           ┃        Tester       ┃        Tested        ┃
       ┃           ┃                     ┃                      ┃
       ┣━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━┫
       ┃   Timer   ┃   Timer_1  Ch_1     ┃   Timer_2   Ch_2     ┃
       ┣━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━┫
       ┃   UART    ┃   UART_6            ┃   UART_4             ┃
       ┣━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━┫
       ┃   SPI     ┃   SPI_1    Master   ┃   SPI_4     Slave    ┃
       ┣━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━┫
       ┃   I2C     ┃   I2C_1    Master   ┃   I2C_2     Slave    ┃
       ┣━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━┫
       ┃   ADC     ┃   DAC               ┃   ADC_1     Ch_0     ┃
       ┗━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━┛




PinOuts given to each device and connection between the tester and the tested devices:

       ┏━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━┓
       ┃    SPI_1 Tester     ┃ Direction  ┃    SPI_4 Tested     ┃
       ┃    Master           ┃            ┃    Slave            ┃
       ┣━━━━━━━━━━┳━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━┳━━━━━━━━━━┫
       ┃   MOSI   ┃   PB5    ┃    ▬▬▬▶    ┃   PE6    ┃   MOSI   ┃
       ┣━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━┫
       ┃   MISO   ┃   PA6    ┃    ◀▬▬▬    ┃   PE5    ┃   MISO   ┃
       ┣━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━┫
       ┃   CLK    ┃   PA5    ┃    ▬▬▬▶    ┃   PE2    ┃   CLK    ┃
       ┣━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━┫
       ┃   NSS    ┃   PA15   ┃    ▬▬▬▶    ┃   PE4    ┃   NSS    ┃
       ┗━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━┛
                                           
                                                                                  
                                                                                  
                                                                                  
       ┏━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━┓
       ┃    I2C_1 Tester     ┃ Direction  ┃    I2C_2 Tested     ┃
       ┃    Master           ┃            ┃    Slave            ┃
       ┣━━━━━━━━━━┳━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━┳━━━━━━━━━━┫
       ┃   SCL    ┃   PB8    ┃    ▬▬▬▶    ┃   PB10   ┃   SCL    ┃
       ┣━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━┫
       ┃   SDA    ┃   PB9    ┃    ◀▬▬▶    ┃   PB11   ┃   SDA    ┃
       ┗━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━┛
                                           
                                                                                  
                                                                                  
                                                                                  
       ┏━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━┓
       ┃    UART_6 Tester    ┃ Direction  ┃    UART_4 Tested    ┃
       ┣━━━━━━━━━━┳━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━┳━━━━━━━━━━┫
       ┃   TX     ┃   PD8    ┃    ▬▬▬▶    ┃   PC11   ┃   RX     ┃
       ┣━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━┫
       ┃   RX     ┃   PD9    ┃    ◀▬▬▬    ┃   PC10   ┃   TX     ┃
       ┗━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━┛
                                                                                  
       ┏━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━┓
       ┃     DAC Tester      ┃ Direction  ┃  ADC_1 Ch_0 Tested  ┃
       ┣━━━━━━━━━━┳━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━┳━━━━━━━━━━┫
       ┃   Aout   ┃   PA4    ┃    ▬▬▬▶    ┃   PA0    ┃   Ain    ┃
       ┗━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━┛
                                  
                                                                                  
                                                                                  
       ┏━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━┓
       ┃ Timer_1 Ch_1 Tester ┃ Direction  ┃ Timer_2 Ch_2 Tested ┃
       ┣━━━━━━━━━━┳━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━┳━━━━━━━━━━┫
       ┃ PWM out  ┃   PE9    ┃    ▬▬▬▶    ┃   PB3    ┃  PWM in  ┃
       ┗━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━┛
   





#### Burning Method:
To burn the program its needed to start the card in bootloder mode. It can be possible by 3 ways:
1. If the program is boroken the bootloader will detect the problem according to CRC from program code existance in the last memory sector and start automatically if it isn't detected.
2. To power up or reset the board from reset button by holding the function button during powrup or restarting. The bootloader will detect the held button and will start itself insteas of testing program.
3. To run dirrectly by burning computer program when the board is on. The burning program will detect the testing program, will send the command to restart, will wait 10 seconds (the bootloader tetects the restart reason and in case of soft restart it will run itself) and after detection the bootloader, the computer program will start sending image-update.


The burning update program uses the same communication protocol as for test running including the responses.

The test request structure "TestData_s" containing the peripherals' bitfield "PeriphBitField_s" is used for sending the image. The Test_ID had to be used as the command type, but because the it also was needed to send the address the Test_ID is used for addresses while for the command type  is used the "Num_Interations". The main reason is that the command type has only 3 values and doesn't need the uint32_t type while the 32-bit-addressing needs it (the uint32_t type).
For the length of the segment is used the variable "Bit_Pattern_Length".
In any case the "OTA_UPDATE_bf"-flag will be set to "1" signalizing that the packet is used for updating.

typedef struct PeriphBitField_s
 {
  uint8_t Timer_bf       : 1;
  uint8_t UART_bf        : 1;
  uint8_t SPI_bf         : 1;
  uint8_t I2C_bf         : 1;
  uint8_t ADC_bf         : 1;
  uint8_t OTA_UPDATE_bf  : 1;
  uint8_t Reserved       : 2;
 }PeriphBitField_s;

 typedef struct TestData_s
 {
  uint32_t Test_ID;               /* In case of OTA Update is used as start address in flash of code segment. */
  uint32_t TestTime;
  int32_t TestVoltage;
  PeriphBitField_s Periph_B_F;
  uint8_t Num_Interations;        /* In case of OTA Update is used as Command Type.  */
  uint8_t Bit_Pattern_Length;     /* In case of OTA Update is used as code segment length. */
 }TestData_s;

For the segment itself there is used the BitPattern array whoues length doesn't exceed the 200 bytes.

For the responding answer there is used the "TestResult_s" structure where the Test_ID is used for sending the address as the "Message_ID" and the "TestResult" variable as determine if the burning passed or failed.
typedef struct TestResult_s
 {
  uint32_t Test_ID;              /* In case of OTA Update is used as start address in flash of code segment. */
  PeriphBitField_s Periph_B_F;
  PeriphBitField_s Results_B_F;
  TestResType_e TestResult;      /* In case of OTA Update is used for signalize if the burning was success of failed. */
 }TestResult_s;



 Sectors in flash definitions:
 See page 79 in "rm0385-stm32f75xxx-and-stm32f74xxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf" file.


                    Table 3. STM32F756xx and STM32F74xxx Flash memory organization
       ┏━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━┓
       ┃   Block     ┃      Name      ┃   Block base address on   ┃   Block base address on   ┃   Sector   ┃
       ┃             ┃                ┃      AXIM interface       ┃      ICTM interface       ┃    size    ┃
       ┣━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━┫
       ┃             ┃    Sector 0    ┃ 0x0800 0000 - 0x0800 7FFF ┃ 0x0020 0000 - 0x0020 7FFF ┃  32 Kbytes ┃
       ┃             ┣━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━┫
       ┃             ┃    Sector 1    ┃ 0x0800 8000 - 0x0800 FFFF ┃ 0x0020 8000 - 0x0020 FFFF ┃  32 Kbytes ┃
       ┃             ┣━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━┫
       ┃             ┃    Sector 2    ┃ 0x0801 0000 - 0x0801 7FFF ┃ 0x0021 0000 - 0x0021 7FFF ┃  32 Kbytes ┃
       ┃             ┣━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━┫
       ┃ Main Memory ┃    Sector 3    ┃ 0x0801 8000 - 0x0801 FFFF ┃ 0x0021 8000 - 0x0021 FFFF ┃  32 Kbytes ┃
       ┃    block    ┣━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━┫
       ┃             ┃    Sector 4    ┃ 0x0802 0000 - 0x0803 FFFF ┃ 0x0022 0000 - 0x0023 FFFF ┃ 128 Kbytes ┃
       ┃             ┣━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━┫
       ┃             ┃    Sector 5    ┃ 0x0804 0000 - 0x0807 FFFF ┃ 0x0024 0000 - 0x0027 FFFF ┃ 256 Kbytes ┃
       ┃             ┣━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━┫
       ┃             ┃    Sector 6    ┃ 0x0808 0000 - 0x080B FFFF ┃ 0x0028 0000 - 0x002B FFFF ┃ 256 Kbytes ┃
       ┃             ┣━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━┫
       ┃             ┃    Sector 7    ┃ 0x080C 0000 - 0x080F FFFF ┃ 0x002C 0000 - 0x002F FFFF ┃ 256 Kbytes ┃
       ┣━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━┫
       ┃             ┃ System memory  ┃ 0x1FF0 0000 - 0x1FF0 EDBF ┃ 0x0010 0000 - 0x0010 EDBF ┃  60 Kbytes ┃
       ┃ Information ┣━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━┫
       ┃    block    ┃      OTP       ┃ 0x1FF0 F000 - 0x1FF0 F41F ┃ 0x0010 F000 - 0x0010 F41F ┃ 1024 bytes ┃
       ┃             ┣━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━┫
       ┃             ┃   Option bytes ┃ 0x1FFF 0000 - 0x1FFF 001F ┃             -             ┃   32 bytes ┃
       ┗━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━┛










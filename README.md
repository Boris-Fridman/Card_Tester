# Card_Tester
The final project from the course RT-Concepts

The project contains the 3 main folders:
1. UUT_Prog - The folder with the program that is burnt to the STM32F756ZG MCU for receiving data from computer and testing the devices in the board and sending back the test results.
2. Comp_Prog - The folder with the program sending commands for test to the unit under test and receiving the test results.
3. Common_Libs - The folder containint the libraries-files containing the common definitions and functions for the both progrms mentioned above.

The program uses sql database "SQLite".
so for using this program it is needed to do:
1. Install SQLite by the command sudo apt install libsqlite3-dev
2. The header must be included: #include <sqlite3.h>
3. The linking must be done (in case of tcc compiler) with the parameter: -lsqlite3 (e.g., gcc main.c -lsqlite3).

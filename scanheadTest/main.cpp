//  File
//      main.cpp
//
//  Abstract
//      The simplest function using cpp method 
//      Please do not use object oriented programming 
//      evaluate case by case or test by test
//      important to load laser inside the main function
//
//  Author
//      Bernhard Schrems, SCANLAB AG
//      adapted for RTC5: Hermann Waibel, SCANLAB AG
//
//  Features
//      - explicit linking to the RTC5DLL.DLL
//      - use of the list buffer as a single list like a circular queue 
//        for continuous data transfer
//      - exception handling
//
//  Comment
//      This application demonstrates how to explicitly link to the
//      RTC5DLL.DLL. For accomplishing explicit linking - also known as
//      dynamic load or run-time dynamic linking of a DLL, the header
//      file RTC5expl.H is included.
//      Before calling any RTC5 function, initialize the DLL by calling
//      the function RTC5open.
//      When the usage of the RTC5 is finished, close the DLL by 
//      calling the function RTC5close.
//      For building the executable, link with the RTC5EXPL.OBJ, which
//      you can generate from the source code RTC5expl.c.
//
//      This routine also shows how to use the list buffer as a circular
//      queue. Methods to halt and to resume the data transfer are also shown.
//
//  Necessary Sources
//      RTC5expl.h, RTC5expl.c
//
// System header files
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <math.h>

// RTC5 header file for explicitly linking to the RTC5DLL.DLL
#include "RTC5expl.h"

// Definition of "pi"
const double Pi = 3.14159265358979323846;

// Change these values according to your system
const UINT   DefaultCard = 1;   //  number of default card
const UINT   ListMemory = 10000;   //  size of list 1 memory (default 4000)
								   // could be apply for config_list() to allocate memory
const UINT   LaserMode = 1;   //  YAG 1 mode
const UINT   LaserControl = 0x18;   //  Laser signals LOW active (Bits #3 and #4)
const UINT   StartGap = 1000;   //  gap ahead between input_pointer and out_pointer
const UINT   LoadGap = 100;   //  gap ahead between out_pointer and input_pointer
const UINT   PointerCount = 0x3F;   //  pointer mask for checking the gap

// RTC4 compatibility mode assumed
const UINT   AnalogOutChannel = 1;   //  AnalogOut Channel 1 used
const UINT   AnalogOutValue = 640;   //  Standard Pump Source Value
const UINT   AnalogOutStandby = 0;   //  Standby Pump Source Value
const UINT   WarmUpTime = 2000000 / 10;   //    2  s [10 us]
const UINT   LaserHalfPeriod = 50 * 8;   //   50 us [1/8 us] must be at least 13
const UINT   LaserPulseWidth = 5 * 8;   //    5 us [1/8 us]
const UINT   FirstPulseKiller = 200 * 8;   //  200 us [1/8 us]
const long   LaserOnDelay = 100 * 1;   //  100 us [1 us]
const UINT   LaserOffDelay = 100 * 1;   //  100 us [1 us]

const UINT   JumpDelay = 250 / 10;   //  250 us [10 us]
const UINT   MarkDelay = 100 / 10;   //  100 us [10 us]
const UINT   PolygonDelay = 50 / 10;   //   50 us [10 us]
const double MarkSpeed = 250.0;   //  [16 Bits/ms]
const double JumpSpeed = 1000.0;   //  [16 Bits/ms]

// Spiral Parameters
const double Amplitude = 10000.0;
const double Period = 512.0;   // amount of vectors per turn
const double Omega = 2.0 * Pi / Period;

// End Locus of a Line
struct locus { long xval, yval; };

const locus BeamDump = { -32000, -32000 }; //  Beam Dump Location

int PlotLine(const locus& destination, UINT* start);
void PlotFlush();
void terminateDLL();            //  waits for a keyboard hit to terminate

#include <iostream>
using namespace std;


int __cdecl main(void*, void*){

	// open dll and
	// RTC5 Board initialization
	if (RTC5open()){
		std::cout<<"Error: RTC5DLL.DLL not found\n";
		terminateDLL();
		return 1; 
	}
	std::cout<<"Initializing the DLL\n\n";
	UINT ErrorCode;			
	ErrorCode = init_rtc5_dll();

	// Handle error code
	if (ErrorCode) {
		const UINT RTC5CountCards = rtc5_count_cards();   //  number of cards found
		
		if (RTC5CountCards) {
			UINT AccError(0);
			
			for (UINT i = 1; i <= RTC5CountCards; i++){
				const UINT Error = n_get_last_error(i);
				
				if (Error != 0){
					AccError |= Error;
					printf("Card no. %d: Error %d detected\n", i, Error);
					n_reset_error(i, Error);
				}
			}

			if (AccError){
				terminateDLL();
				return;
			}
		}
		else{
			printf("Initializing the DLL: Error %d detected\n", ErrorCode);
			terminateDLL();
			return;
		}
	}
	else {

		if (DefaultCard != select_rtc(DefaultCard)){     //  use card no. 1 as default, 
			ErrorCode = n_get_last_error(DefaultCard);

			if (ErrorCode & 256){
				//  In this case load_program_file(0) would not work.
				ErrorCode = n_load_program_file(DefaultCard, 0); //  current working path
			}
			else{
				printf("No acces to card no. %d\n", DefaultCard);
				terminateDLL();
				return;
			}

			if (ErrorCode){
				printf("No access to card no. %d\n", DefaultCard);
				terminateDLL();
				return;
			}
			else{   //  n_load_program_file was successfull
				(void)select_rtc(DefaultCard);
			}
		}
	}

	set_rtc4_mode();
	stop_execution();

	// loading file RTC5OUT.out RTC5RBF.rbf RTC5DAT.da
	// and take the error from loading
	ErrorCode = load_program_file(0);
	if (ErrorCode){
		printf("Program file loading error: %d\n", ErrorCode);
		terminateDLL();
		return 1;
	}

	// file "D2_1to1.ct5", table by default, 2 dimension
	// take error when loading correction file
	ErrorCode = load_correction_file("D2_1to1.ct5", 1, 2); // use 2D only
	if (ErrorCode){
		printf("Correction file loading error: %d\n", ErrorCode);
		terminateDLL();
		return 1;
	}
	select_cor_table(1, 0);   //  table #1 at primary connector (default)
	reset_error(-1); //  stop_execution might have created a RTC5_TIMEOUT error

	std::cout << "Initialization RTC5board and file loading Finished!! Set up Lasers";

	config_list(10000, 0); //  Configure list memory, default: config_list( 4000, 4000 ).
	set_laser_mode();  // 0 CO2 mode YAG 
 
	return 0;
}


//  terminateDLL
//
//  Description
//
//  The function waits for a keyboard hit
//  and then calls free_rtc5_dll().
//  

void terminateDLL()
{
	printf("- Press any key to shut down \n");

	while (!_kbhit());

	(void)_getch();
	printf("\n");

	free_rtc5_dll();
	RTC5close();

}



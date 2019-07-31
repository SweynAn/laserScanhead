//  File
//      main.cpp
//
//  Abstract 
//		First testing program help test the mark jump
//      home postion, jumpspeed, markSpeed, and etc.
//
//  Comment
//      The simplest function using cpp method 
//      Please do not use object oriented programming 
//      evaluate case by case or test by test
//      important to load laser inside the main function
//
//  Author
//      Shiwen An, Popmintchev Labs July, 2019
//
//  Features
//      - explicit linking to the RTC5DLL.DLL
//      - use of the list buffer as a single list like a circular queue 
//        for continuous data transfer
//      - exception handling
//		- Test driven development testing various case and basic function
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
				return 1;
			}
		}
		else{
			printf("Initializing the DLL: Error %d detected\n", ErrorCode);
			terminateDLL();
			return 1;
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
	set_laser_mode(0);  // 0 CO2 mode 1 YAG , Not useful for our experiment
	set_firstpulse_killer( FirstPulseKiller ); // used to kill the first pulse
	set_laser_control( LaserControl ); //setup laser control

	// Test 0 check which function works best for the home position
	// or stand by place?
	// set_standby
	home_position(1,1); // home_position(X,Y) setup for jump near origin (0,0) to disable

    // Timing, delay and speed preset
    // Laser on and laser off
    // some parameter need to measure
    // LaserHalfPeriod ==> (T/2) || (Pi/omega) based on physics of laser
    set_start_list( 1 );
        set_laser_pulses( LaserHalfPeriod, LaserPulseWidth );
        set_scanner_delays( JumpDelay, MarkDelay, PolygonDelay );
        set_laser_delays( LaserOnDelay, LaserOffDelay );
        set_jump_speed( JumpSpeed );
        set_mark_speed( MarkSpeed );
    set_end_of_list();

    execute_list( 1 ); 

    // test: does execute_list pop up the command in the list?




    // Add a switch method for debugging the program
    // Switch 1,2,3,4,5 ... and etc.
    // 1 Laser On/Off
    // 2 Jump :
    //		i) physically ensure the location 
    //         in unit of mm , might need conversion table
    //         from mm to bit as the result
    // 3 Jump and Mark point
    // 4 Mark line using position
    // 5 Test controlling of speed for Marking
    // 6 draw a shape with certain algorithm

    // Test 1 Control laser ON/OFF
    // do this part in the lab
    std::cout << "Test1 On/Off Laser Control\n";
    do{
    	string s;
    	while(! _getch()) disable_laser();
    	while(! _getch()) enable_laser();
    	std::cout << "Continue to check On/Off? [Y/N] \n"
    	std::cin >> s;
    	if( s == "n" || s == "N") break;
	}while(true);


	// Test 2 Control the jump
	double jumpX, jumpY;
	std::cout << "Which location you want to get the point?(x,y)[mm]\n"
	std::cin >> jumpX;
	std::cin >> jumpY;
	std::cout << "hit the keyboard to Stop jumping\n"
    do{
    	load_list(1,0); // sta
    		jump_abs(jumpX,jumpY);
    	set_end_of_list();
    	execute_list(1);
    	if( _getch()) break;
	}while(true);

	// test another function reset to (0,0)
	goto_xy(0,0);

	// Test 3 Control the mark
	double markX, markY;
	std::cout << "Which location you want to get the point?(x,y)[mm]\n"
	std::cin >> markX;
	std::cin >> markY;
	std::cout << "hit the keyboard to Stop marking\n"
    do{
    	load_list(1,0); // sta
    		jump_abs(markX,markY);
    		mark_abs(markX,markY);
    	set_end_of_list();
    	execute_list(1);
    	if( _getch()) break;
	}while(true);

	// Test 4 mark a line, which should be 
	// simple using vector , I used complicated method when
	// first trying to address such problem 

	double startX, startY, endX, endY;
	std::cout << "Which start location you want to get the point?(x,y)[mm]\n"
	std::cin >> startX;
	std::cin >> startY;
	std::cout << "Which start location you want to get the point?(x,y)[mm]\n"
	std::cin >> endX;
	std::cin >> endY;
	std::cout << "hit the keyboard to Stop marking\n"

	// Jump before the execution
	load_list(1,0);
		jump_abs(startX,startY);
	set_end_of_list();
	execute_list(1);

	// do the marking based on vector map instead of using loop
    do{
    	load_list(1,0); // sta
    		mark_abs(endX,endY);
    		mark_abs(startX,startY);
    	set_end_of_list();
    	execute_list(1);
    	if( _getch()) break;
	}while(true);


	// Test 5 test the mark speed influence

    // Lens used for "D2_1to1.ct5" gives 
    // K = 13376 bit/mm
    // V_mark =  speed/K ==> set_mark_speed(V_mark*K)
    // debug test
    
    double mSpeed;
    do{
		std::cout<< "mark speed (m/s) [0 quit test] : ";
		std::cin >> mSpeed;
		if(mSpeed == 0) break;

		// Hard coding for speed test purpose
		load_list(1,0);
    		set_mark_speed(mSpeed*K);
    		jump_abs(-100,-100); 
    	set_end_of_list();
    	execute_list(1);
    	
    	do{
    		load_list(1,0); // sta
    			mark_abs(100,100);
    			mark_abs(-100,-100);
    		set_end_of_list();
    		execute_list(1);
    		if( _getch()) break;
		}while(true);
    }while(true);

	// End of test
	std::cout << "Test Finished! \n";
	terminateDLL()
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



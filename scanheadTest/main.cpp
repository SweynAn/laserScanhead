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
const LONG   R = 20000;                  //  size parameter of the figures
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

struct polygon
{
	LONG xval, yval;
};

const polygon square[] =
{
	  {-R, -R}
	, {-R,  R}
	, {R,  R}
	, {R, -R}
	, {-R, -R}
};

const polygon triangle[] =
{
	  {-R,  0L}
	, {R,  0L}
	, {0L,  R}
	, {-R,  0L}
};


const locus BeamDump = { -32000, -32000 }; //  Beam Dump Location

int PlotLine(const locus& destination, UINT* start);
void draw(const polygon* figure, const size_t& size);
void test1();
void test1_1();
void test1_2();
void terminateDLL();            //  waits for a keyboard hit to terminate

#include <iostream>
using namespace std;


int __cdecl main(void*, void*){

	if (RTC5open())
	{
		printf("Error: RTC5DLL.DLL not found\n");
		terminateDLL();
		return 1;

	}

	printf("Initializing the DLL\n\n");

	UINT  ErrorCode;

	//  This function must be called at the very first
	ErrorCode = init_rtc5_dll();

	if (ErrorCode)
	{
		const UINT RTC5CountCards = rtc5_count_cards();   //  number of cards found

		if (RTC5CountCards)
		{
			UINT AccError(0);

			//  Error analysis in detail
			for (UINT i = 1; i <= RTC5CountCards; i++)
			{
				const UINT Error = n_get_last_error(i);

				if (Error != 0)
				{
					AccError |= Error;
					printf("Card no. %d: Error %d detected\n", i, Error);
					n_reset_error(i, Error);

				}

			}

			if (AccError)
			{
				terminateDLL();
				return 1;

			}

		}
		else
		{
			printf("Initializing the DLL: Error %d detected\n", ErrorCode);
			terminateDLL();
			return 1;

		}

	}
	else
	{
		if (DefaultCard != select_rtc(DefaultCard))     //  use card no. 1 as default, 
		{
			ErrorCode = n_get_last_error(DefaultCard);

			if (ErrorCode & 256)    //  RTC5_VERSION_MISMATCH
			{
				//  In this case load_program_file(0) would not work.
				ErrorCode = n_load_program_file(DefaultCard, 0); //  current working path

			}
			else
			{
				printf("No acces to card no. %d\n", DefaultCard);
				terminateDLL();
				return 1;

			}

			if (ErrorCode)
			{
				printf("No access to card no. %d\n", DefaultCard);
				terminateDLL();
				return 1;

			}
			else
			{   //  n_load_program_file was successfull
				(void)select_rtc(DefaultCard);

			}

		}

	}

	set_rtc4_mode();            //  for RTC4 compatibility

	// Initialize the RTC5
	stop_execution();
	//  If the DefaultCard has been used previously by another application 
	//  a list might still be running. This would prevent load_program_file
	//  and load_correction_file from being executed.

	ErrorCode = load_program_file(0);     //  path = current working path

	if (ErrorCode)
	{
		printf("Program file loading error: %d\n", ErrorCode);
		terminateDLL();
		return 1;

	}

	ErrorCode = load_correction_file( "D2_1to1.ct5",   // initialize like "D2_1to1.ct5",
		1,   // table; #1 is used by default
		2); // use 2D only
	if (ErrorCode)
	{
		printf("Correction file loading error: %d\n", ErrorCode);
		terminateDLL();
		return 1;

	}

	select_cor_table(1, 0);   //  table #1 at primary connector (default)

	//  stop_execution might have created a RTC5_TIMEOUT error
	reset_error(-1);    //  clear all previous error codes

	//  Configure list memory, default: config_list( 4000, 4000 ).
	//  One list only we probably only need the default one
	//config_list(ListMemory, 0);
	
	//  input_list_pointer and out_list_pointer will jump automatically 
	//  from the end of the list onto position 0 each without using
	//  set_end_of_list. auto_change won't be executed.
	//  RTC4::set_list_mode( 1 ) is no more supported

	set_laser_mode(LaserMode);
	set_firstpulse_killer(FirstPulseKiller);

	//  This function must be called at least once to activate laser 
	//  signals. Later on enable/disable_laser would be sufficient.
	set_laser_control(LaserControl);

	// Turn on the optical pump source
	write_da_x(AnalogOutChannel, AnalogOutValue);


	// Test 0 check which function works best for the home position
	// Success!! home_position(0,0) disable the home_position 
	home_position(1, 1); // home_position(X,Y) setup for jump near origin (0,0) to disable

	// Timing, delay and speed preset
	// Laser on and laser off
	// some parameter need to measure
	// LaserHalfPeriod ==> (T/2) || (Pi/omega) based on physics of laser
	set_start_list(1);
	long_delay(WarmUpTime);
	set_laser_pulses(LaserHalfPeriod, LaserPulseWidth);
	set_scanner_delays(JumpDelay, MarkDelay, PolygonDelay);
	set_laser_delays(LaserOnDelay, LaserOffDelay);
	set_jump_speed(JumpSpeed);
	set_mark_speed(MarkSpeed);
	set_end_of_list();

	execute_list(1);

	// Draw to warm up, given function 
	// Please do not use _getch() directly to catch the keyboard input
	// use kbhit+getch together for stop a loop

	do {
		draw(square, sizeof(square) / sizeof(square[0]));
		draw(triangle, sizeof(triangle) / sizeof(triangle[0]));
		if(_kbhit()) if (_getch()) break;
	} while (true);
	
	std::cout << "Test1_1 mark point/line, hit keyboard break";

	do {
		test1_1();
		if (_kbhit()) if (_getch()) break;
	} while (true);

	test1_2();

	test1();

	// restart_list();
	// executed if a list was 

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
	// 6 Random Jump of Laser
    // 7 draw a shape with certain algorithm

    // Test 1 Control laser ON/OFF
    // do this part in the lab
	
	/*
    std::cout << "Test1 On/Off Laser Control\n";
    do{
    	string s;
    	while(! _getch()) disable_laser();
    	while(! _getch()) enable_laser();
		std::cout << "Continue to check On/Off? [Y/N] \n";
    	std::cin >> s;
    	if( s == "n" || s == "N") break;
	}while(true);
	*/

	// restart_list();



	// Test 2 Control the jump
	long jumpX, jumpY;
	std::cout << "Which location you want to get the point?(x,y)[mm]\n";
	std::cin >> jumpX;
	std::cin >> jumpY;
	std::cout << "hit the keyboard to Stop jumping\n";
	
	load_list(1, 0); // sta
	do{
    	jump_abs(jumpX,jumpY);
    	if( _getch()) break;
	}while(true);
	set_end_of_list();
	execute_list(1);

	// test another function reset to (0,0)
	goto_xy(0,0);

	// Test 3 Control the mark
	long markX, markY;
	std::cout << "Which location you want to get the point?(x,y)[mm]\n";
	std::cin >> markX;
	std::cin >> markY;
	std::cout << "hit the keyboard to Stop marking\n";
    do{
    	load_list(1,0); // sta
    		jump_abs(markX,markY);
    		mark_abs(markX,markY);
    	set_end_of_list();
    	execute_list(1);
    	if( _getch()) break;
	}while(true);


	// Test 5 test the mark speed influence

    // Lens used for "D2_1to1.ct5" gives 
    // K = 13376 bit/mm
    // V_mark =  speed/K ==> set_mark_speed(V_mark*K)
    // debug test
    
    long mSpeed, K;
	load_list(1, 0);
		K = get_head_para(1, 1);
	set_end_of_list();
	execute_list(1);

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
	terminateDLL();
	return 0;
}


void test1() {

	// Test 4 mark a line, which should be 
    // simple using vector , I used complicated method when
    // first trying to address such problem 
	LONG startX, startY, endX, endY;
	std::cout << "start point?(x,y)[mm]\n";
	std::cin >> startX;
	std::cin >> startY;
	std::cout << "end point?(x,y)[mm]\n";
	std::cin >> endX;
	std::cin >> endY;
	std::cout << "hit the keyboard to Stop marking\n";

	// Jump before the execution
	while (!load_list(1U, 0U)) {};
	jump_abs(startX, startY);

	// do the marking based on vector map instead of using loop
	//do {
	
	mark_abs(endX, endY);
	mark_abs(startX, startY);

		//if (_getch()) break;
	//} while (true);

	set_end_of_list();
	execute_list(1U);

}

void test1_1() {
	LONG a, b;
	a = 12345;
	b = 12345;

	std::cout << "Test1_1 with a:" << a << "and b: " << b << "\n";

	while (!load_list(1U, 0U)) {}
	mark_abs(a, b);
	set_end_of_list();
	execute_list(1U);
}

//
// advanced version of test1_1
//
void test1_2() {
	LONG x, y, a, b;
	x = 20000;
	y = 20000;
	a = -20000;
	b = -20000;
	std::cout << "Test1_1 with a:" << a << "and b: " << b << "\n";
	std::cout << "Test1_2 with x:" << x << "and y: " << y << "\n";


	while (!load_list(1U, 0U)) {}
	do {
		mark_abs(a, b);
		mark_abs(x, y);
		if (_kbhit()) if (_getch()) break;
	} while (true);
	set_end_of_list();
	execute_list(1U);
}

void draw(const polygon* figure, const size_t& size)
{
	// Only, use list 1, which can hold up to the configured entries
	while (!load_list(1U, 0U)) {} //  wait for list 1 to be not busy
	//  load_list( 1, 0 ) returns 1 if successful, otherwise 0
	//  set_start_list_pos( 1, 0 ) has been executed

	jump_abs(figure->xval, figure->yval);

	size_t i(0U);
	for (figure++; i < size - 1U; i++, figure++)
	{
		mark_abs(figure->xval, figure->yval);
	}
	set_end_of_list();
	execute_list(1U);
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



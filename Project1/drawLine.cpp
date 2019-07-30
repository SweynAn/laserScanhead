//  File
//      drawLine.cpp
//
//  Abstract
//      A console application for continuously plotting a single line
//      by using a Ultrafast laser
//
//  Author
//      Shiwen An, Popmintchev's Lab, University of California, San Diego.
//      adapted for RTC5: Hermann Waibel, SCANLAB AG
//
//  Features
//      - explicit linking to the RTC5DLL.DLL
//      - use list of firgure to draw a line
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
//      RTC5expl.h, RTC5expl.c drawLine.h RTC5DLLx64.dll
//
//  Environment: Win32/ I use windows 10 for development
//
//  Compiler
//      - tested with Visual C++ 7.1

// System header files
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <math.h>

// RTC5 header file for explicitly linking to the RTC5DLL.DLL
#include "RTC5expl.h"
#include "drawLine.h"
#include "drawRectangle.h"
#include <iostream>
using namespace std;



int __cdecl main(void*, void*)
{
	if ( initialRTC() ) 
	{
		std::cout<<"Error has occured, Please check based on Error code and RTC5 Board manual \n" ;
		return 1;
	}

	std::cout<<"Drawing various lines\n" ;
	std::cout<<"Configuring Laser" ;

	initialLaser();

	UINT Busy, Pos;

	do
	{
		get_status(&Busy, &Pos);

	} while (Busy);

	//  Wait until the execution is finished, before launching PlotLine.

	//  Pos denotes the position of the set_end_of_list command.
	//  The position of the input pointer is Pos + 1.
	//  set_start_list_pos( 1, Pos + 1 );   //  optional

	//  Do not send set_end_of_list(), unless you want to finish.
	//  The list buffer then behaves like a circular queue.
	//  Initialize the execution of the list explicitly, as soon as
	//  the input_pointer is StartGap ahead (see PlotLine) via
	//      execute_list_pos( 1, Pos + 1 )
	//  Make sure, that the input_pointer is always ahead of the 
	//  out_pointer and that they don't overtake each other.
	//  If they come too close (LoadGap), the input_pointer will wait or
	//  the out_pointer will be suspended via set_wait (see PlotLine).
	//
	//  CAUTION: WINDOWS might suspend the working thread at any time.
	//  Make sure, that StartGap is large enough so that the out_pointer
	//  can't overtake the input_pointer during that time.
	//  It is recommended to use two alternating lists with set_end_of_list 
	//  and auto_change instead of the continuous download demonstrated.
	//
	//  NOTE: Downloading list commands is buffered with buffer size 16.

	UINT    CarryOn, stopped, eventOff;
	int     i, plotStuff; // Counter and x, y position
	long    markSpeedReset;
	locus   pointStart, pointEnd; // location of start point
	string  plotMore;



	do
	{
		std::cout<< "What do you want to plot today? 0: Straigtline 1: Square or Rectangle\n" ;
		std::cin >> plotStuff;

		std::cout << "What is the Mark Speed? (m/s) \n" ;
		std::cin >> markSpeedReset;

		setMarkSpeed(markSpeedReset);

		if (plotStuff)
		{
			//Plot
			long l, w;

			std::cout<< "Enter the length of the square to choose the line you want to plot. \n" ;
			std::cout<<"Press S to suspend or R to resume plotting. \n";
			std::cout<<"Hit any other key will terminate the drawing system thoroughly.\n";

			std::cout << "Length of the shape: \n";
			std::cin >> l;
			std::cout << "Width of the shape: \n";
			std::cin >> w;

			for (eventOff = stopped = i = 0, CarryOn = 1; CarryOn; i++)
			{
				CarryOn = drawRectangle(l, w);
				if (!CarryOn) break;
			}
		}
		else {
			//Plot
			std::cout<<"Enter the start position and end position to choose the line you want to plot. \n";
			std::cout<<"Press S to suspend or R to resume plotting. \n";
			std::cout<<"Hit any other key will terminate the drawing system thoroughly.\n";

			for (eventOff = stopped = i = 0, CarryOn = 1; CarryOn; i++)
			{
				std::cout<<"What is the position to start (us) \n";
				std::cout<<"x axis: \n";
				std::cin >> pointStart.xval;

				std::cout<<"y axis: \n";
				std::cin >> pointStart.yval;

				std::cout<<"What is the position to end (us) \n";
				std::cout<<"x axis: \n";
				std::cin >> pointEnd.xval;
				std::cout<<"y axis: \n";
				std::cin >> pointEnd.yval;

				// false represent line
				// true represent there should be a rectangle
				CarryOn = drawStraightLine(pointStart, pointEnd, false);
				if (!CarryOn) break;
			}
		}

		std::cout << "Do you want to plot other stuff ? [Y/N] \n";
		std::cin >> plotMore;
		if ( plotMore == "N" || plotMore == "n") break;
		
	} while ( true );



	// Flush the circular queue, on request.
	/*
	if (!eventOff)
	{
		set_end_of_list();

		do
		{
			get_status(&Busy, &Pos);

		} while (Busy);
		//  Busy & 0x0001: list is still running
		//  Busy & 0x00fe: list has finished, but home_jump is still active
		//  Not possible after stop_execution:
		//  Busy & 0xff00: list paused (pause_list or set_wait)

	}
	not sure about whether I need this part or not
	*/

	// Finish
	std::cout<<"\nFinished - press any key to terminate";

	// Activate the pump source standby
	write_da_x(AnalogOutChannel, AnalogOutStandby);

	while (!_kbhit());

	(void)_getch();

	std::cout<<"\n";

	// Close the RTC5.DLL
	free_rtc5_dll();        //  optional
	RTC5close();

	return 0;

}


//
// initialRTC
//
// Description:
//
// Function name initialization:
// build up function for initialize
// catching errors and etc
//
int initialRTC( void ) 
{
	if (RTC5open())
	{
		std::cout<<"Error: RTC5DLL.DLL not found\n";
		terminateDLL();
		return 1;
	}

	std::cout<<"Initializing the DLL\n\n";

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

	ErrorCode = load_correction_file(0,   // initialize like "D2_1to1.ct5",
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

	return 0;  // nothing happened and initialization process complete
}

//
// initialLaser
//
// Description:
//
// Setup laser system, might be signal transfer 
// from one part to another 
// also setup the initial jumpspeed and markspeed
//
//
void initialLaser(void)
{

	//  Configure list memory, default: config_list( 4000, 4000 ).
	//  One list only
	config_list(ListMemory, 0);
	//  input_list_pointer and out_list_pointer will jump automatically 
	//  from the end of the list onto position 0 each without using
	//  set_end_of_list. auto_change won't be executed.
	//  RTC4::set_list_mode( 1 ) is no more supported

	set_laser_mode(LaserMode);
	set_firstpulse_killer(FirstPulseKiller);


	//  This function must be called at least once to activate laser 
	//  signals. Later on enable/disable_laser would be sufficient.

	set_laser_control(LaserControl);

	// Activate a home jump and specify the beam dump 
	home_position(BeamDump.xval, BeamDump.yval);

	// Turn on the optical pump source
	write_da_x(AnalogOutChannel, AnalogOutValue);

	// Timing, delay and speed preset
	set_start_list(1);
	long_delay(WarmUpTime);
	set_laser_pulses(LaserHalfPeriod, LaserPulseWidth);
	set_scanner_delays(JumpDelay, MarkDelay, PolygonDelay);
	set_laser_delays(LaserOnDelay, LaserOffDelay);
	set_jump_speed(JumpSpeed);
	set_mark_speed(MarkSpeed);
	set_end_of_list();

	execute_list(1);

	std::cout<<"Pump source warming up - please wait\r";
}


// 
// drawStraightLine
//
// Description:
// Simple function that defines on and off for straight line
// easier version of Plotline that I could understand and develop
// 
// Function called from libraries:
//      
// false represent line 
// true represent a shape
//
int drawStraightLine(const locus& startPosition, const locus & endPosition, bool shapeOrLine)
{
		
	long a, b, markX, markY, stepMicro ;
	// Returns the present position of the input pointer
	// get_status and get_out_pointer returns the output pointer
	// InPos = get_input_pointer();  
	
	// first get the line function based on the input
	// simple line equation initialization
	// y = a*x+b
	a = (endPosition.yval - startPosition.yval) / 
		(endPosition.xval - startPosition.xval);
	b = startPosition.yval - a * startPosition.xval;

	markX = startPosition.xval;
	markY = startPosition.yval;
	stepMicro = (endPosition.xval-startPosition.xval)/ 1000; // hard coding fix later

	while (!shapeOrLine) {
	
		// detect and catch the keyboard hit
		if (_kbhit()) {
			const char ch((char)_getch());

			switch (ch)
			{
			case 's':
			case 'S':
				// suddenly suspending the requested
				// pause the comment sent to Scanhead
				pause_list();
				// as long as the pause_list is active
				// the plotting is in suspended status
				std::cout<<"\r - plotting suspended - \n";
			case 'r':
			case 'R':
				// enable and restart the list
				// do nothing if not suspended previously
				enable_laser();
				restart_list();
				std::cout<<"\r - resume plotting - \n";
			default:
				// terminate the Laser
				// remove a pending "pause_list" call before calling
				// "Stop_execution"
				disable_laser();
				restart_list(); // optional
				stop_execution();
				std::cout<<"\r - plotting ends -- \n";
				return 0;
			}
		}
		else {

			while (!load_list(1U, 0U)) {} // wait for list 1 to be not busy


			jump_abs(markX, markY);
			
			while(  markX != endPosition.xval ) 
			{
				mark_abs(markX, markY);
				markX += stepMicro;
				markY = a * markX + b;
			}

			while (markX != startPosition.xval) 
			{
				markX -= stepMicro;
				markY = a * markX + b;
				mark_abs(markX, markY);
			}

			set_end_of_list();
			execute_list( 1U );

			markX = startPosition.xval;
			markY = startPosition.yval;
		}
	}

	if (shapeOrLine)
	{
		// detect and catch the keyboard hit
		if (_kbhit()) {
			const char ch((char)_getch());

			switch (ch)
			{
			case 's':
			case 'S':
				// suddenly suspending the requested
				// pause the comment sent to Scanhead
				pause_list();
				// as long as the pause_list is active
				// the plotting is in suspended status
				std::cout<<"\r - plotting suspended - \n";
			case 'r':
			case 'R':
				// enable and restart the list
				// do nothing if not suspended previously
				enable_laser();
				restart_list();
				std::cout<<"\r - resume plotting - \n";
			default:
				// terminate the Laser
				// remove a pending "pause_list" call before calling
				// "Stop_execution"
				disable_laser();
				restart_list(); // optional
				stop_execution();
				std::cout<<"\r - plotting ends -- \n";
				return 0;
			}
		}
		else {

			while (!load_list(1U, 0U)) {} // wait for list 1 to be not busy


			jump_abs(markX, markY);

			while (markX != endPosition.xval)
			{
				mark_abs(markX, markY);
				markX += stepMicro;
				markY = a * markX + b;
			}

			while (markX != startPosition.xval)
			{
				markX -= stepMicro;
				markY = a * markX + b;
				mark_abs(markX, markY);
			}

			set_end_of_list();
			execute_list(1U);

			markX = startPosition.xval;
			markY = startPosition.yval;
		}
	}


	return 0;
}

//
// drawRectangle
//
// Description:
// Simple function that draw the square based on drawStarightLine Function
// 
int drawRectangle(long length, long width) {
	locus startPos, endPos;
	long stepY, stepX;
	int returnVal;

	startPos.xval = -length / 2.0;
	startPos.yval = width / 2.0;
	endPos.xval = length / 2.0;
	endPos.yval = width / 2.0;

	stepX = length / 10000.0;
	stepY = width / 10000.0;

	while( startPos.yval != (-width/2.0) )
	{
		returnVal = drawStraightLine(startPos,endPos,true);
		startPos.yval -= stepY;
		endPos.yval -= stepY;
	}

	return returnVal;
	
}

void setMarkSpeed(long s) {
	// load correction file for lens 117486
    // D2_715.ct5
	// s in meters/second m/s
	// K = 13376 bit/mm
	// V_mark = Speed/K ==> Speed = V_mark*K
	double K = get_head_para(1, 1);
	set_mark_speed((s * K));
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
	std::cout<<"- Press any key to shut down \n";

	while (!_kbhit()); // wait until get the character

	(void)_getch();
	std::cout<<"\n";

	free_rtc5_dll();
	RTC5close();

}




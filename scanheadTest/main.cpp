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
#include <vector>
#include <list>
#include <iostream>
using namespace std;

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
//const UINT   AnalogOutChannel = 1;   //  AnalogOut Channel 1 used
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

//  Additional constants for pixelmode
const UINT   AnalogOutChannel = 1;   //  must be 1 or 2, (used in Pixelmode)
const UINT   LaserOffAnalog = (UINT)-1;   //  DefaultValue
const UINT   LaserOffDigital = (UINT)-1;   //  DefaultValue
const UINT   DefaultPixel = 0;   //  'last' pixel in Pixelmode

const UINT   AnalogBlack = 0;   //  0 - 4095
const UINT   AnalogWhite = 1023;   //  0 - 4095
const UINT   DigitalBlack = 0;   //  PulseWidth for Black
const UINT   DigitalWhite = LaserPulseWidth;//  PulseWidth for White

//  Pulse Width and AnalogOut
//  - a linear control of the pulse width and AnalogOut 
//    within the specified black and white range.
const long   AnalogGain = ((long)AnalogWhite - (long)AnalogBlack) / 255;
const long   DigitalGain = ((long)DigitalWhite - (long)DigitalBlack) / 255;

const double SpeedFactor = 1.0 / 1000.0;
const long   Offset = (long)((double)LaserOnDelay * MarkSpeed * SpeedFactor);


//  Desired Image Parameters
//  add 0 to original Demo4 program
const UINT   Pixels = 512;   //  pixels per line
const UINT   Lines = 100;   //  lines per image
const long   X_Location = -8192;   //  location of the left side of the image
const long   Y_Location = 6400;   //  location of the upper side of the image
const double DotDistance = 32.0;   //  pixel distance  [bits]
const UINT   PixelHalfPeriod = 100 * 8;   //  100 us [1/8 us] must be at least 13


// End Locus of a Line
struct locus { long xval, yval; };

struct polygonLine { long x1, y1, x2, y2;  };

unsigned char frame[Pixels][Lines];

struct image
{
	long    xLocus, yLocus;     // upper left corner of the image in bits
	double  dotDistance;        // pixel distance in bits
	UINT    dotHalfPeriod;      // pixel half period in 1/8 us
	UINT    ppl;                // pixels per line
	UINT    lpi;                // lines per image
	unsigned char* raster;      // pointer to the raster data
};

// The particular Image       
image grayChessboard =
{
	X_Location, Y_Location,
	DotDistance,
	PixelHalfPeriod,
	Pixels,
	Lines,
	&frame[0][0]
};


struct polygon
{
	LONG xval, yval;
};

const polygon square[] ={
	  {-R, -R}
	, {-R,  R}
	, {R,  R}
	, {R, -R}
	, {-R, -R}
};

const polygon triangle[] ={
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
void test1_2();                  //  Line drawing 
void test2_1();				     //	 Circle drawing
void test3_1();                  //  Square Drawing 
void test3_2(long a, double v);  //  Helpper method helps with 3_1
void test4();					 //  bitmap test
void test4_1();					 //  vector method for chessboard
void test4_2(long a, locus& xy, double v);
void test5_1();					 //  vector method for build up circle
void test5_2(std::list <polygonLine>& polygonEdgeList); // helper method for5_1
void terminateDLL();             //  waits for a keyboard hit to terminate

int PrintImage(image* picture);  
void makeChessboard( image * picture); 
								// make chess board using pixel mode           




// _cdecl is "Calling conventions"
// the stack is cleaned up by the caller
// creates larger executables than _stdcall 

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
		set_laser_pulses(LaserHalfPeriod, LaserPulseWidth);
		set_scanner_delays(JumpDelay, MarkDelay, PolygonDelay);
		set_laser_delays(LaserOnDelay, LaserOffDelay);
		set_jump_speed(JumpSpeed);
		set_mark_speed(MarkSpeed);
	set_end_of_list();

	execute_list(1);


	// Lens used for "D2_1to1.ct5" gives 
	// K = 13376 bit/mm
	// V_mark =  speed/K ==> set_mark_speed(V_mark*K)
	std::cout << "[1] Test1 draw simple line based vector\n";
	std::cout << "[2] Test2 draw circle with radius input\n";
	std::cout << "[3] Test3 Fill a square \n";
	std::cout << "[4] Test4 will make a chessboard\n";
	std::cout << "[5] Test5 will drill circular hole on fiber \n";
	std::cout << "Input 'y' to continue the same test, others to exit or switch\n";
	std::cout << "Input 's' to suspend the test [Program developing]\n";


	do {
		std::cout << "Input the test number [1/2/3/4/5]: \n";
		switch (_getch())
		{
		case '1':
			std::cout << "Test1 line drawing [x,y: 0~13mm]: \n";
			std::cout << "Start position; End Position ; Mark Speed\n";
			do {
				test1();
				std::cout << "continue? [y/n] \n";
			} while (_getch() == 'y');
			std::cout << "Test1 Done! \n \n";
			break;
		case '2':
			std::cout << "Test2 circle drawing [r: 0~13mm]: \n";
			std::cout << "Radius, start position \n";
			do {
				test2_1();
				std::cout << "continue? [y/n]\n";
			} while (_getch() == 'y');
			std::cout << "Test2 Done! \n \n";
			break;
		case '3':
			std::cout << "Test3 fill a square [d: 0~48mm]: \n";
			do {
				test3_1();
				std::cout << "continue? [y/n]\n";
			} while ( _getch() == 'y');
			break;
		case '4':
			std::cout << "Test 4 make chessboard \n";
			do {
				test4_1();
				std::cout << "continue? [y/n] \n";
			} while (_getch() == 'y');
			break;
		case '5':
			std::cout << "Test 5 drill circular hole\n";
			do {
				test5_1();
				std::cout << "continue? [y/n] \n";
			} while (_getch() == 'y');
			break;
		default:
			std::cout << "Please give valid input \n";
			break;
		}
		std::cout << "Want other test? [y/n] \n";
	} while (_getch() == 'y');
	// finish this point 
	// End of test
	std::cout << "Test Finished! \n";
	terminateDLL();
	return 0;


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

}

void test5_1() {
	
	list <polygonLine> polygonEdgeList; // list of polygons to fill in 
	double r, s, x, y;
	long i=0;
	polygonLine l;
	s = 16;
	r = 10 * 1337.6;  // radius 10 mm

	// go from y to -y
	for (x=r-s ; x > -r;  ) {
		y = sqrt(r*r - x*x);
		l.x1 = long(x);
		l.y1 = long(y);
		l.x2 = long(x);
		l.y2 = long(-y);
		polygonEdgeList.push_back(l);
		x = x-s;

		i++;
		cout <<i<<": x1: " << l.x1 << " x2: " << l.x2 << " y1: " << l.y1 << " y2: " << l.y2<<"\n";
	}

	// go from x to -x
	for (y = r - s; y > -r; ) {
		x = sqrt(r * r - y * y);
		l.x1 = long(x);
		l.y1 = long(y);
		l.x2 = long(-x);
		l.y2 = long(y);
		polygonEdgeList.push_back(l);
		y = y - s;
	}

	test5_2(polygonEdgeList);
}
void test5_2(std::list <polygonLine>& polygonEdgeList) {
	while (!load_list(1, 0)) {};
	set_mark_speed(1337.6);
	jump_abs(13376,0);
	for (std::list<polygonLine>::iterator it = polygonEdgeList.begin();
		it != polygonEdgeList.end(); ++it) {
		mark_abs(it->x1, it->y1);
		mark_abs(it->x2, it->y2);
	}
	set_end_of_list();
	execute_list(1);
}


void test4() {
	// 100,000 config list to extend the list memory
	config_list(100000, 100000);

	// preset the chessboard image filling the square
	makeChessboard(&grayChessboard);

	while (!PrintImage(&grayChessboard))
	{
		// Do something else while the RTC5 is working. For example:
		// Samsara - turning the wheels
		static char wheel[] = "||//--\\\\";
		static UINT index = 0;
		printf("\r- spending idle time %c %c", wheel[7 - index], wheel[index]);
		++index &= 7;

	}

}

// Preset pixel in an image, this one for chess board
// Unsucessful
void makeChessboard(image* picture) {

	// the square of the chess board step
	// should be 0.25 mm * 1337.6 = 334.4 bit/mm 
	// black represent 0 and white represent 255
	const UINT CHESSBOARDSTEP = 334;


	UINT i, j, k;
	UINT lineInterval, lines;
	unsigned char* pPixel, * pPixel2;
	
	lines = picture->lpi; // 100 lines per image
	lineInterval = picture->ppl; // 512 lineInterval


	// set all to black to see the effect
	// pPixel++ change of the address
	for (j = 0, pPixel = picture->raster; j < lines/2; j++) {
		for (i = 0; i < ((picture->ppl)/4) ; i++, pPixel++) {
			*pPixel = 0;
		}

		for (; i < lineInterval/2; i++, pPixel++) {
			*pPixel = 255;
		}

		for (; i < (lineInterval* 3/4); i++, pPixel++) {
			*pPixel = 0;
		}

		for (; i < lineInterval; i++, pPixel++) {
			*pPixel = 255;
		}
	}
	

}

// print out the image using build in function 
int PrintImage(image* picture){
	static int line = 0;                // current line
	static unsigned char* pPixel;       // pointer to the current pixel

	if (!line){
		pPixel = picture->raster;       // 1st pixel of the 1st line
	}

	//  Check whether the corresponding list is free to be loaded.
	//  If successful load_list returns the list number, otherwise 0 
	if (!load_list((line & 1) + 1, 0)) return 0;

	//  Now, the list is no more busy and already opened 
	//  set_start_list_pos( ( line & 1 ) + 1, 0 ) has been excuted
	set_mark_speed(10);
	// A jump to the beginning of the next line
	jump_abs(picture->xLocus - Offset,
		picture->yLocus - (long)((double)line * picture->dotDistance));

	set_pixel_line(AnalogOutChannel, PixelHalfPeriod, picture->dotDistance, 0.0);

	unsigned char Pixel(*pPixel++);
	UINT PixelCount(1);

	for (UINT i = 1; i < picture->ppl; i++, pPixel++)
	{
		//  Save list buffer space for identical pixel values
		if (*pPixel == Pixel)
		{
			PixelCount++;

		}
		else
		{
			set_n_pixel((UINT)(DigitalBlack + DigitalGain * Pixel),
				(UINT)(AnalogBlack + AnalogGain * Pixel),
				PixelCount);
			PixelCount = 1;
			Pixel = *pPixel;

		}

	}

	set_n_pixel((UINT)(DigitalBlack + DigitalGain * Pixel),
		(UINT)(AnalogBlack + AnalogGain * Pixel),
		PixelCount);

	set_end_of_list();

	// Only, apply execute_list for the first list. Otherwise, use auto_change.
	line ? auto_change() : execute_list(1);

	line++;

	if (line == picture->lpi)
	{
		line = 0;
		return 1;              // Success - image printing finished

	}

	return 0;                  // Image printing not yet finished

}

// vector method making chessboard
void test4_1() {
	std::cout << "Chessboard dimension 10*10 mm\n";
	std::cout << "Each square 2.5*2.5 mm\n";

	// 0.25*1337.6 = 334.4 approximately
	// 
	// need to add the speed parameter 1337.6*vmark
	// use 0.1 so far
	locus xy;
	int size = 8;

	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			if ( ((i + j)%2)==0 ){
				std::cout << "#";
				xy.xval = i * 334;
				xy.yval = j * 334;
				test4_2(167, xy, 133.76);
			}
			else {
				std::cout << " ";
			}
		}
		std::cout << "\n";
	}
}

// helper method for test 4_1
void test4_2(long a, locus &xy, double v) {
	// a is half of the length of square
	long c,x,y,i;
	x = xy.xval;
	y = xy.yval;
	c = a;
	
	i = 10; // printing time
	do {
		while (!load_list(1, 0)) {};
		set_mark_speed(v);
		jump_abs(-a + x, c + y);
		while (c > -a) {
			mark_abs(a + x, c + y);
			mark_abs(-a + x, c + y);
			c -= 8;
		}

		c = -a;
		jump_abs(-c + x, -a + y);

		while (c < a) {
			mark_abs(c + x, -a + y);
			mark_abs(c + x, a + y);
			c += 8;
		}

		set_end_of_list();
		execute_list(1U);
		i--;
	} while (i>0);
}

// 
// Description: test1_1
// run with the speed of algorithm inside a do while loop 
// 
void test1_1() {
	LONG a, b;
	a = 12345;
	b = 12345;

	std::cout << "Test1_1 with a: " << a << "and b: " << b << "\n";

	while (!load_list(1U, 0U)) {}
	mark_abs(a, b);
	set_end_of_list();
	execute_list(1U);

	/* original part of main 
	 * no longer needed as while loop can not help us control
	 * the speed of marking
	 *
	std::cout << "Test1_1 mark point/line, hit keyboard break";
	do {
		test1_1();
		if (_kbhit()) break;
	} while (true);
	 *
	 *
	 */
}

//
// test1_2() from 20000 to -20000
// advanced version of test1_1
//
void test1_2() {
	/*
	std::cout << "Test1_2 mark line for 10s \n";
	test1_2();
	std::cout << "Test1_2 done \n \n";
	*/
	LONG x, y, a, b, t;
	x = 20000;
	y = 20000;
	a = -20000;
	b = -20000;
	t = 1;
	std::cout << "Test1_2 with a:" << a << "and b: " << b << "\n";
	std::cout << "Test1_2 with x:" << x << "and y: " << y << "\n";
	std::cout << "Will plot for 10 seconds \n";

	while (!load_list(1, 0U)) {}
	while(t<20){
		mark_abs(a, b);
		mark_abs(x, y);
		++t;
	}

	set_end_of_list();
	execute_list(1);
}

//
// Description: 
// Test1 jump to start position 
// and mark the stuff till the end position
//
void test1() {

	// Test 4 mark a line, which should be 
    // simple using vector , I used complicated method when
    // first trying to address such problem 
	LONG startX, startY, endX, endY, t;
	double K, vMark;
	std::cout << "start point?(x,y)[mm]\n";
	std::cin >> startX;
	std::cin >> startY;
	std::cout << "end point?(x,y)[mm]\n";
	std::cin >> endX;
	std::cin >> endY;
	std::cout << " Hom many turns? \n";
	std::cin >> t;
	std::cout << " mark speed? [m/s] \n";
	std::cin >> vMark;

	if ( startX<-18 || startX > 18 ||
		startY < -18 || startY > 18 || 
		endX < -18 || endX > 18 || 
		endY < -18 || endY > 18 ) {
		std::cerr << "Out of range!!\n";
		return;// parameter protection
	}

	while (!load_list(1U, 0U)) {};

	// due to the existence of lens/objective
	// divide by 10 is necessary for accurate position?
	K = get_head_para(1, 1);
	set_mark_speed( vMark*K/10 );
	startX = startX * K /10;
	startY = startY * K /10;
	endX = endX * K /10;
	endY = endY * K /10;

	jump_abs(startX, startY);
	while ( t>0) {
		mark_abs(endX, endY);
		mark_abs(startX, startY);
		t--;
	}
	set_end_of_list();
	execute_list(1U);
}


//
// the test of drawing a circle with respect to the center
// To get safe working range of the laser
//
void test2_1() {
	LONG r, t;
	double K, vMark;
	std::cout << "Radius of circle? \n";
	std::cin >> r;
	std::cout << " mark speed? [m/s] \n";
	std::cin >> vMark;
	std::cout << "How many turns? [ max 10000 ] \n";
	std::cin >> t;

	if (r < 0 || r>24) {
		std::cerr << "Out of range!!\n";
		return;// parameter protection
	}

	while (!load_list(1, 0)) {};
	K = get_head_para(1, 1);
	set_mark_speed(vMark * K / 10);
	r = r * K / 10;

	jump_abs(r, 0);
	while (t > 0) {
		arc_abs(0, 0, 360 );
		t--;
	}
	
	set_end_of_list();
	execute_list(1);
}

void test3_1() {

	long a, c, t;
	double K, vMark;

	std::cout << "How many times? \n";
	std::cin >> t;
	std::cout << "Length of square? \n";
	std::cin >> a;
	std::cout << " mark speed? [m/s] \n";
	std::cin >> vMark;

	if (a < 0 || a>36) {
		std::cerr << "Out of range!!\n";
		return;// parameter protection
	}


	while (!load_list(1, 0)) {};
	K = get_head_para(1, 1);
	set_mark_speed(vMark * K / 10);
	a = long(a * K / 20);
	c = a;
	jump_abs(-a, c);
	while (c > -a) {
		mark_abs(a, c);
		mark_abs(-a, c);
		c -= 50;
	}

	c = -a;
	jump_abs(-c, -a);

	while (c < a) {
		mark_abs(c,-a);
		mark_abs(c,a);
		c += 50;
	}

	set_end_of_list();
	execute_list(1U);


	// Unit not complete
	// Still have several problems
	while (t > 1) { 
		
		while (!load_list(1, 0)) {
			if (_kbhit()) {
				switch (_getch()) {
				case 's':
				case 'S':
					pause_list();
					std::cout << "Plot Suspend, Press any key to continue\n";
					break;
				case 'e':
				case 'E':
					restart_list();
					stop_execution();
					std::cout << "--Exit Plot process--\n";
					break;
				default:
					std::cout << "Command not clear, enter [e/s] for exit and pause\n";
				}
			}
		}

		test3_2(a, vMark * K / 10.0);
		t--;
	}
}

//
// Description: Test 3_2 
// the drawing based on the 3_1 
// Could repeat as many times as possible
// 
void test3_2( long a, double v) {
	long c;
	while (!load_list(1, 0)) {};
	set_mark_speed(v);
	c = a;
	jump_abs(-a, c);
	while (c > -a) {
		mark_abs(a, c);
		mark_abs(-a, c);
		c -= 50;
	}

	c = -a;
	jump_abs(-c, -a);

	while (c < a) {
		mark_abs(c, -a);
		mark_abs(c, a);
		c += 50;
	}

	set_end_of_list();
	execute_list(1U);
}


// 
// Description: draw warmup program 
// used for test/example
// Draw to warm up, given function 
// Please do not use _getch() directly to catch the keyboard input
// use kbhit together for stop a loop
void draw(const polygon* figure, const size_t& size)
{

	/*
	std::cout << "Test 0 Warm Up, draw a square and a triangle, hit keyboard break \n";
	do {
		draw(square, sizeof(square) / sizeof(square[0]));
		draw(triangle, sizeof(triangle) / sizeof(triangle[0]));
		if (_kbhit()) break;
	} while (true);
	*/

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



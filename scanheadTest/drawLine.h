//-----------------------------------------------------------------------------
//  File: drawLine.h                            Copyright (c) Popmintchev Labs
//-----------------------------------------------------------------------------
//
//
//
//  Abstract:
//			This header file defines function prototype and some constant value
//	for the laser to execute smoothly
//
//-----------------------------------------------------------------------------

#pragma once

// Definition of "pi"
const double Pi = 3.14159265358979323846;

// Change these values according to your system
// Laser 
const UINT   DefaultCard = 1;      //  number of default card
const UINT   ListMemory = 10000;   //  size of list 1 memory (default 4000)
const UINT   LaserMode = 1;        //  YAG 1 mode
const UINT   LaserControl = 0x18;  //  Laser signals LOW active (Bits #3 and #4)
const UINT   StartGap = 1000;      //  gap ahead between input_pointer and out_pointer
const UINT   LoadGap = 100;        //  gap ahead between out_pointer and input_pointer
const UINT   PointerCount = 0x3F;  //  pointer mask for checking the gap

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
struct locus { double xval, yval; };

struct polygon
{
	long xval, yval;
};


const locus BeamDump = { -32000, -32000 }; //  Beam Dump Location

int initialRTC(void);
void initialLaser(void);
int PlotLine(const locus& destination, UINT* start);
int drawStraightLine(const locus& startPosition, const locus& endPosition, bool shapeOrLine);
void setMarkSpeed(long& s);
void miliToBit(double& b);
void PlotFlush();
void terminateDLL();            //  waits for a keyboard hit to terminate
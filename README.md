# laserScanhead
The development of Scanhead control system

ScanheadTest
gives various test on list command such as:  
config_list()     //adding memory, default (4000,4000) two lists  
load_list()       //load_list(1,0) takes the first list  
set_end_of_list() //  
execute_list()    //   
home_position()  
  
mark_abs()     //vector mark   
jump_abs()     //jump to certain position, might realize random jump later  
arc_abs()      // draw circular arc command (x,y,rad) x,y gives center rad gives degree from -3600 to 3600  
  
Various combination and GUI feature might be added later

TEST1~TEST6 implemented basic functionality used in laser scanhead [Scancube 10] with Pharso Laser. 
Customized hardware feature.

Enable/Disable laser customized command:
set_laser_control(0x18); // gives 5V signal through D-SUB 15 port PIN2-->PIN11, RTC5Board-->Laser
set_laser_control(0); // gives 0V signal through D-SUB 15 port PIN10->PIN1, RTC5Board-->Laser

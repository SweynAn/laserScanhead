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

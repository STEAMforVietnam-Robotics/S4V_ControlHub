#include "System.h"
#include "../robot/Robot.h"

SystemClass SysCtrl;

// Route SysCtrl.stop() through the Robot's stop() so it actually cuts drive.
// (Kept out of the header to avoid a heavy include from every user sketch.)
void SystemClass::stop()
{
    Robot.stop();
}

/*  System.h
 *
 *  Convenience wrappers for the Level-2 "system" namespace of the API
 *  reference (system.wait_sec / system.wait_millisec / system.stop).
 *
 *  Because @c system is a reserved-ish name in Arduino sketches (there is a
 *  global @c system() function coming from newlib) we expose the singleton
 *  as @c SysCtrl. If you insist on the literal @c system.wait_sec() spelling
 *  you can do:
 *
 *      #define system SysCtrl
 *
 *  right before including <ControlHub.h> in your sketch.
 */

#ifndef CONTROLHUB_SYSTEM_H
#define CONTROLHUB_SYSTEM_H

#include <Arduino.h>

class SystemClass {
public:
    void  wait          (float seconds)  { delay((uint32_t)(seconds * 1000.0f)); }
    void  wait_sec      (float seconds)  { wait(seconds); }
    void  wait_millisec (uint32_t ms)    { delay(ms); }

    /** Cut the drivetrain (routes through the global Robot). */
    void  stop();

    void  reset() { ESP.restart(); }
};

extern SystemClass SysCtrl;

#endif

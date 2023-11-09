/* 
 * File:   main.c
 * Author: pconroy
 *
 * Created on November 8, 2023, 10:09 PM
 */

#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <log4c.h>
#include <libepsolar.h>
/*
 * 
 */
int main(int argc, char** argv) 
{
    const char *devPort = "/dev/ttyXRUSB0";
    
    Logger_Initialize( "/home/pconroy/epsolar_lowbattery_shutdown.log", 5 );
    Logger_LogWarning( "Version: %s\n", "v1.0"  );
    Logger_LogWarning( "Remember 2 minute shutdown delay, so don't run this more than every 5 minutes or so!\n" );

    if (!epsolarModbusConnect( devPort, 1 )) {
        Logger_LogFatal( "Unable to open %s to connect to the solar charge controller\n", devPort );
        return (EXIT_FAILURE);
    }

    Logger_LogInfo( "Setting controller clock to 'now' and Load Control to Manual\n" );
    eps_setRealtimeClockToNow();
    eps_setLoadControllingMode( 0 );
    eps_forceLoadOn();

    
    float   batteryVoltage = eps_getBatteryVoltage();
    Logger_LogWarning( "Battery Voltage: %f\n", batteryVoltage );

    if (batteryVoltage < 11.9) {
        Logger_LogWarning( "Shutting down for two hours\n" );
        
        //
        // What time is it now?
        time_t  current_time = time( NULL );
        struct  tm  *tmPtr;
 
        if (current_time > 0) {
            tmPtr = localtime( &current_time );     /* Convert to local time format. */
            int hour = tmPtr->tm_hour;
            int minute = tmPtr->tm_min;
            
            //
            // Turn ON two hours from now -- add two.
            hour = ((hour + 2) % 24);
            
            // Turn OFF two minutes from now -- add two
            minute = ((minute + 2) % 60);
            
            Logger_LogInfo( "Setting Timer 1 ON to: %d:%d:%d\n", hour, tmPtr->tm_min, tmPtr->tm_sec );
            eps_setTurnOnTiming1( hour, tmPtr->tm_min, tmPtr->tm_sec );
            
            Logger_LogInfo( "Setting Timer 1 OFF to: %d:%d:%d\n", tmPtr->tm_hour, minute, tmPtr->tm_sec );
            eps_setTurnOffTiming1( tmPtr->tm_hour, minute, tmPtr->tm_sec );

            Logger_LogInfo( "Setting Load Control Mode to 0x03 - Timer Control\n" );
            eps_setLoadControllingMode( 3 );
        }
    }
    
    Logger_LogInfo( "Disconnecting from charge controller\n" );
    epsolarModbusDisconnect();
    
    Logger_LogWarning( "Low Voltage Check complete\n");
    Logger_Terminate();
    return (EXIT_SUCCESS);
}
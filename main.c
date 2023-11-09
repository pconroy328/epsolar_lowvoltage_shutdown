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
    
    uint16_t     batteryStatusBits = eps_getBatteryStatusBits();
    Logger_LogInfo( "Battery Status Bits: battery status voltage: %s\n", getBatteryStatusVoltage( batteryStatusBits ));
    Logger_LogInfo( "Battery Status Bits: battery status temperature: %s\n", getBatteryStatusTemperature( batteryStatusBits ));
    Logger_LogInfo( "Battery Status Bits: battery status inner resistance: %s\n", getBatteryStatusInnerResistance( batteryStatusBits ));
    Logger_LogInfo( "Battery Status Bits: battery status identification: %s\n", getBatteryStatusIdentification( batteryStatusBits ));
    
    uint16_t  chargingStatusBits   = eps_getChargingEquipmentStatusBits();
    Logger_LogInfo( "Charging Status Bits: charging input voltage: %s\n", getChargingEquipmentStatusInputVoltageStatus( chargingStatusBits ));
    Logger_LogInfo( "Charging Status Bits: charging MOSFET shorted: %s\n", (isChargingMOSFETShorted( chargingStatusBits ) ? "Yes" : "No" ));
    Logger_LogInfo( "Charging Status Bits: charging MOSFET open: %s\n", (isChargingMOSFETOpen( chargingStatusBits ) ? "Yes" : "No" ));
    Logger_LogInfo( "Charging Status Bits: anti reverse MOSFET shorted: %s\n", (isAntiReverseMOSFETShort( chargingStatusBits ) ? "Yes" : "No" ));
    
    Logger_LogInfo( "Charging Status Bits: input over current: %s\n", (isInputOverCurrent( chargingStatusBits ) ? "Yes" : "No" ));
    Logger_LogInfo( "Charging Status Bits: load over current: %s\n", (isLoadOverCurrent( chargingStatusBits ) ? "Yes" : "No" ));
    Logger_LogInfo( "Charging Status Bits: load shorted: %s\n", (isLoadShorted( chargingStatusBits ) ? "Yes" : "No" ));
    
    Logger_LogInfo( "Charging Status Bits: load MOSFET shorted: %s\n", (isLoadMOSFETShorted( chargingStatusBits ) ? "Yes" : "No" ));
    Logger_LogInfo( "Charging Status Bits: disequilibrium in three: %s\n", (isDisequilibriumInThreeCircuits( chargingStatusBits ) ? "Yes" : "No" ));
    Logger_LogInfo( "Charging Status Bits: PV input shorted: %s\n", (isPVInputShorted( chargingStatusBits ) ? "Yes" : "No" ));

    Logger_LogInfo( "Charging Status Bits: charging status: %s\n", getChargingStatus( chargingStatusBits ));
    Logger_LogInfo( "Charging Status Bits: charging status normal: %s\n", (isChargingStatusNormal( chargingStatusBits ) ? "Normal" : "Fault" ));
    Logger_LogInfo( "Charging Status Bits: charging status running: %s\n", (isChargingStatusRunning( chargingStatusBits ) ? "Running" : "Standby" ));

            
    uint16_t  dischargingStatusBits   = eps_getdisChargingEquipmentStatusBits();
    Logger_LogInfo( "Discharging Status Bits: discharging status: %s\n", getDischargingStatusInputVoltageStatus( chargingStatusBits ));
    Logger_LogInfo( "Discharging Status Bits: discharging power: %s\n", getDischargingStatusOutputPower( chargingStatusBits ));
    Logger_LogInfo( "Discharging Status Bits: discharging shorted: %s\n", (isdischargeStatusShorted( chargingStatusBits ) ? "Yes" : "No" ));
    Logger_LogInfo( "Discharging Status Bits: unable to discharging: %s\n", (isdischargeStatusUnableToDischarge( chargingStatusBits ) ? "Yes" : "No" ));
    Logger_LogInfo( "Discharging Status Bits: unable to stop discharge: %s\n", (isdischargeStatusUnableToStopDischarge( chargingStatusBits ) ? "Yes" : "No" ));

    Logger_LogInfo( "Discharging Status Bits: output voltage abnormal: %s\n", (isdischargeStatusOutputVoltageAbnormal( chargingStatusBits ) ? "Yes" : "No" ));
    Logger_LogInfo( "Discharging Status Bits: input over voltage: %s\n", (isdischargeStatusInputOverVoltage( chargingStatusBits ) ? "Yes" : "No" ));
    Logger_LogInfo( "Discharging Status Bits: short in high voltage: %s\n", (isdischargeStatusShortedInHighVoltage( chargingStatusBits ) ? "Yes" : "No" ));
    Logger_LogInfo( "Discharging Status Bits: boost over voltage: %s\n", (isdischargeStatusBoostOverVoltage( chargingStatusBits ) ? "Yes" : "No" ));
    Logger_LogInfo( "Discharging Status Bits: output over voltage: %s\n", (isdischargeStatusOutputOverVoltage( chargingStatusBits ) ? "Yes" : "No" ));
    Logger_LogInfo( "Discharging Status Bits: load status: %s\n", (isdischargeStatusNormal( chargingStatusBits ) ? "Normal" : "Fault" ));
    Logger_LogInfo( "Discharging Status Bits: load running: %s\n", (isdischargeStatusRunning( chargingStatusBits ) ? "Running" : "Standby" ));
    
    
    
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
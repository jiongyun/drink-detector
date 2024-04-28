#include "myApp.h"
#include "interface.h"

// sensor libraries for data interperting
#include "meas_acc/resources.h"
#include "meas_gyro/resources.h"
#include "meas_magn/resources.h"
#include "meas_imu/resources.h"

//Commands formated as byte array with [Command, data?...] (data optional)
enum Commands 
{
    HELLO = 0,
    BEGIN_SUB=1,
    END_SUB=2,
    BLINK_LED = 3
};
//Responses formated as byte array with [Response, tag, data?...] (data optional)
enum Responses 
{
    COMMAND_RESULT = 1,
    DATA = 2,
    ERROR = 3,
};

          
const char IMUPath[]="/Meas/IMU6/104";
const uint8_t DEFAULT_REFERENCE=99; //appears as 63 in hex


bool detectSip(const WB_RES::IMU6Data& imuData) {
    // Ensure arrays are not empty
    if (imuData.arrayAcc.size() > 0 && imuData.arrayGyro.size() > 0) {
        // Get the first (or relevant) set of accelerometer and gyroscope data
        auto accelData = imuData.arrayAcc[0]; // This is a FloatVector3D
        auto gyroData = imuData.arrayGyro[0]; // This is also a FloatVector3D

        // Extract the Z-axis acceleration and X-axis gyroscope data
        float accelZ = accelData.z; // Assuming Z-axis is the third component
        float gyroX = gyroData.x;  // Assuming X-axis is the first component

        // Placeholder logic for sip detection
        const float ACCEL_THRESHOLD = 0.5;
        const float GYRO_THRESHOLD = 0.5;

        if (accelZ > ACCEL_THRESHOLD && abs(gyroX) < GYRO_THRESHOLD) {
            return true;
        }
    }
    return false;
}



void myApp::handleCommand(uint8_t cmd, const uint8_t values[], size_t len){
    switch (cmd)
    {
        case Commands::HELLO:
        {
            // Hello response, for use as a sanity check <3
            uint8_t helloMsg[] = {'H','e','l','l','o','!'};
            //tags aren't explicitly necessary, but they're a good way of grouping responses.
            //The included console uses them to filter and format responses.
            uint8_t tag=1;
            sendPacket(helloMsg, sizeof(helloMsg), tag, Responses::COMMAND_RESULT);
        }
        break;
        case Commands::BEGIN_SUB:
        {
            //unsubscribes to prevent duplicate subscriptions
            unsubscribe(DEFAULT_REFERENCE);
            //subscribes to the path given above, in this case the IMU at 104hz
            subscribe(IMUPath, sizeof(IMUPath), DEFAULT_REFERENCE);
        }
        break;
        case Commands::END_SUB:
        {
            //unsubscribes only from default service
            unsubscribe(DEFAULT_REFERENCE);
        }
        break;
        case Commands::BLINK_LED:
        {
            uint8_t blinkMsg[] = {'B','l','i','n','k','!'};
            blinkLed();
            
            uint8_t tag = 2;
            sendPacket(blinkMsg, sizeof(blinkMsg), tag, Responses::COMMAND_RESULT);
    }
        break;
    

    }
}

void myApp::processData(wb::ResourceId resourceId, const wb::Value &value){
    if (resourceId.localResourceId == WB_RES::LOCAL::MEAS_IMU6::LID) {
        // Parse the IMU data from 'value'
        const WB_RES::IMU6Data &imuData = value.convertTo<const WB_RES::IMU6Data&>();
      

        // Call the sip detection logic
        if (detectSip(imuData)) {
            // Trigger actions like blinking LED
            sipCounter++;
            blinkLed();

            // Update sip count on web console UI
            char sipCountMsg[50];
            sprintf(sipCountMsg, "Sip Count: %d", sipCounter);
            sendPacket(reinterpret_cast<const uint8_t*>(sipCountMsg), strlen(sipCountMsg), 0, Responses::DATA);
        }
    }
    
}

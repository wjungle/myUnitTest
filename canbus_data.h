#define CANBUS_DATA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

    #define EXT_MAX_QUEUE_SIZE 8
    #define MAX_OUTDATA_SIZE 64

    /* VCU_STARTUP_INITIAL_COMMAND */
    struct _StartInitialSettingStruct {
        uint8_t powerSignal;
        uint8_t clockHour;
        uint8_t clockMinutes;
        uint16_t interfaceLanguage;
        uint8_t counter;
        uint8_t speedUnit;

    };

    typedef struct _StartInitialSettingStruct StartInitialSettingStruct;

    void StartInitialQueueInit();
    int StartInitialQueueReceive(StartInitialSettingStruct* ev);
    int StartInitialQueueSend(StartInitialSettingStruct* ev);

    /* VCU_VEHICLE_SPEED_ODOMETER_COMMAND */
    struct _SpeedOdometerStruct {
        uint8_t speed;
        uint16_t batteryCapability;
        uint8_t acceleration;
        uint32_t totalOdometer;

    };

    typedef struct _SpeedOdometerStruct SpeedOdometerStruct;

    void SpeedOdometerQueueInit();
    int SpeedOdometerQueueReceive(SpeedOdometerStruct* ev);
    int SpeedOdometerQueueSend(SpeedOdometerStruct* ev);

    /* VCU_BUTTON_LIGHT_CONTROL_COMMAND */
    struct _ButtonAndLightControlStruct {
        bool isAutoControlBackLight;
        uint8_t brightness;
        bool redLightFlash;
        bool greenLightFlash;
        bool redLightEnable;
        bool greenLightEnable;
        uint8_t engineMode;
        uint8_t gearState;
        bool tirePrssureAbnormal;
        bool bluetoothAbnormal;
        bool warrantyAbnormal;
        bool repairState;
        bool engineControllerTemperatureAbnormal;
        bool engineTemperatureAbnormal;
        bool ABSState;
        bool TCSState;
        bool bluetoothVisible;
        bool highBeamHeadlightVisible;
        bool lowBeamHeadlightVisible;
        bool batterySignalVisible;
        bool turnIndicatorVisible;
        bool rightIndicatorVisible;
        bool leftIndicatorVisible;
        bool bluetoothSignal;
        bool speedLimitVisible;
        bool navigationSpeedVisible;
        bool regenFlashEnable;
        uint8_t switchLayer;

    };

    typedef struct _ButtonAndLightControlStruct ButtonAndLightControlStruct;

    void ButtonLightControlQueueInit();
    int ButtonLightControlQueueReceive(ButtonAndLightControlStruct* ev);
    int ButtonLightControlQueueSend(ButtonAndLightControlStruct* ev);

    /* VCU_VEHICLE_RETURN_INFO_COMMAND */
    struct _VCUReturnInfoStruct {
        uint16_t brightnessInfo;
        uint8_t motoWorkingState;
        bool smartKeyVisible;
        bool powerTrainVisible;
        bool dashboardVisible;
        bool controlVisible;
        bool batteryVisible;
        bool otherVisible;
        uint16_t errorCode;
        uint8_t batteryTemperature;
        uint8_t batterySOC;

    };

    typedef struct _VCUReturnInfoStruct VCUReturnInfoStruct;

    void VCUReturnInfoQueueInit();
    int VCUReturnInfoQueueReceive(VCUReturnInfoStruct* ev);
    int VCUReturnInfoQueueSend(VCUReturnInfoStruct* ev);

    /* VCU_TRIP_WARRANTY_MILEAGE_COMMAND */
    struct _TripWarrantyStruct {
        uint16_t trip1Mileage;
        uint16_t trip2Mileage;
        uint16_t warrantyMileage;
        uint8_t navigationRegenSetting;
        bool warrantyMileageVisible;
        bool tirePressureVisible;
        bool errorCodeVisible;
        bool trip2MileageVisible;
        bool trip1MileageVisible;
        bool batteryMileageVisible;
        bool totalOdometerVisible;

    };

    typedef struct _TripWarrantyStruct TripWarrantyStruct;

    void TripWarrantyQueueInit();
    int TripWarrantyQueueReceive(TripWarrantyStruct* ev);
    int TripWarrantyQueueSend(TripWarrantyStruct* ev);

    /* VCU_GPS_COORDINATE_COMMAND */
    struct _GPSCoordinateStruct {
        uint32_t coordinateX;
        uint32_t coordinateY;

    };

    typedef struct _GPSCoordinateStruct GPSCoordinateStruct;

    /* VCU_RETURN_VALUE_INFO_COMMAND */
    struct _VCUReturnValueInfoStruct {
        uint8_t speedLimited;
        uint8_t navigationSpeed;
        uint8_t totalRidingTime;
        uint8_t remainRegenHour;
        uint8_t remainRegenMinutes;

    };

    typedef struct _VCUReturnValueInfoStruct VCUReturnValueInfoStruct;

    void VCUReturnValueInfoQueueInit();
    int VCUReturnValueInfoQueueReceive(VCUReturnValueInfoStruct* ev);
    int VCUReturnValueInfoQueueSend(VCUReturnValueInfoStruct* ev);

    /* DB_RETURN_INFO_COMMAND */
    struct _DBReturnInfoStruct {
        int8_t dbMeterTemperature;
        uint8_t dbPowerSignal;
        uint16_t dbBrightnessInfo;
        uint16_t dbErrorCode;
        uint8_t dbCounter;
        uint8_t dbLayerState;

    };

    typedef struct _DBReturnInfoStruct DBReturnInfoStruct;

    void DBReturnInfoQueueInit();
    int DBReturnInfoQueueReceive(DBReturnInfoStruct* ev);
    int DBReturnInfoQueueSend(DBReturnInfoStruct* ev);

#ifdef __cplusplus
}
#endif

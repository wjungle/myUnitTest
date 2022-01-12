#ifndef COM_LAYER_MDL_H
#define COM_LAYER_MDL_H

#define PROTOCOL_INTERVAL   100 //ms

typedef enum {
    ID_STARTUP_INIT = 0x00,
    ID_SPD_ODO      = 0x01,
    ID_BTN_LGT      = 0x02,
    ID_VCU_INF      = 0x03,
    ID_TRIP         = 0x04,
    ID_GPS          = 0x05,
    ID_VCU_INF_2    = 0x06,
    ID_DB_INF       = 0x10,
} NUC_2_ITE_ID;

typedef enum {
    CTRL_CAN = 0x00,
    CTRL_CAN_EXT,
    CTRL_CAN_NON,
} NUC_CTRL_ID;

typedef enum {
    TURN_LEFT = 0x0,
    TURN_RIGHT,
    TURN_BOTH,
    TURN_MAX,
} TURN_STATUS;

typedef struct {
    unsigned char len;
    unsigned char ctrl;
    unsigned char id[2];
    unsigned char data[8];
} OutputProtocol;

void initialStartInitial(StartInitialSettingStruct *StartInitial);
void initialSpeedOdometer(SpeedOdometerStruct *SpeedOdometer);
void initialButtonLightControl(ButtonAndLightControlStruct *ButtonAndLightControl);
void initialVehicleReturnInfo(VCUReturnInfoStruct *VehicleReturnInfo);
void initialTripWarrantyMileage(TripWarrantyStruct *TripWarrantyMileage);
void initialReturnValueInfo(VCUReturnValueInfoStruct *ReturnValueInfo);
void initialDbReturnInfo(DBReturnInfoStruct *DbReturnInfo);

// 0x600
void InitialCommonLayerModel();
void setPower(uint8_t powerSignal);
void setClock(uint8_t hour, uint8_t minutes);
void setSpdUnit(uint8_t unit);
// 0x601
void setSpd(uint8_t spd);
void setSpdAuto(void);
void setRemainRange(uint16_t remain);
// 0x602
void setEngineMode(uint8_t mode);
void setGearState(uint8_t status);
void setBattaryOn(bool val);
void setTurnSignalOn(bool val);
void setTurnRightOn(bool val);
void setTurnLeftOn(bool val);
void setTurnAuto(void);
void setSpdLmtOn(bool val);
void setCruConOn(bool val);
void setChargingChanging(bool val);
void setPage(uint8_t page);
void setTirePressure(bool status, uint8_t frontTire, uint8_t rearTire);
void setErrorCode(uint16_t code);
// 0x603
void setBikeWorkStatus(uint8_t status);
void setBatTemp(uint8_t temp);
void setBatSoc(uint8_t soc);
// 0x604
void setRecharge(uint8_t recharge);
//0x606
void setSpdLmt(uint8_t spd);
void setCruCon(uint8_t spd);
void setRemainChargeTime(uint8_t hour, uint8_t minutes);

void setSimVcuData(unsigned char id, OutputProtocol *simNucData);
void setSimNucData(unsigned char id, OutputProtocol *simNucData);
void setSendRepeat(bool val);
bool getSendRepeat(void);


#endif
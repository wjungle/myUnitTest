#include <stdio.h>
#include "canbus_data.h"
#include "common_layer_model.h"
#include "caution_error.h"

#define SPD_MAX 160
#define SPD_MIN 0

int gSpdAuto = false;
int gTurnAuto = false;
int gSendRepeat = false;

/* canbus data struct */
StartInitialSettingStruct   _600;
SpeedOdometerStruct         _601; 
ButtonAndLightControlStruct _602;
VCUReturnInfoStruct         _603;
TripWarrantyStruct          _604;
//GPSCoordinateStruct         _605;
VCUReturnValueInfoStruct    _606;
DBReturnInfoStruct          _610;


void InitialCommonLayerModel() {
        initialStartInitial(&_600);
        initialSpeedOdometer(&_601);
        initialButtonLightControl(&_602);
        initialVehicleReturnInfo(&_603);
        initialTripWarrantyMileage(&_604);
        initialReturnValueInfo(&_606);
        initialDbReturnInfo(&_610);
}

void initialStartInitial(StartInitialSettingStruct *StartInitial) {
    StartInitial->powerSignal = 0;
    StartInitial->clockHour = 0;
    StartInitial->clockMinutes = 0;
    StartInitial->interfaceLanguage = 0x0376;
    StartInitial->counter = 0;
    StartInitial->speedUnit = 0x0A;
}

void initialSpeedOdometer(SpeedOdometerStruct *SpeedOdometer) {
    SpeedOdometer->speed = 0;
    SpeedOdometer->batteryCapability = 0;
    SpeedOdometer->acceleration = 0;
    SpeedOdometer->totalOdometer = 0;
}

void initialButtonLightControl(ButtonAndLightControlStruct *ButtonAndLightControl) {
    ButtonAndLightControl->isAutoControlBackLight = false;
    ButtonAndLightControl->brightness = 0;
    ButtonAndLightControl->redLightFlash = false;
    ButtonAndLightControl->greenLightFlash = false;
    ButtonAndLightControl->redLightEnable = false;
    ButtonAndLightControl->greenLightEnable = false;
    ButtonAndLightControl->engineMode = 0;
    ButtonAndLightControl->gearState = 0;
    ButtonAndLightControl->tirePrssureAbnormal = false;
    ButtonAndLightControl->bluetoothAbnormal = false;
    ButtonAndLightControl->warrantyAbnormal = false;
    ButtonAndLightControl->repairState = false;
    ButtonAndLightControl->engineControllerTemperatureAbnormal = false;
    ButtonAndLightControl->engineTemperatureAbnormal = false;
    ButtonAndLightControl->ABSState = false;
    ButtonAndLightControl->TCSState = false;
    ButtonAndLightControl->bluetoothVisible = false;
    ButtonAndLightControl->highBeamHeadlightVisible = false;
    ButtonAndLightControl->lowBeamHeadlightVisible = false;
    ButtonAndLightControl->batterySignalVisible = false;
    ButtonAndLightControl->turnIndicatorVisible = false;
    ButtonAndLightControl->rightIndicatorVisible = false;
    ButtonAndLightControl->leftIndicatorVisible = false;
    ButtonAndLightControl->bluetoothSignal = false;
    ButtonAndLightControl->speedLimitVisible = false;
    ButtonAndLightControl->navigationSpeedVisible = false;
    ButtonAndLightControl->regenFlashEnable = false;
    ButtonAndLightControl->switchLayer = 0x00;
}

void initialVehicleReturnInfo(VCUReturnInfoStruct *VehicleReturnInfo) {
    VehicleReturnInfo->brightnessInfo = 0;
    VehicleReturnInfo->motoWorkingState = 0x20;
    VehicleReturnInfo->smartKeyVisible = false;
    VehicleReturnInfo->powerTrainVisible = false;
    VehicleReturnInfo->dashboardVisible = false;
    VehicleReturnInfo->controlVisible = false;
    VehicleReturnInfo->batteryVisible = false;
    VehicleReturnInfo->otherVisible = false;
    VehicleReturnInfo->errorCode = 0;
    VehicleReturnInfo->batteryTemperature = 0;
    VehicleReturnInfo->batterySOC = 0;
}

void initialTripWarrantyMileage(TripWarrantyStruct *TripWarrantyMileage) {
    TripWarrantyMileage->trip1Mileage = 0;
    TripWarrantyMileage->trip2Mileage = 0;
    TripWarrantyMileage->warrantyMileage = 0;
    TripWarrantyMileage->navigationRegenSetting = 0xFF;
    TripWarrantyMileage->warrantyMileageVisible = false;
    TripWarrantyMileage->tirePressureVisible = false;
    TripWarrantyMileage->errorCodeVisible = false;
    TripWarrantyMileage->trip2MileageVisible = false;
    TripWarrantyMileage->trip1MileageVisible = false;
    TripWarrantyMileage->batteryMileageVisible = false;
    TripWarrantyMileage->totalOdometerVisible = false;
}

void initialReturnValueInfo(VCUReturnValueInfoStruct *ReturnValueInfo) {
    ReturnValueInfo->speedLimited = 0;
    ReturnValueInfo->navigationSpeed = 0;
    ReturnValueInfo->totalRidingTime = 0;
    ReturnValueInfo->remainRegenHour = 0;
    ReturnValueInfo->remainRegenMinutes = 0;
}

void initialDbReturnInfo(DBReturnInfoStruct *DbReturnInfo) {
    DbReturnInfo->dbMeterTemperature = 0;
    DbReturnInfo->dbPowerSignal = 0;
    DbReturnInfo->dbBrightnessInfo = 0;
    DbReturnInfo->dbErrorCode = 0;
    DbReturnInfo->dbCounter = 0;
    DbReturnInfo->dbLayerState = 0x00;
}

unsigned char getSpeedAuto(void)
{
    static int toggle = 1;
    static unsigned char gSpeed = 0;
    
    if ((gSpeed == SPD_MAX && toggle == 1) ||
        (gSpeed == SPD_MIN && toggle == 0))
        toggle ^= 1;
    
    if (toggle == 1)
        gSpeed++;
    else
        gSpeed--;
    
    return gSpeed;
}

void getTurnAuto(void)
{
    static int turnStatus = 0;
    static int turnCnt = 0;
    static int turnOnOff = 1;
    int times = (333/PROTOCOL_INTERVAL);//333
                
    if (turnStatus == TURN_LEFT)
    {
        setTurnSignalOn(turnOnOff);
        setTurnRightOn(0);
        setTurnLeftOn(1);
        //printf("Turn Left(%d)\n", turnOnOff);
    }
    else if (turnStatus == TURN_RIGHT)
    {
        setTurnSignalOn(turnOnOff);
        setTurnRightOn(1);
        setTurnLeftOn(0);  
        //printf("Turn Right(%d)\n", turnOnOff);
    }
    else if (turnStatus == TURN_BOTH)
    {
        setTurnSignalOn(turnOnOff);
        setTurnRightOn(1);
        setTurnLeftOn(1);   
        //printf("Turn L&R(%d)\n", turnOnOff);
    }
    turnCnt++;
    if (turnCnt%times == 0)
        turnOnOff^=1;
    if (turnCnt%(times*6) == 0)//6
        turnStatus++;
    turnStatus%=TURN_MAX; 
}

void setSendRepeat(bool val)
{
    gSendRepeat = val;
}

bool getSendRepeat(void)
{
    return gSendRepeat;
}

void setPower(uint8_t powerSignal)
{
    _600.powerSignal = powerSignal;
}

void setClock(uint8_t hour, uint8_t minutes)
{
    _600.clockHour = hour;
    _600.clockMinutes = minutes;
}

void setSpdUnit(uint8_t unit)
{
    _600.speedUnit = unit;
}

void setSpd(uint8_t spd)
{
    _601.speed = spd;
}

static void SpdAuto(void)
{
    _601.speed = getSpeedAuto();
}

void setSpdAuto(void)
{
    gSpdAuto = true;
}

void setRemainRange(uint16_t remain)
{
    _601.batteryCapability = remain;
}

void setEngineMode(uint8_t mode)
{
    _602.engineMode = mode;
}

void setGearState(uint8_t status)
{
    _602.gearState = status;
}

void setBattaryOn(bool val)
{
    _602.batterySignalVisible = val;
}

void setTurnSignalOn(bool val)
{
    _602.turnIndicatorVisible = val;
}

void setTurnRightOn(bool val)
{
    _602.rightIndicatorVisible = val;
}

void setTurnLeftOn(bool val)
{
    _602.leftIndicatorVisible = val;
}

static void TurnAuto(void)
{
    getTurnAuto(); 
}

void setTurnAuto(void)
{
    gTurnAuto = true;
}

void setSpdLmtOn(bool val)
{
    _602.speedLimitVisible = val;
}

void setCruConOn(bool val)
{
    _602.navigationSpeedVisible = val;
}

void setChargingChanging(bool val)
{
    _602.regenFlashEnable = val;
}

void setPage(uint8_t page)
{
    _602.switchLayer = page;
}

void setTirePressure(bool status, uint8_t frontTire, uint8_t rearTire)
{
    _602.tirePrssureAbnormal = status;
    _603.errorCode = (frontTire << 8) | (rearTire);
}

void setErrorCode(uint16_t code)
{
    int result;
    int n;

    n = sizeof(error_items) / sizeof(error_items[0]);
    result = binarySearch(error_items, 0, n - 1, code);    
    if (result == -1) {
        printf("\n\tincorrect error code\n");
        return;
    }
    //printf("\ttype = %d\n", error_items[result].type);
    
     _603.errorCode = code;
    
    _603.controlVisible = 0;
    _603.powerTrainVisible = 0;
    _603.smartKeyVisible = 0;
    _603.batteryVisible = 0;
    _602.tirePrssureAbnormal = 0;
    _603.dashboardVisible = 0;
    _603.otherVisible = 0;
    
    switch (error_items[result].type)
    {
    case ET_CONTROL:
        _603.controlVisible = 1;
        break;
    case ET_POWERTRAIN:
        _603.powerTrainVisible = 1;
        break;
    case ET_SMART_KEY:
        _603.smartKeyVisible = 1;
        break;
    case ET_BATTERY:
        _603.batteryVisible = 1;
        break;
    case ET_TIRE_PRESSURE:
        _602.tirePrssureAbnormal = 1;
        break;
    case ET_DASH_BOARD:
        _603.dashboardVisible = 1;
        break;
    case ET_OTHERS:
        _603.otherVisible = 1;
        break;
    default:
        break;
    }
    //printf("===>%d,%d,%d,%d,%d,%d,%d\n", _603.controlVisible, _603.powerTrainVisible, _603.smartKeyVisible,
    //_603.batteryVisible, _602.tirePrssureAbnormal, _603.dashboardVisible, _603.otherVisible);
}

void setBikeWorkStatus(uint8_t status)
{
    _603.motoWorkingState = status;
}

void setBatTemp(uint8_t temp)
{
    _603.batteryTemperature = temp;
}

void setBatSoc(uint8_t soc)
{
    _603.batterySOC = soc;
}

void setRecharge(uint8_t recharge)
{
    _604.navigationRegenSetting = recharge;
}

void setSpdLmt(uint8_t spd)
{
    _606.speedLimited = spd;
}

void setCruCon(uint8_t spd)
{
    _606.navigationSpeed = spd;
}

void setRemainChargeTime(uint8_t hour, uint8_t minutes)
{
    _606.remainRegenHour = hour;
    _606.remainRegenMinutes = minutes;
}

void setSimVcuData(unsigned char id, OutputProtocol *simNucData)
{
    simNucData->len = 0x08;
    simNucData->ctrl = CTRL_CAN;
    simNucData->id[0] = 0x06;
    simNucData->id[1] = id;
    
    if (gSpdAuto && id == ID_SPD_ODO)
        SpdAuto();
    if (gTurnAuto && id == ID_BTN_LGT)
        TurnAuto();
    
    switch (id)
    {
    case ID_STARTUP_INIT:
        simNucData->data[0] = 0x00;
        simNucData->data[1] = _600.powerSignal;
        simNucData->data[2] = _600.clockHour;
        simNucData->data[3] = _600.clockMinutes;
        simNucData->data[4] = (_600.interfaceLanguage & 0xFF00) >> 8;
        simNucData->data[5] = (_600.interfaceLanguage & 0xFF);
        simNucData->data[6] = _600.counter;
        simNucData->data[7] = _600.speedUnit;
        break;
    case ID_SPD_ODO:
        simNucData->data[0] = 0x00;
        simNucData->data[1] = _601.speed;
        simNucData->data[2] = (_601.batteryCapability & 0xFF00) >> 8;
        simNucData->data[3] = (_601.batteryCapability & 0x00FF);
        simNucData->data[4] = 0x00;
        simNucData->data[5] = 0x00;
        simNucData->data[6] = 0x00;
        simNucData->data[7] = 0x00;
        break;
    case ID_BTN_LGT:
        simNucData->data[0] = 0x00;
        simNucData->data[1] = 0x00;
        simNucData->data[2] = _602.engineMode;
        simNucData->data[3] = _602.gearState;
        simNucData->data[4] = (_602.tirePrssureAbnormal << 7);
        simNucData->data[5] = _602.batterySignalVisible << 4 | _602.turnIndicatorVisible << 3 | _602.rightIndicatorVisible << 2 | _602.leftIndicatorVisible << 1;
        simNucData->data[6] = (_602.speedLimitVisible << 7) | (_602.navigationSpeedVisible << 6) | (_602.regenFlashEnable << 4);
        simNucData->data[7] = _602.switchLayer;
        break;
    case ID_VCU_INF:
        simNucData->data[0] = 0x00;
        simNucData->data[1] = 0x00;
        simNucData->data[2] = _603.motoWorkingState;
        simNucData->data[3] = (_603.smartKeyVisible << 7) | (_603.powerTrainVisible << 6) | (_603.dashboardVisible << 5) | (_603.controlVisible << 4) | (_603.batteryVisible << 3) | (_603.otherVisible << 2);
        simNucData->data[4] = (_603.errorCode & 0xFF00) >> 8;
        simNucData->data[5] = (_603.errorCode & 0x00FF);
        simNucData->data[6] = _603.batteryTemperature;
        simNucData->data[7] = _603.batterySOC;    
        break;
    case ID_TRIP:
        simNucData->data[0] = 0x00;
        simNucData->data[1] = 0x00;
        simNucData->data[2] = 0x00;
        simNucData->data[3] = 0x00;
        simNucData->data[4] = 0x00;
        simNucData->data[5] = 0x00;
        simNucData->data[6] = _604.navigationRegenSetting;
        simNucData->data[7] = 0x00;
        break;
    case ID_VCU_INF_2:
        simNucData->data[0] = _606.speedLimited;
        simNucData->data[1] = _606.navigationSpeed;
        simNucData->data[2] = 0x00;
        simNucData->data[3] = _606.remainRegenHour;
        simNucData->data[4] = _606.remainRegenMinutes;
        simNucData->data[5] = 0x00;
        simNucData->data[6] = 0x00;
        simNucData->data[7] = 0x00;
        break;
    case ID_DB_INF:
        simNucData->data[0] = 0x00;
        simNucData->data[1] = 0x00;
        simNucData->data[2] = 0x00;
        simNucData->data[3] = 0x00;
        simNucData->data[4] = 0x00;
        simNucData->data[5] = 0x00;
        simNucData->data[6] = 0x00;
        simNucData->data[7] = 0x00; 
        break;
    default:
        break;
    }
}

void setSimNucData(unsigned char id, OutputProtocol *simNucData)
{
    int len = 0;
    
    simNucData->ctrl = CTRL_CAN_NON;
    simNucData->id[0] = 0x1;
    simNucData->data[0] = 0x1;
    simNucData->data[1] = 1;
    simNucData->data[2] = 50;
    while (simNucData->data[len])
        len++;
    simNucData->len = len;
    printf("setSimNucData, len=%d", len);
}
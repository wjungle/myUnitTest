#define CAUTION_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif


typedef enum _ERROR_TYPE
{
    ET_CONTROL = 0x00,
    ET_POWERTRAIN,
    ET_SMART_KEY,
    ET_BATTERY,
    ET_TIRE_PRESSURE,
    ET_DASH_BOARD,
    ET_OTHERS,
    ET_MAX,
} ERROR_TYPE;

typedef struct _ErrorMessage
{
    ERROR_TYPE type;
    int id;
    char *msg;
} ErrorMessage;

ErrorMessage error_items[] = 
{
    {ET_CONTROL,    1000, ""},
    {ET_CONTROL,    1100, ""},
    {ET_CONTROL,    1200, ""},
    {ET_POWERTRAIN, 1300, ""},
    {ET_POWERTRAIN, 1301, ""},
    {ET_CONTROL,    1400, ""},
    {ET_CONTROL,    1600, ""},
    
    {ET_CONTROL, 1800, "Control Unit Error - Please seek service at an authorised dealer as soon as possible."},
    {ET_CONTROL, 1801, "ABS Error - Please seek service at an authorised dealer as soon as possible."},
    {ET_CONTROL, 1802, "Control Unit Error - Please stop riding immediately and contact an authorized dealer for safety. "},
    
    {ET_POWERTRAIN, 2000, ""},
    {ET_POWERTRAIN, 2200, ""},
    {ET_POWERTRAIN, 2300, ""},
    {ET_POWERTRAIN, 2400, ""},
    {ET_POWERTRAIN, 2401, ""},
    {ET_POWERTRAIN, 2402, ""},
    {ET_POWERTRAIN, 2403, ""},
    {ET_POWERTRAIN, 2500, ""},
    {ET_POWERTRAIN, 2501, ""},
    {ET_POWERTRAIN, 2502, ""},
    {ET_POWERTRAIN, 2600, "Power System Error - Please stop riding immediately and contact an authorized dealer for safety."},    
    {ET_SMART_KEY,  3000, ""},
    {ET_BATTERY,    4000, "Battery System Error - Please stop riding immediately and contact an authorized dealer for safety."},
    {ET_BATTERY,    4001, "Low Battery Warning - Re-charge your bike soon."},

    {ET_BATTERY, 4002, "Battery System Error - Please seek service at an authorised dealer as soon as possible."},
    {ET_BATTERY, 4003, "Battery System Error - Battery temperture too low."},
    {ET_BATTERY, 4004, "Battery System Error - Battery temperture too high."},
    {ET_BATTERY, 4005, "Battery System Error - Battery temperture too high, power system derating."},
    {ET_BATTERY, 4006, "Battery System Error - Battery temperature too low, charge the battery after it heats up."},
    {ET_BATTERY, 4007, "Battery System Error - Battery temperature too high, please stop riding immediately and contact an authorized dealer for service."},
    {ET_BATTERY, 4008, "Battery System Error - Battery temperature too high, charge the battery after it cools down."},

    {ET_BATTERY,    4400, ""},
    {ET_BATTERY,    4600, ""},
    {ET_BATTERY,    4700, ""},
    {ET_BATTERY,    5000, ""},    
    {ET_DASH_BOARD, 6000, "Dashboard System Error - Please seek service at an authorised dealer as soon as possible."},        
    {ET_OTHERS,     7300, ""},
    {ET_OTHERS,     7500, ""},
    {ET_OTHERS,     7600, ""},
    {ET_OTHERS,     7700, ""},
    {ET_TIRE_PRESSURE, 7900, ""},
    {ET_OTHERS,     9000, ""},
    
    {ET_OTHERS, 9001, "Communication System Error - Please seek service at an authorised dealer as soon as possible. "},
    {ET_OTHERS, 9002, "Maintenance mileage reached, please return to an authorised dealer for service. "},
    {ET_OTHERS, 9003, "USB Error - Please seek service at an authorised dealer as soon as possible."},
    {ET_OTHERS, 9004, "Blinker Error - Please seek service at an authorised dealer as soon as possible."},
    {ET_OTHERS, 9005, "Headlight Error - Please seek service at an authorised dealer as soon as possible."},
    {ET_OTHERS, 9006, "Taillight Error - Please seek service at an authorised dealer as soon as possible."},
    {ET_OTHERS, 9007, "Brake Light Error - Please seek service at an authorised dealer as soon as possible."},
    {ET_OTHERS, 9008, "Position Light Error - Please seek service at an authorised dealer as soon as possible."},
    {ET_OTHERS, 9009, "Tyre Pressure Error - Please seek service at an authorised dealer as soon as possible."},
    {ET_OTHERS, 9010, "Vehicle Fall Detected - Please return and check vehicle condition."},
    {ET_OTHERS, 9011, "Vehicle Displacement Detected - Please check vehicle condition as soon as possible."},
    {ET_OTHERS, 9012, "OBC Error - Please stop riding immediately and contact an authorized dealer for safety."},
    {ET_OTHERS, 9013, "DC-DC Error - Please stop riding immediately and contact an authorized dealer for safety."},
    {ET_OTHERS, 9014, "Headlight Error - Please seek service at an authorised dealer as soon as possible."},
    {ET_OTHERS, 9015, "Daytime Running Light Error - Please seek service at an authorised dealer as soon as possible."},
    {ET_OTHERS, 9016, "License Plate Light Error - Please seek service at an authorised dealer as soon as possible."},
    {ET_MAX, 0, NULL},
};


int binarySearch(ErrorMessage error_items[], int l, int r, int x)
{
    int mid;
    
    if (r >= l)
    {
        mid =  l + (r - l) / 2;
        
        if (error_items[mid].id == x)
            return mid;
        
        if (error_items[mid].id > x)
            return binarySearch(error_items, l, mid - 1, x);
        else    
            return binarySearch(error_items, mid + 1, r, x);
    }
    
    return -1;
}

#ifdef __cplusplus
}
#endif

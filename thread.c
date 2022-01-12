#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <Windows.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#include "rs232.h"
#include "canbus_data.h"
#include "common_layer_model.h"

//#define WRITE_BINARY
//#define DEBUG_PACKAGE

#define true 1
#define false 0
#define INPUT_SIZE 32

#define RECV_BUF_SIZE 64
#define SEND_BUF_SIZE 64

typedef enum _secondMenu {
    SPEED = 0x0,
    LAYER,
    TEMPERATURE,
    BATTERY,
    TIME,
    CRUISE,
    LIMIT,
    CAUTION,
} SecondMenu;

typedef enum _inputType {
    IT_STRING = 0x0,
    IT_ONE_NUMBER,
    IT_TWO_NUMBER,
} InputType;

typedef struct _User2uart {
    unsigned int com_port;
    unsigned char secondMenu;
    char secondFlag;
/*
    unsigned int spdValue;
    unsigned int modeValue;
    unsigned int layerValue;
    char tempValue;
    unsigned int batteryValue;
    unsigned int timeHValue;
    unsigned int timeMValue;
    unsigned int cruiseValue;
    unsigned int limitValue;
*/
    unsigned int Value;
    unsigned int Value2;
} User2uart;

typedef void (*UnitTestImplement)(User2uart*);
typedef struct _UnitTestImplStruct {
    unsigned int id;
    UnitTestImplement func;
} UnitTestImplStruct;

typedef struct _SendAttr {
    unsigned char repeat;
    unsigned int com_port;
} SendAttr;

FILE *fp;

sem_t bin_sem;
sem_t men_sem;
sem_t sen_sem;
sem_t isu_sem;
static unsigned char exit_send = false;
static unsigned char stop_send = false;
char user_input[INPUT_SIZE];
void *DataTask(void *arg);
void *MenuTask(void *arg);
void *SecondMenuTask(void *arg);
void printMenu(void);

void UT_speedAutoRun(User2uart* data);
void UT_speedUnit(User2uart* data);
void UT_UnitSecond(User2uart* data);
void UT_layerUnit(User2uart* data);
void UT_turnSignal(User2uart* data);
void UT_temperatureUnit(User2uart* data);
void UT_batteryLevelUnit(User2uart* data);
void UT_timeUnit(User2uart* data);
void UT_cruisingUnit(User2uart* data);
void UT_speedLimitUnit(User2uart* data);
void UT_cautionUnit(User2uart* data);

void UnitSecondTest(SecondMenu secMenu);

void delay(int milli_seconds)
{
    // Converting time into milli_seconds
    //int milli_seconds = 1000 * number_of_seconds;
    // Storing start time
    clock_t start_time = clock();

    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds) ;
}

unsigned char packOutoutData(unsigned char* outDataBuf, OutputProtocol *simNucData)
{

    int i = 0, j = 0;
    int cmdLen = 0;
    int dataLen = 8;
    unsigned char checkSum = 0;

    if (outDataBuf == NULL)
        return 0;

    outDataBuf[i++] = 0xAA;     // Header
    outDataBuf[i++] = 0x55;     // Header
    
    if (simNucData->ctrl == CTRL_CAN)
    {
        outDataBuf[i++] = dataLen;     // Body length
        outDataBuf[i++] = CTRL_CAN;    // IDE
        outDataBuf[i++] = simNucData->id[0];   
        outDataBuf[i++] = simNucData->id[1];       
        memcpy(&outDataBuf[i], &simNucData->data[0], dataLen);
            
        i += dataLen;
    } 
    else if (simNucData->ctrl == CTRL_CAN_NON)
    {
        outDataBuf[i++] = simNucData->len;     // Body length
        outDataBuf[i++] = simNucData->ctrl;
        outDataBuf[i++] = simNucData->id[0];
        memcpy(&outDataBuf[i], &simNucData->data[0], simNucData->len);
        
        i += simNucData->len;
    }
    
    // checksum
    for (j = 2; j < i; j++)
    {
        checkSum += outDataBuf[j];
    }
    checkSum %= 256;
    outDataBuf[i++] = checkSum;
    cmdLen = i;
 
#ifdef DEBUG_PACKAGE
    printf("\n");
    for (i = 0; i< cmdLen; i++)
        printf("%02X:", outDataBuf[i]);
    printf(":>cmdLen = %d\n", cmdLen);

#endif  

    return cmdLen;
}

static void resetStruct(OutputProtocol *simNucData)
{
    memset(simNucData, 0, sizeof(OutputProtocol));
}

static void UartSendImpl(int com_port, int id)
{
    int len;
    OutputProtocol simNucData = {0};
    unsigned char outDataBuf[SEND_BUF_SIZE] = {0};
    
    resetStruct(&simNucData);
    setSimVcuData(id, &simNucData);
    len = packOutoutData(outDataBuf, &simNucData);
#ifdef WRITE_BINARY
    fwrite(outDataBuf, len, sizeof(unsigned char), fp);
#else
    RS232_SendBuf(com_port, outDataBuf, len);
            
#endif 
}

void SendOne(unsigned int com_port)
{
    int id;
    
    for (id = 0 ; id <= ID_VCU_INF_2; id++)
        UartSendImpl(com_port, id);
}

void *SendTask(void *arg)
{
    SendAttr* attr = ((SendAttr*) arg);
    //printf("SendTask=%d\n", attr->com_port);
    static unsigned int _00_cnt = 0;
    static unsigned int _01_cnt = 0;
    static unsigned int _02_cnt = 0;
    static unsigned int _03_cnt = 0;
    static unsigned int _04_cnt = 0;
    //static unsigned int _05_cnt = 0;
    static unsigned int _06_cnt = 0;
    
    sem_wait(&sen_sem);
    while(!exit_send) {
        if (!stop_send) {
#if 0
            if (getSendRepeat()) {
                UartSendImpl(attr->com_port, 1);
                delay(PROTOCOL_INTERVAL);
            } else {
                UartSendImpl(attr->com_port, 1);
                stop_send = true;
            }
#else
            if (getSendRepeat()) {
                if (_00_cnt > PROTOCOL_INTERVAL)
                {
                    //printf("00_cnt: %d\n", _00_cnt);
                    UartSendImpl(attr->com_port, ID_STARTUP_INIT);
                    _00_cnt = 0;
                }
                
                if (_01_cnt > PROTOCOL_INTERVAL)
                {
                    //printf("01_cnt: %d\n", _01_cnt);
                    UartSendImpl(attr->com_port, ID_SPD_ODO);
                    _01_cnt = 0;
                }
                
                if (_02_cnt > PROTOCOL_INTERVAL)
                {
                    //printf("_02_cnt: %d\n", _02_cnt);
                    UartSendImpl(attr->com_port, ID_BTN_LGT);
                    _02_cnt = 0;
                }
         
                if (_03_cnt > PROTOCOL_INTERVAL)
                {
                    //printf("_03_cnt: %d\n", _03_cnt);
                    UartSendImpl(attr->com_port, ID_VCU_INF);
                    _03_cnt = 0;
                }
                
                if (_04_cnt > PROTOCOL_INTERVAL)
                {
                    //printf("_04_cnt: %d\n", _04_cnt);
                    UartSendImpl(attr->com_port, ID_TRIP);
                    _04_cnt = 0;
                }
                
                if (_06_cnt > PROTOCOL_INTERVAL)
                {
                    //printf("_06_cnt: %d=====>\n", _06_cnt);
                    UartSendImpl(attr->com_port, ID_VCU_INF_2);
                    _06_cnt = 0;
                }
                _00_cnt++;
                _01_cnt++;
                _02_cnt++;
                _03_cnt++;
                _04_cnt++;
                //_05_cnt++;
                _06_cnt++;    
                delay(1); // 1ms
            }
            else {
                SendOne(attr->com_port);
                stop_send = true;
            }
#endif
        }
    }
    pthread_exit(NULL);
    return NULL;
}

void printMenu(void)
{
    printf("==========================\n");
    printf("0. Exit\n");
    printf("1. speed auto run test\n");   
    printf("2. [unit test] speed\n");
    printf("3. [unit test] layer\n");
    printf("4. turn signal test\n");
    printf("5. [unit test] temperature\n");
    printf("6. [unit test] battery level\n");
    printf("7. [unit test] time\n");
    printf("8. [unit test] cruising speed\n");
    printf("9. [unit test] speed limit\n");
    printf("10.[unit test] error and caution\n");
    printf("77. Stop send uart\n");
    printf("==========================\n\n");
    printf("Enter your choice :  ");
}

/************************************/
static UnitTestImplStruct UnitTestTable[] = {
    {1, UT_speedAutoRun}, 
    {2, UT_speedUnit}, 
    {3, UT_layerUnit},
    {4, UT_turnSignal},
    {5, UT_temperatureUnit},
    {6, UT_batteryLevelUnit},
    {7, UT_timeUnit},
    {8, UT_cruisingUnit},
    {9, UT_speedLimitUnit},   
    {10, UT_cautionUnit},    
    //{21, UT_speedUnitSecond}, 
};

void jumpToUnitTest(unsigned char testNum, User2uart* data) {
    int i = 0;
    int UnitTestTableSize;
    
    UnitTestTableSize= sizeof(UnitTestTable) / sizeof(UnitTestTable[0]);
    for (i = 0; i < UnitTestTableSize; i++) {
        if (UnitTestTable[i].id == testNum) {
            // callback for finding implementaion
            UnitTestTable[i].func(data);
            break;
        }
    }
}

void UT_speedAutoRun(User2uart* data) {
    setPower(0xFF);
    setClock(12, 59);
    setSpdUnit(0x0A);   // Km/kph
    setSpdAuto();
    setEngineMode((uint8_t)0x42); // Confort
    setGearState(0x30);     // P
    setBattaryOn(1);
    setSpdLmtOn(1);
    setCruConOn(1);
    setPage(0x00);      // mainPage
    setBikeWorkStatus(0x32);    // riding
    setBatTemp(120);
    setBatSoc(88);
    setSpdLmt(40);
    setCruCon(40);    
    
    setSendRepeat(true);
    sem_post(&sen_sem);
    stop_send = false;
}

void UnitTestMethod(User2uart *data)
{
    if (!data)
        return;
    
    switch (data->secondMenu)
    {
    case SPEED:
        setSpd(data->Value);
        setEngineMode(data->Value2);
        break;
    case LAYER:
        setPage(data->Value);
        break;
    case TEMPERATURE:
        setBatTemp(data->Value);
        break;
    case BATTERY:
        setBatSoc(data->Value);
        break;
    case TIME:
        setClock(data->Value, data->Value2);
        break;
    case CRUISE:
        setCruCon(data->Value);
        break;
    case LIMIT:
        setSpdLmt(data->Value);
        break;
    case CAUTION:
        setErrorCode(data->Value);
        break;
    }
}

void UnitSecondTest(SecondMenu secMenu)
{
    User2uart data2 = {0};
    int res;
    pthread_t secm_thread;
    void *thread_result;
    
    data2.secondMenu = secMenu;
    res = pthread_create(&secm_thread, NULL, SecondMenuTask, &data2);
    if (res != 0) {
        perror("pthread_create failed");
        exit(EXIT_FAILURE);
    } 
    while (1) { 
        sem_wait(&isu_sem);
        
        if (data2.secondFlag == 'q')
            break;
 
        setPower(0xFF);
        //setEngineMode(0x41);
        setSpd(0);
        setSpdLmtOn(1);
        setCruConOn(1);
        setBattaryOn(1);

        UnitTestMethod(&data2);
        
        setSendRepeat(false);
        sem_post(&sen_sem);
        stop_send = false;
    }
    
    res = pthread_join(secm_thread, &thread_result);
    if (res != 0) {
        perror("pthread_join failed");
        exit(EXIT_FAILURE);
    } 
}

void UT_speedUnit(User2uart* data) {
    UnitSecondTest(SPEED);
}

void UT_layerUnit(User2uart* data) {
    UnitSecondTest(LAYER);
}

void UT_turnSignal(User2uart* data) {
  
    setTurnAuto();
    
    setSendRepeat(true);
    sem_post(&sen_sem);
    stop_send = false;
}

void UT_temperatureUnit(User2uart* data) {
    UnitSecondTest(TEMPERATURE);
}

void UT_batteryLevelUnit(User2uart* data) {
    UnitSecondTest(BATTERY);
}

void UT_timeUnit(User2uart* data) {
    UnitSecondTest(TIME);
}

void UT_cruisingUnit(User2uart* data) {
    UnitSecondTest(CRUISE);
}

void UT_speedLimitUnit(User2uart* data) {
    UnitSecondTest(LIMIT);
}

void UT_cautionUnit(User2uart* data) {
    UnitSecondTest(CAUTION);
}

/************************************/

int UartInit(int com_port)
{
#ifndef WRITE_BINARY      
    int baudrate = 115200;
    char mode[]  = {'8', 'N', '1', 0};    
   
#ifdef _WIN32
    /* Open COM Port */
    if (RS232_OpenComport(com_port, baudrate, mode, 0)) {
        return 1;
    }
    printf("Open COM Port Success\n\n");
#endif
#else
    fp = fopen("unittest.bin", "wb+");
    
    if (fp == NULL)
        printf("open unit test binary fail\n");  
#endif
    return 0;
}

void HudExit(int com_port)
{
    /* Close COM Port */
    RS232_CloseComport(com_port);
    printf("Close COM Port\n");    
}

int main(int argc, char *argv[])
{
    int res;
    //pthread_t data_thread;
    pthread_t send_thread;
    //void *thread_result;
    SendAttr sendAttr;
    unsigned char select;
    User2uart userInfo;
    int goon = 1;
    
#ifndef WRITE_BINARY      
    if (argv[1] == NULL) {
        printf("please input COM#\n");
        printf("usage: ./myuart.exe COM#\n");
    }
    else {
        sendAttr.com_port = RS232_GetPortnr(argv[1]);
    }
  
    res = UartInit(sendAttr.com_port);
    if (res)
        return(0);
#endif  
    
    res = sem_init(&bin_sem, 0, 0);
    if (res != 0) {
        perror("Semaphore initialization failed");
        exit(EXIT_FAILURE);
    }
    res = sem_init(&men_sem, 0, 0);
    if (res != 0) {
        perror("Semaphore initialization failed");
        exit(EXIT_FAILURE);
    }
    res = sem_init(&sen_sem, 0, 0);
    if (res != 0) {
        perror("sen_init failed");
        exit(EXIT_FAILURE);
    }    
    res = sem_init(&isu_sem, 0, 0);
    if (res != 0) {
        perror("sem_init failed");
        exit(EXIT_FAILURE);
    }      
    
    res = pthread_create(&send_thread, NULL, SendTask, &sendAttr);
    if (res != 0) {
        perror("pthread_create failed");
        exit(EXIT_FAILURE);
    }   

    while (goon) {
        printf("Input number. Enter '0' to exit\n");
        printMenu();
        fflush(stdin);
        fgets(user_input, INPUT_SIZE, stdin);

        select = atoi(user_input);

        switch (select)
        {
        case 0:
            goon = 0;
            break;
        case 77:
            stop_send = true;
            //RS232_flushTX(sendAttr.com_port);
            //printf("RS232_flushTX:%d\n", sendAttr.com_port);
            break;
        default:
            jumpToUnitTest(select, &userInfo);
            break;
        }
    } ;
    printf("\nWaiting for thread to finish...\n");
    printf("Thread joined\n");
    sem_destroy(&bin_sem);
    exit_send = true;
    fclose(fp);
#ifndef WRITE_BINARY     
    HudExit(sendAttr.com_port);
#endif
    exit(EXIT_SUCCESS);
}

void *SecondMenuTask(void *arg) {
    
    UT_UnitSecond((User2uart*) arg);
    
    pthread_exit(NULL);
    return NULL;
}

void splitStr(char* str, int *val1, int *val2)
{
    char *delim = ",";
    char *pch;
    int cnt = 0;
 
    //printf("==>%s, and split result is \n", str);
    pch = strtok(str, delim);
    while (pch != NULL)
    {
        //printf ("%s\n", pch);
        if (cnt == 0)
            *val1 = atoi(pch);
        else if (cnt == 1)
            *val2 = atoi(pch);
        pch = strtok (NULL, delim);
        cnt++;
    }
}

unsigned char isNumber(const char* str, int *val1, int *val2)
{
    InputType bool_number = IT_ONE_NUMBER;
    //unsigned char bool_negative = 0;
    char* ori_str = (char *)str;
    
    while (*str != 0x0A)
    {
        if (*str == '-') {
            //bool_negative = 1;
        } else if (*str == ',') {
            splitStr(ori_str, val1, val2);
            bool_number = IT_TWO_NUMBER;
            break;
        } else if (isdigit(*str)) {
            
        } else {
            bool_number = IT_STRING;
            break;
        }
        str++;
    }
    return bool_number;
}

void UT_UnitSecond(User2uart* data)
{
    int inSecond = 1;
    InputType res;
    int value1, value2;
    
    memset(user_input, 0, sizeof(user_input));
    while (inSecond) {
        printf("\n\n\t===========================\n");
        printf("\t q <<< return previous menu \n");
        if (data->secondMenu == SPEED) {
            printf("\t set the value of speed(#), engine mode(65,66,67) \n");
            printf("\t===========================\n\n");
            printf("\tEnter speed / engine mode:  ");
        } else if (data->secondMenu == LAYER) {
            printf("\t set the value of layer(0,16,32,48,64,112) \n");
            printf("\t===========================\n\n");
            printf("\tEnter layer values :  ");  
        } else if (data->secondMenu == TEMPERATURE) {
            printf("\t set the value of temperature(-128~127)except -1 \n");
            printf("\t===========================\n\n");
            printf("\tEnter temperature values :  ");
        } else if (data->secondMenu == BATTERY) {
            printf("\t set the value of battery level(0~100) \n");
            printf("\t===========================\n\n");
            printf("\tEnter battery level values :  ");
        } else if (data->secondMenu == TIME) {
            printf("\t set the value of time(hh:mm) \n");
            printf("\t===========================\n\n");
            printf("\tEnter time (hh,mm) values :  ");
        } else if (data->secondMenu == CRUISE) {
            printf("\t set the value of cruising speed (0~160) \n");
            printf("\t===========================\n\n");
            printf("\tEnter cruising speed values :  ");
        } else if (data->secondMenu == LIMIT) {
            printf("\t set the value of speed limit(0~160) \n");
            printf("\t===========================\n\n");
            printf("\tEnter speed limit values :  ");
        } else if (data->secondMenu == CAUTION) {
            printf("\t set the value of error code(E-####) \n");
            printf("\t===========================\n\n");
            printf("\tEnter error code values :  ");
        } else {
            printf("\tERROR!!!\n\n");
        }
        
        fflush(stdin);
        fgets(user_input, INPUT_SIZE, stdin);
        
        res = isNumber(user_input, &value1, &value2);
        if (res == IT_ONE_NUMBER) {
            data->Value = atoi(user_input);
            printf("\t(%d) NUMBER is %d\n", res, data->Value);
            //data->secondFlag = 1;
        } else if (res == IT_TWO_NUMBER) {
            data->Value = value1;
            data->Value2 = value2;
            printf("\t(%d) NUMBER is %d, %d\n",res, value1, value2);
        } else {
            printf("\t(%d) String is %s\n", res, user_input);
            data->secondFlag = 'q';
        }
        sem_post(&isu_sem);
        
        if (data->secondFlag == 'q')
            break;
    } ;
}

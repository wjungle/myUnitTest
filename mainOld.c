#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#include "rs232.h"
#include "canbus_data.h"
#include "common_layer_model.h"

//#define WRITE_BINARY
#define DEBUG_PACKAGE
//#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) /* do nothing */
#endif
#define REPEAT_TIMES    3

#define true 1
#define false 0

/* https://www.teuniz.net/RS-232/ */
#ifdef _WIN32
#define WIN32_COM   5//2//5   /* COM7 = ttyS6 */
#endif

#define RS232_ERROR   1
#define RS232_SUCCESS 0

#define RECV_BUF_SIZE 64
#define SEND_BUF_SIZE 64

#define PROTOCOL_INTERVAL   100 //ms

#define UNIT_LEN    3

#define SPD_MAX 160
#define SPD_MIN 0

static pthread_t a_thread;
static pthread_t data_thread;
static pthread_t send_thread;
static volatile int extQuit;

FILE *fp;
int gTest = false;
int gStart = false;
int gSpd = false;
int gSpdu = false;
int gModeu = false;
int gLayeru = false;
int gTrans = false;
int gTurn = false;
int gNCan = false;
int gCharge = false;
int gBatt = false;
int gTherm = false;
static volatile int gFire = false;

int spdValue;
int modeValue;
int layerValue;

sem_t bin_sem;
sem_t bin_sem2;
sem_t ver_sem;


void delay(int milli_seconds)
{

    // Converting time into milli_seconds

    //int milli_seconds = 1000 * number_of_seconds;

    // Storing start time

    clock_t start_time = clock();

    // looping till required time is not achieved

    while (clock() < start_time + milli_seconds) ;
}

#if 0
unsigned char calcChecksum(OutputProtocol *TpData, int len)
{
    int i;
    unsigned char checkSum = 0;
    
    return checkSum;
}
#endif

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
    //sem_post(&bin_sem2);    

    return cmdLen;
}

static void resetStruct(OutputProtocol *simNucData)
{
    memset(simNucData, 0, sizeof(OutputProtocol));
}

static void UartSendImpl(int id)
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
    RS232_SendBuf(WIN32_COM, outDataBuf, len);
            
#endif 
}

static void* DataTask(void* arg)
{
}

#define INTERVAL 100
static void* UartSendTask(void* arg)
{
    static unsigned int _00_cnt = 0;
    static unsigned int _01_cnt = 0;
    static unsigned int _02_cnt = 0;
    static unsigned int _03_cnt = 0;
    static unsigned int _04_cnt = 0;
    //static unsigned int _05_cnt = 0;
    static unsigned int _06_cnt = 0;
    
    sem_wait(&bin_sem);
    while (1)//
    {   
#if 1
        
        if (_00_cnt > INTERVAL)
        {
            UartSendImpl(ID_STARTUP_INIT);
            //printf("00_cnt: %d\n", _00_cnt);
            _00_cnt = 0;
        }
        
        if (_01_cnt > INTERVAL)
        {
            //printf("01_cnt: %d\n", _01_cnt);
            UartSendImpl(ID_SPD_ODO);
            _01_cnt = 0;
        }
        
        if (_02_cnt > INTERVAL)
        {
            //printf("_02_cnt: %d\n", _02_cnt);
            UartSendImpl(ID_BTN_LGT);
            _02_cnt = 0;
        }
 
        if (_03_cnt > INTERVAL)
        {
            //printf("_03_cnt: %d\n", _03_cnt);
            UartSendImpl(ID_VCU_INF);
            _03_cnt = 0;
        }
        
        if (_04_cnt > INTERVAL)
        {
            //printf("_04_cnt: %d\n", _04_cnt);
            UartSendImpl(ID_TRIP);
            _04_cnt = 0;
        }
        
        if (_06_cnt > INTERVAL)
        {
            //printf("_06_cnt: %d\n", _06_cnt);
            UartSendImpl(ID_VCU_INF_2);
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
#endif
    }
}

static void* ExternalTask(void* arg)
{
    while (!extQuit)
    {
        int id, i, j;
        int len;
        //int page = 0x00;
        int turnStatus = 0;
        int turnCnt = 0;
        int turnOnOff = 1;
        //unsigned char writeLen = 0;
        unsigned char outDataBuf[SEND_BUF_SIZE] = {0};
        OutputProtocol simNucData = {0};

        InitialCommonLayerModel();
        
        if (gStart)
        {
            setPower(0xFF);     //demo: 0x55
            setClock(11, 50);
            setSpdUnit(0x0B);   // Km(0x0A)/kph(0x10)
            setSpd(36);
            setEngineMode(0x43); // 0x41; Confort(0x42) ; sport:0x43
            setGearState(0x30);     // P
            setBattaryOn(1);
            setSpdLmtOn(1);
            setCruConOn(1);
            setPage(0x00);      // mainPage
            setBikeWorkStatus(0x32);    // riding
            setBatTemp(-128);
            setBatSoc(18);
            setRecharge(1);
            setSpdLmt(50);
            setCruCon(40);

            for (i = 0; i < REPEAT_TIMES; i++)
                for (id = ID_STARTUP_INIT; id <= ID_VCU_INF_2; id++)
                {
                    resetStruct(&simNucData);
                    setSimVcuData(id, &simNucData);
                    len = packOutoutData(outDataBuf, &simNucData);
#ifdef WRITE_BINARY
                    /*writeLen = */fwrite(outDataBuf, len, sizeof(unsigned char), fp);
#else
                    RS232_SendBuf(WIN32_COM, outDataBuf, len);
                    Sleep(PROTOCOL_INTERVAL);
#endif
                }

            gStart = false;
        }
        else if (gSpd)
        {
            int toggle = 1;
            for (j = SPD_MIN; j <= SPD_MAX;) {
                setPower(0xFF);
                setClock(12, 59);
                setSpdUnit(0x0A);   // Km/kph
                
                setSpd(j);
                
                setEngineMode(0x42); // Confort
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
#if 1
                for (id = ID_STARTUP_INIT; id <= ID_TRIP; id++)
                {
                    resetStruct(&simNucData);
                    setSimVcuData(id, &simNucData);
                    len = packOutoutData(outDataBuf, &simNucData);
#ifdef WRITE_BINARY
                    fwrite(outDataBuf, len, sizeof(unsigned char), fp);
#else
                    RS232_SendBuf(WIN32_COM, outDataBuf, len);
                    //Sleep(50);//PROTOCOL_INTERVAL
                    delay(10);//10, 15, 20ms is OK
#endif 
                }
#else
                gFire = true;
                //printf("gFire = %d\n", gFire);
                delay(10);
                
#endif
                if (j == SPD_MAX || j == SPD_MIN)
                    toggle ^= 1;
                if (toggle == 0)
                    j++;
                else
                    j--;

            }
            gSpd = false;
        }
        
        else if (gSpdu)
        {
            setPower(0xFF);
            setEngineMode(modeValue); // eco
            setSpd(spdValue);
            setSpdLmtOn(1);
            setCruConOn(1);
            
            for (id = ID_STARTUP_INIT; id <= ID_BTN_LGT; id++)
            {
                resetStruct(&simNucData);
                setSimVcuData(id, &simNucData);
                //setSimVcuData(ID_SPD_ODO, &simNucData); //TODO
                len = packOutoutData(outDataBuf, &simNucData);
#ifdef WRITE_BINARY
                fwrite(outDataBuf, len, sizeof(unsigned char), fp);
#else
                RS232_SendBuf(WIN32_COM, outDataBuf, len);
                Sleep(PROTOCOL_INTERVAL);
#endif 
            }

            gSpdu = false;
        }
        
        else if (gModeu)
        {
            setPower(0xFF);
            setEngineMode(modeValue);
            setSpd(39);
            setSpdLmtOn(1);
            setCruConOn(1);
            
            for (id = ID_STARTUP_INIT; id <= ID_BTN_LGT; id++)
            {
                resetStruct(&simNucData);
                setSimVcuData(id, &simNucData);
                len = packOutoutData(outDataBuf, &simNucData);
#ifdef WRITE_BINARY
                fwrite(outDataBuf, len, sizeof(unsigned char), fp);
#else
                RS232_SendBuf(WIN32_COM, outDataBuf, len);
                Sleep(PROTOCOL_INTERVAL);
#endif 
            }

            gModeu = false;
        }

        else if (gLayeru)
        {
            setPower(0xFF);
            setEngineMode(0x41);
            //if (layerValue == 0x00)
            setSpd(0);
            setSpdLmtOn(1);
            setCruConOn(1);
            setPage(layerValue);
            
            for (i = 0; i < 1; i++)
            for (id = ID_STARTUP_INIT; id <= ID_BTN_LGT; id++)
            {
                resetStruct(&simNucData);
                setSimVcuData(id, &simNucData);
                len = packOutoutData(outDataBuf, &simNucData);
#ifdef WRITE_BINARY
                fwrite(outDataBuf, len, sizeof(unsigned char), fp);
#else
                RS232_SendBuf(WIN32_COM, outDataBuf, len);
                Sleep(PROTOCOL_INTERVAL);
#endif 
            }

            gLayeru = false;
        }
        
        else if (gTrans)
        {
            //while (1)
            {
                setPower(0xFF);
                setClock(12, 59);
                setSpdUnit(0x0A);   // Km/kph
                setSpd(66);
                setEngineMode(0x43); // Confort:0x42; sport:0x43
                setGearState(0x30);     // P
                setBattaryOn(1);
                setSpdLmtOn(1);
                setCruConOn(1);
            
            #if 1
                setPage(0x40);  
            #else
                if (j % 20 == 0)
                    page = (((page >> 4) & 0x0F) + 1) << 4;
                printf("page = %X\n", page);
                setPage(page);      // mainPage
                if (page > 0x40)
                    break;
            #endif
            
                setBikeWorkStatus(0x32);    // riding
                setBatTemp(120);
                setBatSoc(88);
                setSpdLmt(40);
                setCruCon(40);
                j++;
                
                resetStruct(&simNucData);
                setSimVcuData(ID_BTN_LGT, &simNucData); //TODO
                len = packOutoutData(outDataBuf, &simNucData);
#ifdef WRITE_BINARY
                fwrite(outDataBuf, len, sizeof(unsigned char), fp);
#else
                RS232_SendBuf(WIN32_COM, outDataBuf, len);
                Sleep(PROTOCOL_INTERVAL);
#endif 
            }
            gTrans = false;
        }
        else if (gTurn)
        {
            int times = 2;
            int delayTime = 190;
            while (1)
            {
                if (turnStatus == 0)
                {
                    setTurnSignalOn(turnOnOff);
                    setTurnRightOn(0);
                    setTurnLeftOn(1);
                    printf("Turn Left(%d)\n", turnOnOff);
                }
                else if (turnStatus == 1)
                {
                    setTurnSignalOn(turnOnOff);
                    setTurnRightOn(1);
                    setTurnLeftOn(0);  
                    printf("Turn Right(%d)\n", turnOnOff);
                }
                else if (turnStatus == 2)
                {
                    setTurnSignalOn(turnOnOff);
                    setTurnRightOn(1);
                    setTurnLeftOn(1);   
                    printf("Turn L&R(%d)\n", turnOnOff);
                }
                turnCnt++;
                if (turnCnt%times == 0)
                    turnOnOff^=1;
                if (turnCnt%(times*6) == 0)
                    turnStatus++;
                turnStatus%=3;
                
                resetStruct(&simNucData);
                setSimVcuData(ID_BTN_LGT, &simNucData);
                len = packOutoutData(outDataBuf, &simNucData);

#ifdef WRITE_BINARY
                /*writeLen = */fwrite(outDataBuf, len, sizeof(unsigned char), fp);
#else
                RS232_SendBuf(WIN32_COM, outDataBuf, len);
                Sleep(delayTime);
#endif
            }
            gTurn = false;
        }
        
        if (gNCan)
        {
            resetStruct(&simNucData);
            setSimNucData(id, &simNucData); //start-up
            len = packOutoutData(outDataBuf, &simNucData);
#ifdef WRITE_BINARY
            /*writeLen = */fwrite(outDataBuf, len, sizeof(unsigned char), fp);
#else
            RS232_SendBuf(WIN32_COM, outDataBuf, len);
            Sleep(PROTOCOL_INTERVAL);
#endif

            gNCan = false;
        }

        if (gCharge)
        {
            //setPower(0x55);     //demo: 0x55
            //setEngineMode(0x42); // Confort
            //setGearState(0x30);     // P
            //setBattaryOn(1);
            //setPage(0x00);      // mainPage
            setTirePressure(false, 32, 36);
            setRemainRange(220);
            setBikeWorkStatus(0x1E);    // charging
            setBatTemp(25);
            setBatSoc(66);
            setChargingChanging(true);
            //setRemainChargeTime(1, 35);
            setRemainChargeTime(0, 0);

            for (id = ID_STARTUP_INIT; id <= ID_VCU_INF_2; id++)
            {
                resetStruct(&simNucData);
                setSimVcuData(id, &simNucData);
                len = packOutoutData(outDataBuf, &simNucData);
#ifdef WRITE_BINARY
                /*writeLen = */fwrite(outDataBuf, len, sizeof(unsigned char), fp);
#else
                RS232_SendBuf(WIN32_COM, outDataBuf, len);
                Sleep(PROTOCOL_INTERVAL);
#endif
                }

            gCharge = false;
        }

        if (gTest)
        {
            //pthread_create(&send_thread, NULL, UartSendTask, NULL);
            
            int toggle = 1;
            
            setPower(0xFF);
            setClock(12, 59);
            setSpdUnit(0x0A);   // Km/kph
            
            setSpd(j);
            
            setEngineMode(0x42); // Confort
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
            
            for (j = SPD_MIN; j <= SPD_MAX;) {
                if (j == SPD_MAX || j == SPD_MIN)
                    toggle ^= 1;
                if (toggle == 0)
                    j++;
                else
                    j--;
                
                //printf("spd= %d\n", j);
                setSpd(j);
                //UartSendImpl(ID_SPD_ODO);
                //delay(100);
                //gFire = true;
                sem_post(&bin_sem);
            }
            gTest = false;
        }

#if 0
        readLen = RS232_PollComport(WIN32_COM, inDataBuf, RECV_BUF_SIZE);
        if (readLen > 0) {
            //printf("readLen = %d\n", readLen);
            //fflush(stdout);
            //errorCorrection(readLen);
        }
#endif
        Sleep(PROTOCOL_INTERVAL);
    }
    
}

int HudInit(void)
{
#ifndef WRITE_BINARY      
    int baudrate = 115200;
    char mode[]  = {'8', 'N', '1', 0};    
   
#ifdef _WIN32
    /* Open COM Port */
    if (RS232_OpenComport(WIN32_COM, baudrate, mode)) {
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

void HudExit(void)
{
    extQuit = true;
    pthread_join(a_thread, NULL);
    pthread_join(send_thread, NULL);
    
    /* Close COM Port */
    RS232_CloseComport(WIN32_COM);
    printf("\nClose COM Port\n");    
}


void SpeedMenu(int typeData)
{
    int inSecond = 1;
    
    while(inSecond) {
        if (typeData == 6) {
            printf("\n\n\t===========================\n");
            printf("\t-1: <<< return previous menu \n");
            printf("\t set the value of speed, engine mode(65,66,67) \n");
            printf("\t===========================\n\n");
            printf("\tEnter speed / engine mode:  ");
            scanf("%d, %d", &spdValue, &modeValue);
            printf("\tyou input (speed/mode) value: (%d, %d)\n", spdValue, modeValue);
            gSpdu = true;
        } else if (typeData == 7) {
            printf("\n\n\t===========================\n");
            printf("\t-1: <<< return previous menu \n");
            printf("\t set the value of layer(0,16,32,48,64) \n");
            printf("\t===========================\n\n");
            printf("\tEnter layer values :  ");
            scanf("%d",&layerValue);
            printf("\tyou input layer value: (%d)\n", layerValue);
            gLayeru = true;
        } else {
            printf("error input\n");
        }
        printf("\n");

        if (spdValue == -1 || layerValue == -1)
            inSecond = 0;
        //else {
        //    gSpdu = true;
        //}
    }
}

int main()
{
    int choice;
    int ret;
    
    ret = HudInit();
    if (ret)
        return(0);
    
    pthread_create(&a_thread, NULL, ExternalTask, NULL);
    pthread_create(&data_thread, NULL, DataTask, NULL);
    pthread_create(&send_thread, NULL, UartSendTask, NULL);
    
    sem_init(&bin_sem, 0, 0);
    //sem_init(&bin_sem2, 0, 0);
    //sem_init(&ver_sem, 0, 0);
    //sem_wait(&ver_sem);
    //printf("[HUD simulator] HUD Protocol version: %d\n", gVersion);  

    while(1) {
        printf("\n===================\n");
        printf("1. start up test\n");   
        printf("2. speed auto run test\n");
        printf("3. transfer test\n");
        printf("4. turn signal test\n");
        printf("5. internal message\n");
        printf("6. speed unit test\n");
        printf("7. layer switch unit test\n");
        printf("8. charging state test\n");
        printf("9. test test test\n");
        //printf("9. thermal auto run test\n");
        printf("0. Exit\n");
        printf("===================\n\n");
        printf("Enter your choice :  ");
        scanf("%d",&choice);
        
        switch (choice)
        {
            case 1:
                printf("your select is ==> %s\n", "start up test");
                gStart = true;
                break;
            case 2:
                printf("your select is ==> %s\n", "speed auto run test");
                gSpd = true;
                break;
            case 3:
                printf("your select is ==> %s\n", "transfer test");
                gTrans = true;
                break; 
            case 4:
                printf("your select is ==> %s\n", "turn signal test");
                gTurn = true;
                break; 
            case 5:
                printf("your select is ==> %s\n", "internal non-CAN test");
                gNCan = true;
                break; 
            case 6:
                printf("your select is ==> %s\n", "speed unit test");
                gSpdu = true;
                SpeedMenu(choice);
                break; 
            case 7:
                printf("your select is ==> %s\n", "layer switch unit test");
                //gLayeru = true;
                SpeedMenu(choice);
                break; 
            case 8:
                printf("your select is ==> %s\n", "charging state test");
                gCharge = true;
                break; 
            case 9:
                printf("your select is ==> %s\n", "test test test");
                gTest = true;
                break; 
            case 0:
                fclose(fp);
                exit(0);
                
        }
    }

    return(0);
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <pthread.h>

#include "rs232.h"

//#define DEBUG_PACKAGE
//#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) /* do nothing */
#endif

#define true 1
#define false 0

#ifdef _WIN32
#define WIN32_COM   5   /* COM6 = ttyS5 */
#endif

#define RS232_ERROR   1
#define RS232_SUCCESS 0

#define RECV_BUF_SIZE 64
#define SEND_BUF_SIZE 64

#define DLE                 0x9F    // Data Link Escape
#define STX                 0x02    // Start of text
#define ETX                 0x03    // End of text

#define FACTORY_MODE_CMD            "ASUS_PRODUCT_TEST"
#define ADJUSTMENT_MODE_CMD         "ASUS_PRODUCT_ADJUSTMENT"

typedef enum GET_UART_COMMAND_STATE_TAG
{
    GET_DLE,
    GET_STX,
    GET_TP,
    GET_DATA,
    GET_CHECKSUM,
    GET_ETX,
} GET_UART_COMMAND_STATE;

typedef enum {
    TP_VERSION_INFO = 0x01,
    TP_REQ_MSG_TRANS = 0x20,
    TP_RET_KEY_PRESS = 0xa0,
    TP_RET_ADC_LIGHT = 0xa2,
    TP_RET_ADC_THERMAL = 0xa3,
} HUD_2_VEH_TP;

typedef enum {
    ASUS_PRODUCION = 1,
    ASUS_ADJUSTMENT,
} ASUS_PROD_EN;

typedef enum {
    TP_PRODUCT_TEST = 0x21,
    TP_CHANGE_SCREEN = 0xa0, // and request
    TP_ADJUST_BACKLIGHT= 0xa1,
} SIM_2_HUD_TP;

typedef struct {
    unsigned char tp;
    unsigned char fixedLen;
    unsigned char msgLen;
} Hud2VehAttr;

/* errorCorrection -> parseTpData */
typedef struct {
    unsigned char tp;
    unsigned char d[4];
    unsigned char msgLen;
} Hud2VehCmd;

typedef union {
    unsigned char d[4];
    char string[32];
} S2uData;

typedef struct {
    unsigned char tp;
    S2uData data;
} OutputProtocol;

static GET_UART_COMMAND_STATE gState = GET_DLE;
static unsigned char gParsing = 0;
static unsigned char gTp;
static unsigned char gPlCnt = 0;
static unsigned int dataPos = 0;
static unsigned char inDataBuf[RECV_BUF_SIZE];
static unsigned char cmdBuf[RECV_BUF_SIZE];
static unsigned int cmdPos = 0;
static Hud2VehCmd h2vCmd;

static pthread_t a_thread;
static volatile int extQuit;

int gVersion = 0;
int gTest = false;
int gCommand = false;
int gChgScn = false;
int gChgAdc = false;
int adcVal;

int gKey = 1;

static int isValidTp(unsigned char tp)
{
if (tp == TP_VERSION_INFO || tp == TP_REQ_MSG_TRANS || tp == TP_RET_KEY_PRESS ||
    tp == TP_RET_ADC_LIGHT || tp == TP_RET_ADC_THERMAL)
        return 1;
    else
        return 0;
}

static int getDataLen(unsigned char tp)
{
    if (tp == TP_RET_ADC_LIGHT)
        return 3;
    else if (tp == TP_RET_ADC_THERMAL)
        return 2;
    else
        return 1;
}

static void parseTpData(Hud2VehCmd *cmd)
{
    //printf("TP=%02X\n", cmd->tp);
    switch(cmd->tp) {
        case TP_VERSION_INFO:
            gVersion = cmd->d[0];
            break;
        case TP_REQ_MSG_TRANS:
            break;
        case TP_RET_KEY_PRESS:
            if (cmd->d[0] == 0x0)
                printf(">>>>>> HUD Return OK");
            gKey = 0;
            break;
        case TP_RET_ADC_LIGHT:
            printf(">>>>>> ADC LIGHT is: %x-%x-%x ", cmd->d[0], cmd->d[1], cmd->d[2]);
            gKey = 0;
            break;
        case TP_RET_ADC_THERMAL:
            printf(">>>>>> ADC THERMAL is: %x-%x ", cmd->d[0], cmd->d[1]);
            gKey = 0;
            break;
    }
}

static void errorCorrection(unsigned char readLen)
{
    int i;
    unsigned int dataLen = 0;
    unsigned int count = 0;
    unsigned int checkSum = 0;
    
    if (readLen)
    {
        while (readLen--)
        {
            switch (gState)
            {
            case GET_DLE:
                if (DLE == inDataBuf[count])
                {
                    DEBUG_PRINT("[GET_DLE] Right, getchar=0x%02x\n", inDataBuf[count]);
                    cmdBuf[cmdPos++] = inDataBuf[count];
                    if (!gParsing) {
                        gParsing = 1;
                        gState = GET_STX;
                    } else
                        gState = GET_ETX;
                }
                else
                {
                    DEBUG_PRINT("[GET_DLE] Wrong, getchar=0x%02x\n", inDataBuf[count]);
                    cmdPos = 0;
                    memset(cmdBuf, 0, RECV_BUF_SIZE);
                    gState = GET_DLE;
                }
                break;
                
            case GET_STX:
                if (STX == inDataBuf[count])
                {
                    DEBUG_PRINT("[GET_STX] Right, getchar=0x%02x\n", inDataBuf[count]);
                    cmdBuf[cmdPos++] = inDataBuf[count];
                    gState = GET_TP;
                }
                else
                {
                    DEBUG_PRINT("[GET_STX] Wrong, getchar=0x%02x\n", inDataBuf[count]);
                    cmdPos = 0;
                    memset(cmdBuf, 0, RECV_BUF_SIZE);
                    gState = GET_DLE;
                }
                break;
            
            case GET_TP:
                if (isValidTp(inDataBuf[count]))
                {
                    gTp = inDataBuf[count];
                    DEBUG_PRINT("[GET_TP] Right, getchar=0x%02x\n", inDataBuf[count]);
                    cmdBuf[cmdPos++] = inDataBuf[count];
                    /* self-define */
                    h2vCmd.tp = gTp;
                    gState = GET_DATA;
                }
                else
                {
                    DEBUG_PRINT("[GET_TP] Wrong, getchar=0x%02x\n", inDataBuf[count]);
                    cmdPos = 0;
                    memset(cmdBuf, 0, RECV_BUF_SIZE);
                    /* self-define */
                    memset(&h2vCmd, 0, sizeof(h2vCmd));
                    gState = GET_DLE;
                }
                break;
                
            case GET_DATA:
                dataLen = getDataLen(gTp);
                if (gPlCnt != dataLen - 1)
                {
                    gState = GET_DATA;
                    gPlCnt++;
                } 
                else {
                    gState = GET_CHECKSUM;
                    gPlCnt = 0;
                }
                cmdBuf[cmdPos++] = inDataBuf[count];
                h2vCmd.d[dataPos++] = inDataBuf[count];
                h2vCmd.msgLen = dataLen;
                DEBUG_PRINT("[GET_DATA] getchar=0x%x\n", inDataBuf[count]);
                break;
                
            case GET_CHECKSUM:
                //TODO
                checkSum = h2vCmd.tp;
                for (i = 0; i < h2vCmd.msgLen; i++)
                    checkSum ^= h2vCmd.d[i];
                
                //DEBUG_PRINT("inDataBuf[count] = %x\n", inDataBuf[count]);
                if (checkSum == inDataBuf[count])
                {
                    DEBUG_PRINT("[GET_CHECKSUM] Right, checkSum=0x%x\n", checkSum);
                    // Get one command
                    cmdBuf[cmdPos++] = inDataBuf[count];
                    gState = GET_DLE;
                }
                else
                {
                    DEBUG_PRINT("[GET_CHECKSUM] Wrong, checkSum=0x%x, inDataBuf[count]=0x%x\n", checkSum, inDataBuf[count]);
                }
                break;

            case GET_ETX:
                if (ETX == inDataBuf[count])
                {
                    DEBUG_PRINT("[GET_ETX] Right, getchar=0x%02x\n", inDataBuf[count]);
                    cmdBuf[cmdPos++] = inDataBuf[count];
                    gParsing = 0;
                    //isCmdCompleted = true;
                    for (i = 0; i < cmdPos; i++) {
                        DEBUG_PRINT("%02X-", cmdBuf[i]);
                    }
                    DEBUG_PRINT("<<<<<====TP: ");
                    DEBUG_PRINT("%02X \n", h2vCmd.tp);
                    /*
                    if (v2uCmd.tp == 0x5) {
                        DEBUG_PRINT("<Data: ");
                        for (i = 0; i < dataPos; i++) 
                            DEBUG_PRINT("%02X ", v2uCmd.v2hData.data[i]);
                        DEBUG_PRINT("=======\n");
                    }
                    */
                    parseTpData(&h2vCmd);

                }
                else
                {
                    DEBUG_PRINT("[GET_ETX] Wrong, getchar=0x%02x\n", inDataBuf[count]);
                    cmdPos = 0;
                    memset(cmdBuf, 0, RECV_BUF_SIZE);
                    gState = GET_DLE;
                }
                checkSum = 0;
                cmdPos = 0;
                memset(cmdBuf, 0, RECV_BUF_SIZE);
                dataPos = 0;
                memset(&h2vCmd, 0, sizeof(h2vCmd));
                gState = GET_DLE;
                break;
            }        
                    
            count++;
        }
    }    
}

unsigned char calcChecksum(OutputProtocol *TpData, int len)
{
    int i;
    unsigned char checkSum = 0;
    
    checkSum = TpData->tp;
    for (i = 0; i < len + 1; i++) {
        //printf("%x-", TpData->data.string[i]);
        checkSum ^= TpData->data.string[i];
    }
    return checkSum;
}

unsigned char packOutoutData(unsigned char* outDataBuf, OutputProtocol *TpData)
{
    int i = 0, j = 0;
    int n = 0;
    int cmdLen = 0;
    unsigned char checkSum;
    
    if (outDataBuf == NULL)
        return 0;
    
    outDataBuf[i++] = DLE;
    outDataBuf[i++] = STX;
    outDataBuf[i++] = TpData->tp;     
    
    if (TpData->tp == TP_CHANGE_SCREEN) {
        outDataBuf[i++] = TpData->data.d[0];
        checkSum = TpData->tp ^ TpData->data.d[0];
    } else if (TpData->tp == TP_ADJUST_BACKLIGHT) {
        checkSum = TpData->tp;
        while (j < 4) {
            outDataBuf[i] = TpData->data.d[j++];
            checkSum ^= outDataBuf[i];
            i++;
        }
    } else {
        n = strlen(TpData->data.string);
        //printf("n = %d\n", n);
        checkSum = calcChecksum(TpData, n);
        memcpy(&outDataBuf[i], TpData->data.string, n);
        i+=n;  
    }
    outDataBuf[i++] = checkSum;
    outDataBuf[i++] = DLE;    
    outDataBuf[i++] = ETX;
    cmdLen = i;    
    
#ifdef DEBUG_PACKAGE
    printf("\n");
    for (i = 0; i< cmdLen; i++)
        printf("%02X:", outDataBuf[i]);
    printf(":>cmdLen = %d\n", cmdLen);

#endif
    return cmdLen;
}

static void* ExternalTask(void* arg)
{
    while(!extQuit)
    {
        int i;
        int len;
        unsigned char readLen = 0;
        //unsigned char isToHud = false;
        unsigned char outDataBuf[SEND_BUF_SIZE] = {0};
        OutputProtocol TpData;

        if (gTest)
        {
            outDataBuf[0] = 0x9F;
            outDataBuf[1] = 0x02;
            outDataBuf[2] = 0x04;
            outDataBuf[3] = 0x05;    
            outDataBuf[4] = 0x01;
            outDataBuf[5] = 0x9F;    
            outDataBuf[6] = 0x03;

            for (i = 0; i < 1; i++)
                RS232_SendBuf(WIN32_COM, outDataBuf, 7);
            gTest = false;
        }

        if (gCommand)
        {
            if (gCommand == ASUS_PRODUCION || gCommand == ASUS_ADJUSTMENT)
            {
                TpData.tp = TP_PRODUCT_TEST;
                if (gCommand == ASUS_PRODUCION) {
                    memcpy(TpData.data.string, FACTORY_MODE_CMD, strlen(FACTORY_MODE_CMD));
                }
                else if (gCommand == ASUS_ADJUSTMENT) {
                    memcpy(TpData.data.string, ADJUSTMENT_MODE_CMD, strlen(ADJUSTMENT_MODE_CMD));
                }
                len = packOutoutData(outDataBuf, &TpData);
                RS232_SendBuf(WIN32_COM, outDataBuf, len);
                gCommand = false;
            }
        }
        if (gChgScn)
        {
            TpData.tp = TP_CHANGE_SCREEN;
            TpData.data.d[0] = gChgScn;
            len = packOutoutData(outDataBuf, &TpData);
            RS232_SendBuf(WIN32_COM, outDataBuf, len);
            gChgScn = false;
        }
        if (gChgAdc)
        {
            TpData.tp = TP_ADJUST_BACKLIGHT;
            TpData.data.d[0] = 0;
            TpData.data.d[1] = 0;
            TpData.data.d[2] = (adcVal & 0xFF00) >> 8;
            TpData.data.d[3] = (adcVal & 0x00FF) >> 0;
            len = packOutoutData(outDataBuf, &TpData);
            RS232_SendBuf(WIN32_COM, outDataBuf, len);
            gChgAdc = false;
        }

        readLen = RS232_PollComport(WIN32_COM, inDataBuf, RECV_BUF_SIZE);
        if (readLen > 0) {
            //printf("readLen = %d\n", readLen);
            //fflush(stdout);
            errorCorrection(readLen);
        }
        
        Sleep(1);
    }
}

int HudInit(void)
{
    
    int baudrate = 115200;  /* 115200 */
    char mode[]  = {'8', 'E', '1', 0};    
   
#ifdef _WIN32
    /* Open COM Port */
    if (RS232_OpenComport(WIN32_COM, baudrate, mode)) {
        return 1;
    }
    printf("Open COM Port Success\n\n");
#endif

    pthread_create(&a_thread, NULL, ExternalTask, NULL);
    return 0;
}

void HudExit(void)
{
    extQuit = true;
    pthread_join(a_thread, NULL);
    
    /* Close COM Port */
    RS232_CloseComport(WIN32_COM);
    printf("\nClose COM Port\n");    
}

void SecondMenu(int typeData)
{
    int choice;
    int inSecond = 1;
    
    while(inSecond) {
        if (typeData == 1) {
            //gCommand = TP_CHANGE_SCREEN;
            printf("\n\n\t===========================\n");
            printf("\t0. <<< return previous menu \n");
            printf("\t1. show color bar \n");
            printf("\t2. show gray \n");
            printf("\t3. show line pair 2 \n");
            printf("\t4. show white \n");
            printf("\t5. show black \n");
            printf("\t6. show white frame\n");
            printf("\t7. get adc of light \n");
            printf("\t8. get adc of thermal \n");
            printf("\t9. get if key is pressed \n");
            printf("\t===========================\n\n");
            printf("\tEnter your choice :  ");
            scanf("%d",&choice);
        } else if (typeData == 2) {
            //gCommand = TP_ADJUST_BACKLIGHT;
            printf("\n\n\t===========================\n");
            printf("\t0. <<< return previous menu \n");
            printf("\t1. set the value of backlight \n");
            printf("\t===========================\n\n");
            printf("\tEnter your choice :  ");
            scanf("%d",&choice);
        }
        printf("\n");
        printf("\tyour select is ==> %d\n", choice);

        if (typeData == 1) {
            switch (choice)
            {
                case 0:
                    inSecond = 0;
                    break;
                case 1:
                    //printf("\tyour select is ==> %d\n", choice);
                    printf("\t<<show color bar>>\n");
                    gChgScn = 0x34;
                    break;
                case 2:
                    //printf("\tyour select is ==> %d\n", choice);
                    printf("\t<<show gray>>\n");
                    gChgScn = 0x35;
                    break;
                case 3:
                    //printf("\tyour select is ==> %d\n", choice);
                    printf("\t<<show line pair 2>>\n");
                    gChgScn = 0x36;
                    break;
                case 4:
                    //printf("\tyour select is ==> %d\n", choice);
                    printf("\t<<show white>>\n");
                    gChgScn = 0x37;
                    break;
                case 5:
                    //printf("\tyour select is ==> %d\n", choice);
                    printf("\t<<show black>>\n");
                    gChgScn = 0x38;
                    break;
                case 6:
                    //printf("\tyour select is ==> %d\n", choice);
                    printf("\t<<show white frame>>\n");
                    gChgScn = 0x39;
                    break;
                //////
                case 7:
                    //printf("\tyour select is ==> %d\n", choice);
                    printf("\t<<get adc of light>>\n");
                    gChgScn = 0x31;
                    while (gKey);
                    gKey = 1;
                    break;
                case 8:
                    //printf("\tyour select is ==> %d\n", choice);
                    printf("\t<<get adc of thermal>>\n");
                    gChgScn = 0x32;
                    while (gKey);
                    gKey = 1;
                    break;
                case 9:
                    //printf("\tyour select is ==> %d\n", choice);
                    printf("\t<<get if key is pressed>>\n");
                    gChgScn = 0x33;
                    break;
                default:
                    printf("\t>>>>>>>>no function\n");
                    break;
            }
        } else if (typeData == 2) {
            switch (choice)
            {
                case 0:
                    inSecond = 0;
                    break;
                case 1:
                    printf("\n\tEnter ADC value(0~965):  ");
                    scanf("%d", &adcVal);
                    if (adcVal < 0 || adcVal > 0x3C5) {
                        printf("\tADC value exceeds limit!\n");
                        gChgAdc = false;
                    }
                    else {
                        printf("\tyou input adc value: %d\n", adcVal);
                        gChgAdc = true;
                    }
                    break;
                default:
                    break;
            }
        #if 0
            scanf("%d",&adcVal);
            if (adcVal) {
                gChgAdc = true;
            } else {
                gChgAdc = false;
            }
        #endif
        }
    }
}

int main()
{
    int choice;
    int ret;
    
    ret = HudInit();
    if (ret)
        return(0);
    
    Sleep(1);
    printf("[HUD simulator] HUD version: %d\n", gVersion);  

    while(1) {
        printf("\n===================\n");
        printf("1. Production test \n");
        printf("2. Adjustment mode\n");
        printf("3. test test\n");
        printf("0. Exit\n");
        printf("===================\n\n");
        printf("Enter your choice :  ");
        scanf("%d",&choice);
        
        switch (choice)
        {
            case ASUS_PRODUCION:
                gCommand = ASUS_PRODUCION;
                printf("your select is ==> %s\n", FACTORY_MODE_CMD);
                while (gKey); 
                gKey = 1;
                SecondMenu(choice);
                break;
            case ASUS_ADJUSTMENT:
                gCommand = ASUS_ADJUSTMENT;
                printf("your select is ==> %s\n", ADJUSTMENT_MODE_CMD);
                SecondMenu(choice);
                break;
            case 3:
                gTest = true;
                printf("your select is ==> %s\n", "TEST");
                //SecondMenu(choice);
                break;
            case 0:
                exit(0);
                
        }
    }

    return(0);
}

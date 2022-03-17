#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "iniparser.h"
#include "dictionary.h"

#define LEN_MAX 16

typedef enum _WIESS_CUSTOMIZE_SECTION
{
    WS_SPEED    = 0,
    WS_GEAR,
    WS_ODO,
    WS_TEST1,
    WS_TEST2,
    WS_TEST3,
    WS_TEST4,
    WS_TEST5,
    WS_TEST6,
    WS_TEST7,
    WS_TEST8,
    WS_TEST9,    
    WS_TEST10, 
} WIESS_CUSTOMIZE_SECTION;

struct wiess_section_table
{
    WIESS_CUSTOMIZE_SECTION widget;
    char *widget_string;
} ;

struct wiess_section_table ws_table[] = 
{
    {WS_SPEED,  "speed"},
    {WS_GEAR,   "gear"},
    {WS_ODO,    "odo"},
    {WS_TEST1,  "test1"},
    {WS_TEST2,  "test2"},
    {WS_TEST3,  "test3"},
    {WS_TEST4,  "test4"},
    {WS_TEST5,  "test5"},
    {WS_TEST6,  "test6"},
    {WS_TEST7,  "test7"},
    {WS_TEST8,  "test8"},
    {WS_TEST9,  "test9"},
    {WS_TEST10, "test10"},
} ;

typedef enum _WIESS_CUSTOMIZE_KEY
{
    WK_CANID,
    WK_BYTE_H,
    WK_BYTE_L,
    WK_BIT_H,
    WK_BIT_L,
} WIESS_CUSTOMIZE_KEY;

struct wiess_key_table
{
    WIESS_CUSTOMIZE_KEY key;
    char *key_string;
} ;

struct wiess_key_table wk_table[] =
{
    {WK_CANID,  "canid"},
    {WK_BYTE_H, "byteH"},
    {WK_BYTE_L, "byteL"},
    {WK_BIT_H,  "bitH"},
    {WK_BIT_L,  "bitL"},    
} ;

struct widgetInfo_t
{
    WIESS_CUSTOMIZE_SECTION widget;
    char byteH;
    char byteL;
    char bitH;
    char bitL;
    int len;
} ;

typedef struct widgetInfo_t WidgetInfo_t;

struct widgetNode_t
{
    WidgetInfo_t info;
    struct widgetNode_t* next;
} ;
typedef struct widgetNode_t WidgetNode_t;

/*
typedef struct 
{
    tElementInfo speed;
    tElementInfo gear;
    tElementInfo odo;
} tCanbusFormat;
*/

/* Link List */
struct canIdNode_t
{
    int canId;
    //struct canIdNode *next;
    WidgetNode_t* start;
};
typedef struct canIdNode_t CanIdNode_t;
typedef CanIdNode_t *CanIdNodePtr;


/* Dynamic Array */
struct _darray_t;
typedef struct _darray_t Darray_t;

struct _darray_t
{  
    size_t size;        // used size
    size_t capacity;    // allocate size
    //size_t head;
    size_t tail;
    //int *elems;
    CanIdNodePtr elems;
    //struct widgetInfo_t* start;
};
Darray_t* darray_create(void);
bool darray_push_back(Darray_t* self, void* data);
bool darray_id_exist(Darray_t* self, const int canid);
int darray_find_id(Darray_t* self, const int canid);
bool darray_empty(const Darray_t* self);
bool darray_is_full(const Darray_t* self);
int darray_ring_size(const Darray_t* self);
int darray_size(const Darray_t* self);
int darray_capacity(const Darray_t* self);
void darray_print(const Darray_t* self);

Darray_t* darray_create(void)
{
    Darray_t* darr = malloc(sizeof(Darray_t));
    
    if (darr != NULL)
    {
        darr->size = 0;
        darr->capacity = 5; // for test
        //darr->head = 0;
        darr->tail = 0;
        darr->elems = malloc(darr->capacity * sizeof(CanIdNode_t));
        if (!darr->elems)
        {
            free(darr->elems);
            free(darr);
            darr = NULL;
            return darr;
        }
    }
    return darr;
}

static bool darray_expand(Darray_t* self)
{
    size_t capacity;
    size_t need = 1;
    //int* old_darr;
    //int* new_darr;
    assert(self);
    
    printf("capacity=%d, size=%d\n", darray_capacity(self), darray_size(self));
    if (self->size + need > self->capacity)
    {
        //capacity = (darray_capacity(self) << 1;   //double memory alloc
        capacity = darray_capacity(self) + (darray_capacity(self) >> 1);   //1.5 memory alloc
        CanIdNodePtr old_darr = self->elems;
        //printf(">>>(%d)\n", __LINE__);
        CanIdNodePtr new_darr = malloc(capacity * sizeof(CanIdNode_t));
        //printf(">>>(%d) %p\n", __LINE__, new_darr);
        if (!new_darr)
            return false;
        printf(">>>(%d)\n", __LINE__);
        size_t s = 0;
        while (s < darray_size(self))
        {
            printf("mv old to new: s=%I64d old=%d\n", s, old_darr[s].canId);
            new_darr[s] = old_darr[s];
            s++;
        }
        
        self->elems = new_darr;
        self->tail = s;
        self->capacity = capacity;
        printf("expand>>>>>> capacity=%d, size=%d\n", darray_capacity(self), darray_size(self));
        //printf(">>>>%I64d, %I64d\n", self->tail, self->capacity);
        free(old_darr);
        #if 0
        new_darr = (int*)realloc(self->elems, self->capacity * sizeof(int));
        if (new_darr)
        {
            self->elems = new_darr;
            self->capacity = capacity;
        }
        #endif
    }
    //return ((self->size + need) <= self->capacity) ? false : true;
    return true;
}

bool darray_push_back(Darray_t* self, void* data) 
{
    assert(self);
    if (darray_expand(self))
    {
        self->elems[self->tail++] = *((CanIdNodePtr)data);
        self->size++;
    } 
    else
    {
        printf("expand failed\n");
    }
    //printf("darray size=%d\n", darray_size(self));
    return true;
}

// TODO: improve performance
bool darray_id_exist(Darray_t* self, const int canid)
{
    assert(self);
    for (int i = 0; i < darray_size(self); i++)
    {
        if (canid == self->elems[i].canId)
            return true;
    }
    return false;
}

// TODO: improve performance
int darray_find_id(Darray_t* self, const int canid)
{
    assert(self);
    for (int i = 0; i < darray_size(self); i++)
    {
        if (canid == self->elems[i].canId)
            return i;
    }
    return -1;
}

/*
bool darray_ring_append(Darray_t* self, int data) //void*
{
    assert(self);
    
    if (darray_is_full(self))
    {
        self->head = (self->head + 1) % self->capacity;
        self->elems[self->head] = 0;
    }
        
    self->tail = (self->tail + 1) % self->capacity;
    self->elems[self->tail] = data;
    self->size++;
    printf("head=%I64d, tail=%I64d\n", self->head, self->tail);
    
    return true;
}
*/

/* method of getting size/capacity  */
bool darray_empty(const Darray_t* self)
{
    assert(self);
    //return self->head == self->tail;
    return self->tail == 0;
}

bool darray_is_full(const Darray_t* self)
{
    //int real_capacity = self->capacity - 1;
    assert(self);
    //return darray_ring_size(self) == real_capacity;
    return self->capacity == self->size;
}

//for ring buffer
/*
int darray_ring_size(const Darray_t* self)
{
    int size = 0;
    
    if (self->head < self->tail)
        size = self->tail - self->head;
    else
        size = self->tail + self->capacity - self->head;
    return size;
}
*/

int darray_size(const Darray_t* self)
{
    return self->size;
}

int darray_capacity(const Darray_t* self)
{
    return self->capacity;
}

void darray_print(const Darray_t* self)
{
    int i;
    assert(self);
    
    if (darray_empty(self))
        printf("drray is empty!\n");
    else
    {
#if 0
        int curIdx = self->head + 1;
        for (i = 0; i < darray_ring_size(self); i++)
        {
            printf("%d ", self->elems[curIdx]);
            curIdx = (curIdx + 1) % self->capacity;
        }
#else
        printf("=====capacity=%I64d, size=%I64d=====\n", self->capacity, self->size);
        for (i = 0; i < darray_size(self); i++)
        {
            printf("=====CANID(0x%x)=====\n", self->elems[i].canId);

            WidgetNode_t *node = self->elems[i].start;
            while (node != NULL)
            {
                //printf("have widget\n");
                //printf("widget=%d ", self->elems->start->info.widget);
                //printf("byte H:%d L:%d\n", self->elems->start->info.byteH, self->elems->start->info.byteL);
                printf("\twidget=%d ", node->info.widget);
                printf("byte H:%d L:%d\n", node->info.byteH, node->info.byteL);
                node = node->next;
            }
        }
#endif
    }
}

/*
bool is_drrary_empty(const Darray_t* self)
{
    assert(self);
    return self->size <= 0;
}
*/

//tCanbusFormat theCanbusFmt;
static dictionary* canFmtIni;

void add_widget(const int canId, Darray_t* darray, WidgetInfo_t* info)
{
    WidgetNode_t* nodePtr;
    nodePtr = (WidgetNode_t*)malloc(sizeof(WidgetNode_t));
    nodePtr->info.widget = info->widget;
    nodePtr->info.byteH = info->byteH;
    nodePtr->info.byteL = info->byteL;
    nodePtr->next = NULL;
    
    int idx = darray_find_id(darray, canId);
    //printf("idx=%d(%d)\n", idx, __LINE__);
    if (idx != -1)
    {
        if (darray->elems[idx].start == NULL) {
            darray->elems[idx].start = nodePtr;
            //printf("new canid==>(%d)%x\n", __LINE__, canId);
            return;
        }
        else
        {
            WidgetNode_t* curPtr = darray->elems[idx].start;
            while (curPtr->next != NULL)
                curPtr = curPtr->next;
            curPtr->next = nodePtr;
            //printf("cat canid==>(%d)%x\n", __LINE__, canId);
        }
    }
    return;
}

void make_string(char *name, int len, char *key)
{
    int i = len;
    int size = LEN_MAX;

    while (i < size) {
        name[i] = '\0';
        i++;
    }
    //printf("%s, %d, %s\n", name, len, key); 
    strcat(name, key);
    //printf("%s, %d, %s\n", name, len, key); 
    return;
}

int main(int argc, char* argv[])
{
    int canId;
    WidgetInfo_t info;
    int numWidget = 0;
    int numKey = 0;
    int i, len;
    char name[LEN_MAX] = {0};
  
    canFmtIni = iniparser_load("canbusfmt.ini");
    if (!canFmtIni)
    {
        canFmtIni = dictionary_new(0);
        assert(canFmtIni);
        
        dictionary_set(canFmtIni, "speed", NULL);
        dictionary_set(canFmtIni, "gear", NULL);
        dictionary_set(canFmtIni, "odo", NULL);
    }
    
    Darray_t* darrayPtr = darray_create();
    numWidget = sizeof(ws_table)/sizeof(struct wiess_section_table);
    numKey = sizeof(wk_table)/sizeof(struct wiess_key_table);
    printf("==>%d, %d\n", numWidget, numKey);
    
    for (i = 0; i < numWidget ; i++) //numWidget 5
    {
        strcpy(name, ws_table[i].widget_string);
        strcat(name, ":");
        len = strlen(name);
        
        make_string(name, len, wk_table[WK_CANID].key_string);
        canId = iniparser_getint(canFmtIni, name, 0x101);
        if (!darray_id_exist(darrayPtr, canId)) //darrayPtr->elems.start == NULL || 
        {
            //printf("\t===>create\n");
            CanIdNode_t canIdNode = {canId, NULL/*, NULL*/};
            darray_push_back(darrayPtr, (void*)&canIdNode); 
        }
        
        info.widget = ws_table[i].widget;
        make_string(name, len, wk_table[WK_BYTE_H].key_string);
        info.byteH = iniparser_getint(canFmtIni, name, 0);
        
        make_string(name, len, wk_table[WK_BYTE_L].key_string);
        info.byteL = iniparser_getint(canFmtIni, name, 0);
        
        add_widget(canId, darrayPtr, &info);
        
    #if 0
        for (j = 0; j < numKey; j++)
        { 
            make_string(name, len, wk_table[j].key_string);
            printf("===>%s\n", name);
        }
    #endif    
        //canId = iniparser_getint(canFmtIni, name, 0x101);
    }
    // speed
    //canId = iniparser_getint(canFmtIni, "speed:canid", 0x101);
    //if canId is new
    //CanIdNode canIdNode = {canId, NULL, NULL};
    //darray_push_back(darrayPtr, (void*)&canIdNode);  
    /*
    info.widget = WS_SPEED;
    info.byteH = iniparser_getint(canFmtIni, "speed:byteH", 0);
    info.byteL = iniparser_getint(canFmtIni, "speed:byteL", 0);
    add_widget(canId, darrayPtr, &info);

    // odo
    info.widget = WS_ODO;
    canId = iniparser_getint(canFmtIni, "odo:canid", 0x102);
    info.byteH = iniparser_getint(canFmtIni, "odo:byteH", 0);
    info.byteL = iniparser_getint(canFmtIni, "odo:byteL", 0); 
    add_widget(canId, darrayPtr, &info);
    */
    darray_print(darrayPtr);
/*
    // gear
    theCanbusFmt.gear.canId = iniparser_getint(canFmtIni, "gear:canid", 0x102);
    theCanbusFmt.gear.byteH = iniparser_getint(canFmtIni, "gear:byteH", 0);
    theCanbusFmt.gear.byteL = iniparser_getint(canFmtIni, "gear:byteL", 0);  

 
*/
    //printf("0x%x\n", theCanbusFmt.speed.canId);

#if 0    
    Darray_t* darray = darray_create();
    int cnt = 1;
    int number = atoi(argv[1]);
    printf(">>>(%d)\n", __LINE__);
    for (int i = 0; i < number; i++)
    {
        darray_push_back(darray, (void*)&cnt);
        cnt++;
    }
    printf(">>>(%d)\n", __LINE__);

    darray_print(darray);
#endif
}

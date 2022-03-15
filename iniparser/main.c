#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "iniparser.h"
#include "dictionary.h"

typedef enum _WIESS_CUSTOMIZE_WIDGET
{
    WS_SPEED    = 1,
    WS_GEAR     = 2,
    WS_ODO      = 3,
} WIESS_CUSTOMIZE_WIDGET;

struct widgetInfo_t
{
    WIESS_CUSTOMIZE_WIDGET widget;
    char byteH;
    char byteL;
    char bitH;
    char bitL;
    int len;
    struct widgetInfo_t* next;
} ;
typedef struct widgetInfo_t WidgetInfo_t;

/*
typedef struct 
{
    tElementInfo speed;
    tElementInfo gear;
    tElementInfo odo;
} tCanbusFormat;
*/

/* Link List */
struct canIdNode
{
    int canId;
    struct canIdNode *next;
};
typedef struct canIdNode CanIdNode;
typedef CanIdNode *CanIdNodePtr;


/* Dynamic Array */
struct _darray_t;
typedef struct _darray_t darray_t;

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
darray_t* darray_create(void);
bool darray_push_back(darray_t* self, void* data);
bool darray_empty(const darray_t* self);
bool darray_is_full(const darray_t* self);
int darray_ring_size(const darray_t* self);
int darray_size(const darray_t* self);
int darray_capacity(const darray_t* self);
void darray_print(const darray_t* self);

darray_t* darray_create(void)
{
    darray_t* darr = malloc(sizeof(darray_t));
    
    if (darr != NULL)
    {
        darr->size = 0;
        darr->capacity = 5; // for test
        //darr->head = 0;
        darr->tail = 0;
        darr->elems = malloc(darr->capacity * sizeof(CanIdNode));
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

static bool darray_expand(darray_t* self)
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
        CanIdNodePtr new_darr = malloc(capacity * sizeof(CanIdNode));
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

bool darray_push_back(darray_t* self, void* data) 
{
    assert(self);
    if (darray_expand(self))
    {
        self->elems[self->tail++] = *((CanIdNodePtr)data);
        //self->tail = (self->tail + 1);
        self->size++;
        //printf("head=%I64d, tail=%I64d\n", self->head, self->tail);
    } 
    else
    {
        printf("expand failed\n");
    }
    
    return true;
}

/*
bool darray_ring_append(darray_t* self, int data) //void*
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
bool darray_empty(const darray_t* self)
{
    assert(self);
    //return self->head == self->tail;
    return self->tail == 0;
}

bool darray_is_full(const darray_t* self)
{
    //int real_capacity = self->capacity - 1;
    assert(self);
    //return darray_ring_size(self) == real_capacity;
    return self->capacity == self->size;
}

//for ring buffer
/*
int darray_ring_size(const darray_t* self)
{
    int size = 0;
    
    if (self->head < self->tail)
        size = self->tail - self->head;
    else
        size = self->tail + self->capacity - self->head;
    return size;
}
*/

int darray_size(const darray_t* self)
{
    return self->size;
}

int darray_capacity(const darray_t* self)
{
    return self->capacity;
}

void darray_print(const darray_t* self)
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
            printf("%d(0x%x) ", self->elems[i].canId, self->elems[i].canId);
        }
#endif
    }
}

/*
bool is_drrary_empty(const darray_t* self)
{
    assert(self);
    return self->size <= 0;
}
*/

//tCanbusFormat theCanbusFmt;
static dictionary* canFmtIni;

void add_node(CanIdNodePtr *start, int canId)
{
    CanIdNodePtr newPtr;
    newPtr = (CanIdNodePtr)malloc(sizeof(CanIdNode));
    newPtr->canId = canId;
    newPtr->next = NULL;
    
    if (start == NULL) {
        *start = newPtr;
        return;
    }
    else {
        CanIdNodePtr curPtr = *start;
        while (curPtr->next != NULL)
            curPtr = curPtr->next;
        curPtr->next = newPtr;
    }
    return;
}


int main(int argc, char* argv[])
{
    int canId;
    WidgetInfo_t* widgetPtr;
    canFmtIni = iniparser_load("canbusfmt.ini");
    if (!canFmtIni)
    {
        canFmtIni = dictionary_new(0);
        assert(canFmtIni);
        
        dictionary_set(canFmtIni, "speed", NULL);
        dictionary_set(canFmtIni, "gear", NULL);
        dictionary_set(canFmtIni, "odo", NULL);
    }
    
    darray_t* darray = darray_create();
    // speed
    canId = iniparser_getint(canFmtIni, "speed:canid", 0x101);
    
    
    widgetPtr = (WidgetInfo_t*)malloc(sizeof(WidgetInfo_t));
    if (!widgetPtr)
        printf("errlr\n");
    widgetPtr->widget = WS_SPEED;
    widgetPtr->byteH = iniparser_getint(canFmtIni, "speed:byteH", 0);
    widgetPtr->byteL = iniparser_getint(canFmtIni, "speed:byteL", 0);
    widgetPtr->next = NULL;
    darray_push_back(darray, (void*)&canId);

/*
    // gear
    theCanbusFmt.gear.canId = iniparser_getint(canFmtIni, "gear:canid", 0x102);
    theCanbusFmt.gear.byteH = iniparser_getint(canFmtIni, "gear:byteH", 0);
    theCanbusFmt.gear.byteL = iniparser_getint(canFmtIni, "gear:byteL", 0);  

    // odo
    theCanbusFmt.odo.canId = iniparser_getint(canFmtIni, "odo:canid", 0x102);
    theCanbusFmt.odo.byteH = iniparser_getint(canFmtIni, "odo:byteH", 0);
    theCanbusFmt.odo.byteL = iniparser_getint(canFmtIni, "odo:byteL", 0);  
*/
    //printf("0x%x\n", theCanbusFmt.speed.canId);

#if 0    
    darray_t* darray = darray_create();
    int cnt = 1;
    int number = atoi(argv[1]);
    printf(">>>(%d)\n", __LINE__);
    for (int i = 0; i < number; i++)
    {
        darray_push_back(darray, (void*)&cnt);
        cnt++;
    }
    printf(">>>(%d)\n", __LINE__);
#endif
    darray_print(darray);
}

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "iniparser.h"
#include "dictionary.h"

typedef struct
{
    int canId;
    char byteH;
    char byteL;
    char bitH;
    char bitL;
    int len;
} tElementInfo;

typedef struct 
{
    tElementInfo speed;
    tElementInfo gear;
    tElementInfo odo;
} tCanbusFormat;

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
    size_t alloc_size;  // current size
    size_t capacity;    // maximum size
    size_t head;
    size_t tail;
    int *elems;
};
darray_t* darray_create(void);
bool darray_append(darray_t* self, int data);
bool darray_is_empty(const darray_t* self);
bool darray_is_full(const darray_t* self);
int darray_size(const darray_t* self);
void darray_print(const darray_t* self);

darray_t* darray_create(void)
{
    darray_t* darr = malloc(sizeof(darray_t));
    
    if (darr != NULL)
    {
        darr->alloc_size = 0;
        darr->capacity = 5; // for test
        darr->head = 0;
        darr->tail = 0;
        darr->elems = malloc(darr->capacity * sizeof(int));
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

bool darray_append(darray_t* self, int data) //void*
{
    assert(self);
    
    // if (!darray_expand(self)) 
        
    //if (self->alloc_size > 0)
    if (darray_is_full(self))
    {
        self->head = (self->head + 1) % self->capacity;
        self->elems[self->head] = 0;
    }
        
    self->tail = (self->tail + 1) % self->capacity;
    self->elems[self->tail] = data;
    self->alloc_size++;
    printf("head=%I64d, tail=%I64d\n", self->head, self->tail);
    
    return true;
}

bool darray_is_empty(const darray_t* self)
{
    assert(self);
    return self->head == self->tail;
}

bool darray_is_full(const darray_t* self)
{
    int real_capacity = self->capacity - 1;
    assert(self);
    return darray_size(self) == real_capacity;
}

int darray_size(const darray_t* self)
{
    int size = 0;
    
    if (self->head < self->tail)
        size = self->tail - self->head;
    else
        size = self->tail + self->capacity - self->head;
    return size;
}

void darray_print(const darray_t* self)
{
    int i;
    int curIdx;
    
    assert(self);
    if (darray_is_empty(self))
        printf("drray is empty!\n");
    else
    {
        curIdx = self->head + 1;
        for (i = 0; i < darray_size(self); i++)
        {
            printf("%d ", self->elems[curIdx]);
            curIdx = (curIdx + 1) % self->capacity;
        }
    }
}

/*
bool is_drrary_empty(const darray_t* self)
{
    assert(self);
    return self->alloc_size <= 0;
}
*/

tCanbusFormat theCanbusFmt;
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


int main()
{
    canFmtIni = iniparser_load("canbusfmt.ini");
    if (!canFmtIni)
    {
        canFmtIni = dictionary_new(0);
        assert(canFmtIni);
        
        dictionary_set(canFmtIni, "speed", NULL);
        dictionary_set(canFmtIni, "gear", NULL);
        dictionary_set(canFmtIni, "odo", NULL);
    }
    // speed
    theCanbusFmt.speed.canId = iniparser_getint(canFmtIni, "speed:canid", 0x101);
    theCanbusFmt.speed.byteH = iniparser_getint(canFmtIni, "speed:byteH", 0);
    theCanbusFmt.speed.byteL = iniparser_getint(canFmtIni, "speed:byteL", 0);

    // gear
    theCanbusFmt.gear.canId = iniparser_getint(canFmtIni, "gear:canid", 0x102);
    theCanbusFmt.gear.byteH = iniparser_getint(canFmtIni, "gear:byteH", 0);
    theCanbusFmt.gear.byteL = iniparser_getint(canFmtIni, "gear:byteL", 0);  

    // odo
    theCanbusFmt.odo.canId = iniparser_getint(canFmtIni, "odo:canid", 0x102);
    theCanbusFmt.odo.byteH = iniparser_getint(canFmtIni, "odo:byteH", 0);
    theCanbusFmt.odo.byteL = iniparser_getint(canFmtIni, "odo:byteL", 0);  

    //printf("0x%x\n", theCanbusFmt.speed.canId);
    
    darray_t* darray = darray_create();
    darray_append(darray, 11);
    darray_append(darray, 22);
    darray_append(darray, 33);
    darray_append(darray, 44);
    //darray_append(darray, 55);
    //darray_append(darray, 66);
    darray_print(darray);
    //printf("darray_is_empty:%d\n", darray_is_empty(darray));
}

#ifndef __ROUTE_H__
#define __ROUTE_H__

#include "lib_io.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct{
    int m_id;
    int m_cpu;
    int m_mem;
} VirtualMachine;

typedef struct{
    int m_cpu;
    int m_mem;
    int m_hard;
} Machine;

typedef struct{
    Machine m_machine;

    int m_numOfVM;
    VirtualMachine ** m_vm;

    time_t m_startTime;
    time_t m_endTime;    
} Info;
	
typedef struct{
    char m_idOfRecord[20];
    int m_idOfVM;
    time_t m_time;
} Record;

void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename);

Info parseInfo(char *info[MAX_INFO_NUM]);

Record** parseRecord(char * data[MAX_DATA_NUM], int data_num);

Machine parseMachine(char *machine);

time_t parseTime(char *time);

VirtualMachine* parseVirtualMachine(char *vm);

int* predict_vm(Info info, Record **record, int data_num);

#endif

#include "predict.h"

#include <stdio.h>

//��Ҫ��ɵĹ��������
void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename)
{
	// ��Ҫ���������
	char * result_file = (char *)"17\n\n0 8 0 20";
	Info information = parseInfo(info);
	Record **record = parseRecord(data, data_num);

	int * result = predict_vm(information, record, data_num);
	for(int i = 0; i < 16; ++i)
		printf("%d ", result[i]);
	// ֱ�ӵ�������ļ��ķ��������ָ���ļ���(ps��ע���ʽ����ȷ�ԣ�����н⣬��һ��ֻ��һ�����ݣ��ڶ���Ϊ�գ������п�ʼ���Ǿ�������ݣ�����֮����һ���ո�ָ�)
	write_result(result_file, filename);
}

int* predict_vm(Info info, Record **record, int data_num) { 
	int *vm = (int*)malloc(sizeof(int) * 16); 
	int test_time = info.m_endTime - info.m_startTime;
	int train_time = record[data_num-1]->m_time - record[0]->m_time;
	printf("test_time %d, train_time %d\n", test_time, train_time);
	int sum = 0;
	for (int i=0; i<data_num; i++){
		for (int j=0; j<info.m_numOfVM; j++)
			if (record[i]->m_idOfVM==info.m_vm[j]->m_id) {
				vm[j]++;
				break;
			}
	}

	for(int j = 0; j< info.m_numOfVM; j++){
		vm[j] = vm[j]*test_time/train_time;
	}
	return vm;
}

Machine parseMachine(char *machine){
	Machine m;
	sscanf(machine, "%d %d %d", &m.m_cpu, &m.m_mem, &m.m_hard);
	printf("%d %d %d\n", m.m_cpu, m.m_mem, m.m_hard);
	return m;
}

time_t parseTime(char *time){
	tm t;
	sscanf(time, "%d-%d-%d %d:%d:%d", &t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec);
	//printf("%d-%d-%d %d:%d:%d\n", t.year, t.month, t.day, t.hour, t.minute, t.second);
	return mktime(&t);
}

VirtualMachine* parseVirtualMachine(char *vm){
	VirtualMachine *virtualmachine = (VirtualMachine*)malloc(sizeof(VirtualMachine));
	sscanf(vm, "flavor%d %d %d", &virtualmachine->m_id, &virtualmachine->m_cpu, &virtualmachine->m_mem);
	printf("%d %d %d\n", virtualmachine->m_id, virtualmachine->m_cpu, virtualmachine->m_mem);
	return virtualmachine;
}

Info parseInfo(char *info[MAX_INFO_NUM]){
	Info information;

	int index = 0;
	Machine m = parseMachine(info[index]);
	information.m_machine.m_cpu = m.m_cpu;
	information.m_machine.m_mem = m.m_mem; 
	information.m_machine.m_hard = m.m_hard;

	index = 2;
	int numOfVM = 0;
	sscanf(info[index], "%d", &numOfVM);
	information.m_numOfVM = numOfVM;

	information.m_vm = (VirtualMachine**)malloc(sizeof(VirtualMachine*) * numOfVM);

	index = 3;
	for(int i = 0; i < numOfVM; ++i){
		information.m_vm[i] = parseVirtualMachine(info[index]);
		++index;
	}

	// 跳过cpu这一行
	index += 3;
	information.m_startTime = parseTime(info[index]);
	information.m_endTime = parseTime(info[++index]);

	return information;
}

Record** parseRecord(char * data[MAX_DATA_NUM], int data_num){
	Record **r = (Record**)malloc(sizeof(Record*) * data_num);

	for(int i = 0; i < data_num; ++i){
		r[i] = (Record*)malloc(sizeof(Record));
		char time[50];
		sscanf(data[i], "%s flavor%d %s %s", r[i]->m_idOfRecord, &r[i]->m_idOfVM, time, time + 11);
		time[10] = '\t';
		r[i]->m_time = parseTime(time);
	}
	return r;
}
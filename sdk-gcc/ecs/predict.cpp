#include "predict.h"

#include <stdio.h>

//��Ҫ��ɵĹ��������
void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename)
{
	// ��Ҫ���������
	char * result_file = (char *)malloc(sizeof(char) * MAX_DATA_NUM);
	Info information = parseInfo(info);
	Record **record = parseRecord(data, data_num);
	ResultLine *r = NULL;
	int numOfMachine = 0;

	int * vm = predict_vm(information, record, data_num);
	/*for(int i = 0; i < 16; ++i)
		printf("%d ", vm[i]);
	printf("\n");*/
	int count = print_vm(result_file, vm, information);
	r = vm_placement(vm, information, numOfMachine, count);
	print_placement(result_file, numOfMachine, r);
	// ֱ�ӵ�������ļ��ķ��������ָ���ļ���(ps��ע���ʽ����ȷ�ԣ�����н⣬��һ��ֻ��һ�����ݣ��ڶ���Ϊ�գ������п�ʼ���Ǿ�������ݣ�����֮����һ���ո�ָ�)
	write_result(result_file, filename);
}

int* predict_vm(Info info, Record **record, int data_num) { 
	int *vm = (int*)malloc(sizeof(int) * 16); 
	int test_time = info.m_endTime - info.m_startTime;
	int train_time = record[data_num-1]->m_time - record[0]->m_time;
	//printf("test_time %d, train_time %d\n", test_time, train_time);	
	for (int i = 0; i < data_num; i++){
		for (int j = 0; j < info.m_numOfVM; j++)
			if (record[i]->m_idOfVM == info.m_vm[j]->m_id) {
				vm[j]++;
				break;
			}
	}

	for(int j = 0; j< info.m_numOfVM; j++){
		vm[j] = vm[j]*test_time/train_time;
	}
	return vm;
}

int print_vm(char *buffer, int *vm, Info info){
	int count = 0,offset = 0;	
	for (int i = 0; i < info.m_numOfVM; i++){
		if (vm[i] > 0){
			count += vm[i];
		}
	}
	offset = sprintf(buffer, "%d\n", count);
	for (int i = 0; i < info.m_numOfVM; i++){
		offset += sprintf(buffer + offset, "flavor%d %d\n", info.m_vm[i]->m_id, vm[i]);		
	}
	offset += sprintf(buffer + offset, "\n");
	//printf("%s",buffer);
	return count;	
}

ResultLine* vm_placement(int *vm, Info info, int &numOfMachine, int count){
	int /*mem_cnt = 0, cpu_cnt = 0,*/ m_cnt = 0;
	ResultLine *r = (ResultLine*)malloc(sizeof(ResultLine) * MAX_DATA_NUM);
	VirtualMachine vm_one[count+100];
	int cnt = 0;
	// expand vm to vm_one
	for (int i=0; i<info.m_numOfVM; i++){
		for (int j=0; j<vm[i]; j++){
			vm_one[cnt].m_id = info.m_vm[i]->m_id;
    			vm_one[cnt].m_cpu = info.m_vm[i]->m_cpu;
    			vm_one[cnt].m_mem = info.m_vm[i]->m_mem/1024;
			cnt++;
		}
	}
	int next[count+100][info.m_machine.m_cpu+100][info.m_machine.m_mem+100]; //next[i] means the next one of vm_one[i] is planned
	int arrange[count+100]; //arrange[i] means vm_one[i] is arranged or not
	for (int i = 0; i < count; i++)
		arrange[i] = 0;

	double utility[count+100][info.m_machine.m_cpu+100][info.m_machine.m_mem+100];

	//for (int i = 0; i < count; i++)
	//	printf("%d %d %d %d \n", i, vm_one[i].m_id, vm_one[i].m_cpu, vm_one[i].m_mem);
	int count_backup = count;	
	while (count_backup > 0) {
		for (int k=0; k<count; k++)
        		for (int i = 0; i <= info.m_machine.m_cpu; i++)
               			for (int j = 0; j <= info.m_machine.m_mem; j++)
                        		utility[k][i][j] = 0;
		//printf("%d %d %d %d %d %d\n", arrange[0], arrange[1], arrange[2], arrange[3], arrange[4], arrange[5]);
		// find the max utility at present
		for (int i = count-1; i>=0; i--) {
			if (arrange[i]) continue;
			for (int j = count; j>i; j--)
				for (int k = vm_one[i].m_cpu; k <= info.m_machine.m_cpu; k++)
					for (int l = vm_one[i].m_mem; l <= info.m_machine.m_mem; l++)
						if (utility[j][k-vm_one[i].m_cpu][l-vm_one[i].m_mem]+1.0*vm_one[i].m_cpu/info.m_machine.m_cpu+1.0*vm_one[i].m_mem/info.m_machine.m_mem > utility[i][k][l]) {
				utility[i][k][l] = utility[j][k-vm_one[i].m_cpu][l-vm_one[i].m_mem]+1.0*vm_one[i].m_cpu/info.m_machine.m_cpu+1.0*vm_one[i].m_mem/info.m_machine.m_mem;
				next[i][k][l] = j;
			}
		}
		int index = 0;
		double maxutility = 0;
		for (int i = 0; i < count; i++) {
		//printf("%d %f %d\n", i, utility[i][info.m_machine.m_cpu][info.m_machine.m_mem], next[i][info.m_machine.m_cpu][info.m_machine.m_mem]);
		if (arrange[i]==0 && utility[i][info.m_machine.m_cpu][info.m_machine.m_mem] > maxutility) {
				maxutility = utility[i][info.m_machine.m_cpu][info.m_machine.m_mem]; 
				index = i;
			}
		}
		// delete the selected vm_one, apply for new physical machine
		int cpu_present = info.m_machine.m_cpu;
		int mem_present = info.m_machine.m_mem;
		while (index < count) {
			//printf("index = %d\n", index);
			arrange[index] = 1;
			if(r[m_cnt].m_numOfPair==0 || r[m_cnt].m_pair[r[m_cnt].m_numOfPair-1].m_id != vm_one[index].m_id) {
				r[m_cnt].m_numOfPair++;
				r[m_cnt].m_pair[r[m_cnt].m_numOfPair-1].m_id = vm_one[index].m_id;
			}
                        r[m_cnt].m_pair[r[m_cnt].m_numOfPair-1].m_numOfVM++;
			int tmp = index;
			index = next[index][cpu_present][mem_present];
			cpu_present -= vm_one[tmp].m_cpu;
			mem_present -= vm_one[tmp].m_mem;
			count_backup--;
		}
		m_cnt++;
	
	}
	//printf("count = %d, cnt = %d\n", count, cnt);
	//printf("cpu:%d mem:%d\n",info.m_machine.m_cpu, info.m_machine.m_mem);
	/*for ( int i = 0; i < info.m_numOfVM; i++){
		if (vm[i] > 0){
			int n = vm[i];
			int vm_mem = 0, vm_cpu = 0;
			//for (int j = 0; j < info.m_numOfVM; j++){
			//	if( i == info.m_vm[j]->m_id){
			vm_mem = info.m_vm[i]->m_mem;
			vm_cpu = info.m_vm[i]->m_cpu;					
				//}
			//}
			//printf("vm_mem:%d vm_cpu:%d\n", vm_mem, vm_cpu);
			for(int j = 0; j < n;){
				if (mem_cnt + vm_mem <= info.m_machine.m_mem * 1024 && cpu_cnt + vm_cpu <= info.m_machine.m_cpu){
					mem_cnt += vm_mem;
					cpu_cnt += vm_cpu;		
					//printf("memcnt:%d cputcnt:%d\n", mem_cnt, cpu_cnt);					
					r[m_cnt].m_pair[r[m_cnt].m_numOfPair].m_id = info.m_vm[i]->m_id;
					r[m_cnt].m_pair[r[m_cnt].m_numOfPair].m_numOfVM++;
					j++;
				}else{			
					if(r[m_cnt].m_pair[r[m_cnt].m_numOfPair].m_numOfVM)	r[m_cnt].m_numOfPair++;							
					m_cnt++;
					mem_cnt = 0;
					cpu_cnt = 0;					
				}
			}
			r[m_cnt].m_numOfPair++;
		}
	}
	*/	
	numOfMachine = m_cnt;
	
	return r;
	
}

void print_placement(char * buffer, int numOfMachine, ResultLine * r){	
	int offset = strlen(buffer);
	//printf("len_part:%d\n",offset);
	//printf("%s\n", buffer);
	offset += sprintf(buffer + offset, "%d\n", numOfMachine);
	//printf("%d\n", numOfMachine);
	for(int i = 0; i < numOfMachine; i++){
		offset += sprintf(buffer + offset, "%d", i+1);		
		for(int j = 0; j < r[i].m_numOfPair; j++){
			//printf("%d\n",r[i].m_numOfPair);
			offset += sprintf(buffer + offset," flavor%d %d", r[i].m_pair[j].m_id, r[i].m_pair[j].m_numOfVM);
			//printf("%d %d\n",r[i].m_pair[j].m_id, r[i].m_pair[j].m_numOfVM);
		}
		offset += sprintf(buffer + offset, "\n");
	}
	//printf("%s", buffer);
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

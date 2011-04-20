#ifndef __TASKS
#define __TASKS

#include "defs.h"

typedef struct taskgrouptemp {
	void (*func)(int taskIndex, void *data, void **result, int *resultSize); 
	unsigned int size;
	unsigned finished;
} TaskGroup;

typedef struct tasktemp {
	void *data;
	void *result;
	int resultSize;
	unsigned char finished;
	unsigned short taskgroup;
	int *depTask;
	unsigned short depTaskSize, depTaskNum;
	unsigned short *depGroup;
	unsigned short depGroupSize, depGroupNum;
} Task;

typedef struct tasksstemp {
	Task *tasks;
	int sizeTasks, numTasks;
	TaskGroup *groups;
	unsigned short sizeGroups, numGroups;
} Tasks;

void tasksFromNative(Tasks* t, unsigned short sizeGroups, int sizeTasks);
void taskGroupFromNative(TaskGroup* tg, void (*func)(int taskIndex, void *data, void **result, int *resultSize));
void taskFromNative(Task* t, void *data, unsigned short taskgroup, unsigned short depTaskSize, unsigned short depGroupSize);

unsigned short tasksAddGroup(Tasks* t, void (*func)(int taskIndex, void *data, void **result, int *resultSize)); 
int tasksAddTask(Tasks* t, void *data, unsigned short taskgroup, unsigned short depTaskSize, unsigned short depGroupSize); 
void tasksAddTaskDep(Tasks* t, int taskIndex, int taskDep);
void tasksAddGroupDep(Tasks* t, int taskIndex, unsigned short groupDep);
void tasksRun(Tasks* t, int taskIndex);
void tasksRunMany(Tasks* t, int *taskIndexes, int numTasks);
int tasksGetGroupMembers(Tasks* t, unsigned short groupIndex, int**recv);
int tasksIsTaskFinished(Tasks* t, int taskIndex);
int tasksIsGroupFinished(Tasks* t, unsigned short groupIndex);
int tasksIsReady(Tasks* t, int taskIndex);

void tasksFree(Tasks* t);
void taskGroupFree(TaskGroup* tg);
void taskFree(Task* t);

//void cachePut_box(Cache *c, abstractPointer p, void* send, unsigned flags);
//void cacheGet_box(Cache *c, abstractPointer p, void** recv, unsigned flags);
//void cacheFlush_box(Cache *c);
//void cacheClear_box(Cache *c);

//void cacheFence(Cache *c);

#endif

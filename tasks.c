#include "tasks.h"

void tasksFromNative(Tasks* t, unsigned short sizeGroups, int sizeTasks)
{
	assert(t);
	assert(sizeGroups);
	assert(sizeTasks>0);
	
	t->sizeTasks=sizeTasks;
	t->sizeGroups=sizeGroups;
	t->numTasks=0;
	t->numGroups=0;
	t->tasks=malloc(sizeof(Task) * sizeTasks);
	t->groups=malloc(sizeof(TaskGroup) * sizeTasks);
	
}

void taskGroupFromNative(TaskGroup* tg, void (*func)(int taskIndex, void *data, void **result, int *resultSize))
{
	assert(tg);
	assert(func);
	
	tg->func=func;
	tg->size=0;
	tg->finished=0;
	
}

void taskFromNative(Task* t, void *data, unsigned short taskgroup, unsigned short depTaskSize, unsigned short depGroupSize)
{
	assert(t);
	//assert(taskgroup);
	
	t->data=data;
	t->taskgroup=taskgroup;
	
	t->depTaskSize=depTaskSize;
	if(t->depTaskSize)
		t->depTask=malloc(t->depTaskSize * sizeof(int));
	else
		t->depTask=NULL;
	
	t->depGroupSize=depGroupSize;
	if(t->depGroupSize)
		t->depGroup=malloc(t->depGroupSize * sizeof(unsigned short));
	else
		t->depGroup=NULL;
	
	t->depTaskNum=0;
	t->depGroupNum=0;
	t->result=NULL;
	t->resultSize=0;
	t->finished=0;
}

unsigned short tasksAddGroup(Tasks* t, void (*func)(int taskIndex, void *data, void **result, int *resultSize))
{
	assert(t);
	assert(func);
	assert(t->numGroups<t->sizeGroups);
	
	unsigned short res = t->numGroups;
	(t->numGroups)++;
	taskGroupFromNative(&(t->groups[res]), func);
	
	return res;
}

int tasksAddTask(Tasks* t, void *data, unsigned short taskgroup, unsigned short depTaskSize, unsigned short depGroupSize)
{
	assert(t);
	assert(t->numTasks<t->sizeTasks);
	
	int res = t->numTasks;
	(t->numTasks)++;
	taskFromNative(&(t->tasks[res]), data, taskgroup, depTaskSize, depGroupSize);
	(t->groups[taskgroup].size)++;
	
	return res;
}

void tasksAddTaskDep(Tasks* t, int taskIndex, int taskDep)
{
	assert(t);
	assert(taskIndex<t->numTasks);
	assert(taskDep<t->numTasks);
	
	Task* task=&(t->tasks[taskIndex]);
	assert(task->depTaskNum < task->depTaskSize);
	
	unsigned short temp=task->depTaskNum;
	(task->depTaskNum)++;
	
	task->depTask[temp]=taskDep;
	
}

void tasksAddGroupDep(Tasks* t, int taskIndex, unsigned short groupDep)
{
	assert(t);
	assert(taskIndex<t->numTasks);
	assert(groupDep<t->numGroups);
	
	Task* task=&(t->tasks[taskIndex]);
	assert(task->depGroupNum < task->depGroupSize);
	
	unsigned short temp=task->depGroupNum;
	(task->depGroupNum)++;
	
	task->depGroup[temp]=groupDep;
	
}

void tasksRun(Tasks* t, int taskIndex)
{
	assert(t);
	assert(taskIndex<t->numTasks);
	
	if(tasksIsTaskFinished(t,taskIndex))
		return;
	
	void (*func)(int taskIndex, void *data, void **result, int *resultSize);
	func = t->groups[t->tasks[taskIndex].taskgroup].func;
	
	//no dep check
	
	int i;
	for(i=0;i<t->tasks[taskIndex].depTaskNum; i++)
		tasksRun(t,t->tasks[taskIndex].depTask[i]);
	
	int *groupdep;
	for(i=0;i<t->tasks[taskIndex].depGroupNum; i++)
	{
		int recvSize = tasksGetGroupMembers(t, t->tasks[taskIndex].depGroup[i], &groupdep);
		tasksRunMany(t,groupdep, recvSize);
		free(groupdep);
	}
	
	func(taskIndex, t->tasks[taskIndex].data, &(t->tasks[taskIndex].result), &(t->tasks[taskIndex].resultSize));
	
	(t->groups[t->tasks[taskIndex].taskgroup].finished)++;
	t->tasks[taskIndex].finished=1;
	//printf("tasksRun taskIndex=%d\n", taskIndex);
}

void tasksRunMany(Tasks* t, int *taskIndexes, int numTasks)
{
	assert(t);
	assert(taskIndexes);
	assert(numTasks>0);
	
	//mb parall
	
	int i;
	for(i=0; i<numTasks; i++)
		tasksRun(t,taskIndexes[i]);
	
}

int tasksGetGroupMembers(Tasks* t, unsigned short groupIndex, int**recv)
{
	assert(t);
	assert(groupIndex<t->numGroups);
	assert(recv);
	
	int size= t->groups[groupIndex].size;
	int i,j;
	*recv = malloc(sizeof(int) * size);
	
	j=0;
	for(i=0;i<t->numTasks;i++)
		if(t->tasks[i].taskgroup == groupIndex)
		{
			(*recv)[j]=i;
			j++;
		}
	
	return size;
}

int tasksIsTaskFinished(Tasks* t, int taskIndex)
{
	assert(t);
	assert(taskIndex<t->numTasks);
	
	return t->tasks[taskIndex].finished;
}

int tasksIsGroupFinished(Tasks* t, unsigned short groupIndex)
{
	assert(t);
	assert(groupIndex<t->numGroups);
	
	return (t->groups[groupIndex].finished == t->groups[groupIndex].size);
}

int tasksIsReady(Tasks* t, int taskIndex)
{
	assert(t);
	assert(taskIndex<t->numTasks);
	
	int i;
	for(i=0;i<t->tasks[taskIndex].depGroupNum; i++)
		if(!tasksIsGroupFinished(t, t->tasks[taskIndex].depGroup[i]))
			return 0;
		
	for(i=0;i<t->tasks[taskIndex].depTaskNum; i++)
		if(!tasksIsTaskFinished(t, t->tasks[taskIndex].depTask[i]))
			return 0;
		
	return 1;
}

void tasksFree(Tasks* t)
{
	assert(t);
	
	int i;
	
	for(i=0;i<t->numTasks; i++)
		taskFree(&(t->tasks[i]));
	for(i=0;i<t->numGroups; i++)
		taskGroupFree(&(t->groups[i]));
	
	free(t->tasks);
	free(t->groups);
	t->tasks=NULL;
	t->groups=NULL;
	t->sizeTasks=-1;
	t->sizeGroups=0;
	t->numTasks=-1;
	t->numGroups=0;
	
}

void taskGroupFree(TaskGroup* tg)
{
	assert(tg);
	
	tg->func=NULL;
	tg->size=0;
	tg->finished=0;
}

void taskFree(Task* t)
{
	assert(t);
	
	if(t->data)
	{
		free(t->data);
		t->data=NULL;
	}
	
	t->taskgroup=0;
	t->depTaskSize=0;
	if(t->depTask)
	{
		free(t->depTask);
		t->depTask=NULL;
	}
	
	t->depGroupSize=0;
	if(t->depGroup)
	{
		free(t->depGroup);
		t->depGroup=NULL;
	}
	
	if(t->result)
	{
		free(t->result);
		t->result=NULL;
	}
	
	t->resultSize=0;
	t->finished=0;
}

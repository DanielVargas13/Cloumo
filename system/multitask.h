/*
 * マルチタスク
 */

#ifndef _MULTITASK_H_
#define _MULTITASK_H_

const int MAX_TASKS = 1000;
const int kTaskGdt0 = 3;
const int kMaxTasksLevel = 100;
const int MAX_TASKLEVELS = 10;

struct TSS32 {
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	int es, cs, ss, ds, fs, gs;
	int ldtr, iomap;
};

class Task {
public:
	char* name_;
	int sel_, flags_;
	int level_, priority_;
	Queue queue_;
	TSS32 tss_;
	//int fpu[108 / 4];
	//int stack;

public:
	void run(int, int);
	void sleep();
};

struct TaskLevel {
	int running;
	int now;
	Task* tasks[kMaxTasksLevel];
};

class TaskController {
public:
	static int now_lv_;
	static char lv_change_;
	static TaskLevel* level_;
	static Task* tasks0_;

public:
	//static Task *task_fpu_;
	static Timer* timer_;

public:
	static Task* init();
	static Task* alloc();
	static void switchTask();
	static void switchTaskSub();
	static Task* getNowTask();
	static void add(Task*);
	static void remove(Task*);
	static void idleLoop();
};

#endif
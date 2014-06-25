#include "headers.h"

void Task::run(int level, int priority) {
	if (level < 0) level = level_;
	if (priority > 0) priority_ = priority;
	
	if (flags_ == 2 && level_ != level) {
		TaskController::remove(this);
	}
	if (flags_ != 2) {
		level_ = level;
		TaskController::add(this);
	}
	
	TaskController::lv_change_ = 1;
}

void Task::sleep() {
	if (flags_ == 2) {
		Task* now_task = TaskController::getNowTask();
		TaskController::remove(this);
		if (this == now_task) {
			TaskController::switchTaskSub();
			now_task = TaskController::getNowTask();
			FarJump(0, now_task->sel_);
		}
	}
}

Timer*     TaskController::timer_     = 0;
int        TaskController::now_lv_    = 0;
char       TaskController::lv_change_ = 0;
TaskLevel* TaskController::level_     = 0;
Task*      TaskController::tasks0_    = 0;

Task* TaskController::init() {
	level_ = new TaskLevel[MAX_TASKLEVELS];
	tasks0_ = (Task*)malloc4k(MAX_TASKS * sizeof(Task));//new Task[kMaxTasks];
	for (int i = 0; i < MAX_TASKS; i++) {
		tasks0_[i].flags_ = 0;
		tasks0_[i].sel_ = (kTaskGdt0 + i) * 8;
		SetSegmentDescriptor((SegmentDescriptor*)kAdrGdt + kTaskGdt0 + i, 103, (int)&tasks0_[i].tss_, kArTss32);
	}
	for (int i = 0; i < MAX_TASKLEVELS; i++) {
		level_[i].running = 0;
		level_[i].now = 0;
	}

	/* メインタスク */
	Task* task = alloc();
	task->name_ = (char*)kMainTaskName;
	task->flags_ = 2; /* 動作中マーク */
	task->priority_ = 2; /* 0.02秒 */
	task->level_ = 0;
	add(task);
	switchTaskSub();
	LoadTr(task->sel_);
	timer_ = TimerController::alloc();
	timer_->set(task->priority_);

	/* アイドルタスク */
	Task *idle = alloc();
	idle->name_ = "Idle";
	idle->tss_.esp = (unsigned int)malloc4k(64 * 1024) + 64 * 1024;
	idle->tss_.eip = (int)&idleLoop;
	idle->tss_.es = 1 * 8;
	idle->tss_.cs = 2 * 8;
	idle->tss_.ss = 1 * 8;
	idle->tss_.ds = 1 * 8;
	idle->tss_.fs = 1 * 8;
	idle->tss_.gs = 1 * 8;
	idle->run(MAX_TASKLEVELS - 1, 1);

	//task_fpu_ = null;

	return task;
}

Task* TaskController::alloc() {
	for (int i = 0; i < MAX_TASKS; i++) {
		if (!tasks0_[i].flags_) {
			Task *task = &tasks0_[i];
			task->flags_ = 1;
			task->tss_.eflags = 0x00000202;
			task->tss_.eax = 0;
			task->tss_.ecx = 0;
			task->tss_.edx = 0;
			task->tss_.ebx = 0;
			task->tss_.ebp = 0;
			task->tss_.esi = 0;
			task->tss_.edi = 0;
			task->tss_.es = 0;
			task->tss_.ds = 0;
			task->tss_.fs = 0;
			task->tss_.gs = 0;
			task->tss_.ldtr = 0;
			task->tss_.iomap = 0x40000000;
			task->tss_.ss0 = 0;
			//task->fpu[0] = 0x037f; /* CW(control word) */
			//task->fpu[1] = 0x0000; /* SW(status word)  */
			//task->fpu[2] = 0xffff; /* TW(tag word)     */
			//task->fpu[3..108/4-1] = 0;
			return task;
		}
	}
	// もうタスクは作れない
	return 0;
}

void TaskController::switchTask() {
	TaskLevel *tl = &level_[now_lv_];
	Task *new_task;
	Task *now_task = tl->tasks[tl->now];
	tl->now++;
	if (tl->now == tl->running) tl->now = 0;
	if (lv_change_) {
		switchTaskSub();
		tl = &level_[now_lv_];
	}
	new_task = tl->tasks[tl->now];
	timer_->set(new_task->priority_);
	if (new_task != now_task) FarJump(0, new_task->sel_);
}

void TaskController::switchTaskSub() {
	int i;
	for (i = 0; i < MAX_TASKLEVELS; i++) {
		if (level_[i].running > 0) break;
	}
	now_lv_ = i;
	lv_change_ = 0;
}

Task *TaskController::getNowTask() {
	TaskLevel *tl = &level_[now_lv_];
	return tl->tasks[tl->now];
}

void TaskController::add(Task *task) {
	TaskLevel *tl = &level_[task->level_];
	tl->tasks[tl->running] = task;
	tl->running++;
	task->flags_ = 2;
}

void TaskController::remove(Task *task) {
	int i;
	TaskLevel* tl = &level_[task->level_];

	for (i = 0; i < tl->running; i++)
		if (tl->tasks[i] == task)
			break;

	tl->running--;
	if (i < tl->now) tl->now--;
	if (tl->now >= tl->running) tl->now = 0;
	task->flags_ = 1;

	for (; i < tl->running; i++) {
		tl->tasks[i] = tl->tasks[i + 1];
	}
}

void TaskController::idleLoop() {
	for (;;) {
		Hlt();
	}
}

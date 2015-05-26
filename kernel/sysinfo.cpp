#include <stdio.h>
#include <pistring.h>
#include "../headers.h"

void showSysInfo(int benchScore) {
	string str;
	auto memTotal = MemoryTotal();
	
	// Clear the screen
	Rectangle clearRange(Point(2, 2), Size(SheetCtl::window[0]->frame.size.width - 3, SheetCtl::window[0]->frame.size.height - 3));
	SheetCtl::window[0]->fillRect(clearRange, 0xffffff);
	
	// Benchmark Result
	str = to_string(benchScore);
	SheetCtl::window[0]->drawString("Benchmark Score:", Point(2, 2), 0);
	SheetCtl::window[0]->drawString(str, Point(2 + 8 * 17, 2), 0);
	
	// Memory Information
	str = "RAM: " + to_string(MemoryTest(0x00400000, 0xbfffffff) / 1024 / 1024) + " MB    FREE: " + to_string(memTotal / 1024 / 1024) + " MB (" + to_string(memTotal) + " Byte)";
	SheetCtl::window[0]->drawString(str, Point(2, 2 + 16), 0);
	
	// Display Information
	str = "Resoultion: " + to_string(SheetCtl::resolution.width) + " x " + to_string(SheetCtl::resolution.height) + " (" + to_string(SheetCtl::color) + "-bit color)";
	SheetCtl::window[0]->drawString(str, Point(2, 2 + 16 * 2), 0);
	
	// Task List
	SheetCtl::window[0]->drawString("level priority flag task name", Point(2 + 1, 2 + 16 * 4 + 1), 0);
	int j = 0;
	char s[20];
	for (auto &&level : TaskSwitcher::level) {
		for (int i = 0; i < level.running; ++i) {
			Task &task = *level.tasks[i];
			if (task.running) {
				sprintf(s, "%5d %8d %4s ", task.level, task.priority, task.running ? "(oo)" : "(__)");
				str = string(s) + task.name;
				SheetCtl::window[0]->drawString(str, Point(2 + 1, 2 + 16 * 5 + j * 16 + 2), 0);
				++j;
			}
		}
	}
	SheetCtl::window[0]->drawRect(Rectangle(2, 2 + 16 * 4, SheetCtl::window[0]->frame.size.width - 1 - 1 - 2, 16 + j * 16 + 3), 0);
	SheetCtl::window[0]->drawLine(Line(3, 2 + 16 * 5 + 1, SheetCtl::window[0]->frame.size.width - 1 - 2, 2 + 16 * 5 + 1), 0);
	SheetCtl::window[0]->drawLine(Line(3 + 5 * 8 + 3, 2 + 16 * 4 + 1, 3 + 5 * 8 + 3, 2 + 16 * 5 + j * 16 + 2), 0);
	SheetCtl::window[0]->drawLine(Line(3 + 14 * 8 + 3, 2 + 16 * 4 + 1, 3 + 14 * 8 + 3, 2 + 16 * 5 + j * 16 + 2), 0);
	SheetCtl::window[0]->drawLine(Line(3 + 19 * 8 + 3, 2 + 16 * 4 + 1, 3 + 19 * 8 + 3, 2 + 16 * 5 + j * 16 + 2), 0);
	
	// Refresh the screen
	SheetCtl::window[0]->refresh(clearRange);
}

void SysinfoMain() {
	Task *task = TaskSwitcher::getNowTask();
	int count = 0, count0 = 0;
	
	Timer *timer = new Timer(task->queue);
	timer->set(100);
	
	showSysInfo(0);
	
	for (;;) {
		++count;
		Cli();
		if (task->queue->isempty()) {
			//task->sleep(); ベンチマーク測定のため
			Sti();
		} else {
			int data = task->queue->pop();
			Sti();
			if (data == timer->data) {
				showSysInfo(count - count0);
				count0 = count;
				timer->set(100);
			}
		}
	}
}

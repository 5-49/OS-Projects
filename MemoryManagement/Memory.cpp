#include<iostream>
#include<cstring>
#include<iomanip>
#include<queue>
#include<vector>
#include<random>
#include <stdlib.h>
#include <time.h> 
using namespace std;

const int frameNum = 4;
const int insNum = 320;
int alg;//1代表FIFO，2代表LRU
int missing = 0;//
int curIns = 0;//当前执行指令（0-319）
int dir = 1;//指令跳转方向
int cnt = 1;//记录执行次数
int oldPage;//记录要被替换掉的页面
queue<int>fifoQueue;
queue<int>lruQueue;
vector<int>frame;

//获取下一条指令
int getIns() {
	int nextIns;
	
	if (dir == 1) {
		while (1) {
			nextIns = curIns + 1;
			if (nextIns > 319) {
				nextIns = nextIns-319;
			}
			else break;
		}
	}

	else if(dir == 2) {
		while (1) {//向前跳转，前一步是顺序执行
			srand((unsigned)time(NULL));
			if (curIns == 2) {
				dir++;
				nextIns = getIns();
			}
			nextIns = rand() % (curIns - 2);
			if (nextIns > 319) {
				nextIns = nextIns - 319;
			}
			else break;
		}
	}
	else if (dir == 3) {
		while (1) {	
			nextIns = curIns + 1;
			if (nextIns > 319) {
				nextIns = nextIns - 319;
			}
			else break;
		}
	}
	else if (dir == 4) {
		while (1) {//向后跳转，前一步是顺序执行
			srand((unsigned)time(NULL));
			nextIns = rand() % (319 - curIns) + curIns + 1;
			if (nextIns > 319) {
				nextIns = nextIns - 319;
			}
			else break;
		}
	}


	if (dir == 4)dir = 1;
	else dir++;

	return nextIns;
}

//判断进行该页是否在内存中
bool isIn(int page) {
	if (frame.size() == 0) return false;
	for (int i = 0; i < frame.size(); i++) {
		if (frame[i] == page) return true;
	}
	return false;
}

void display() {
		cout << setw(10) << "此步骤执行后内存中存储页面为:";
		for (vector<int>::iterator m = frame.begin(); m != frame.end(); m++)  
		{ 
			cout << *m << " " ; 
		}
		cout << endl << endl;
}

//先进先出
void FIFO() {
	//初始化
	srand((unsigned)time(NULL));
	curIns = rand() % 319;
	cnt = 1;
	while (cnt <= 320) {
		if (isIn(curIns / 10)) {//页面已经在内存
			cout << cnt << "次：" << curIns << "指令已经在内存中" << endl;
			display();
		}
		else  {//页面不在内存
			missing++;
			if (frame.size() == 4) {//内存已满
				oldPage = fifoQueue.front();
				fifoQueue.pop();
				fifoQueue.push(curIns / 10);
				cout << cnt << "次：" << curIns << "不再内存中，请求调出" << oldPage << "页" << endl;
				vector<int>::iterator ele = find(frame.begin(), frame.end(), oldPage);
				*ele = curIns / 10;
				display();
			}
			else {//内存还没满 
				cout << cnt << "次：" << curIns << "不再内存中，内存未满，直接加入内存" << endl;
				frame.push_back(curIns / 10);
				fifoQueue.push(curIns / 10);
				display();
			}
		}

		curIns = getIns();
		cnt++;
	}
}

//最近最久未访问
void LRU() {
	//初始化
	srand((unsigned)time(NULL));
	curIns = rand() % 319;
	cnt = 1;
	int time[32] = { 0 };//记录各个页面的访问时间，越小代表越久
	int t = 0;

	while (cnt <= 320) {
		lruQueue.push(curIns / 10);
		time[curIns / 10] = ++t;
		if (isIn(curIns / 10)) {//页面已经在内存
			cout << cnt << "次：" << curIns << "指令已经在内存中" << endl;
			display();
		}
		else {//页面不在内存
			missing++;
			if (frame.size() == 4) {//内存已满
				int oldPage=frame[0];
				//找time[frame[0-3]]的最小值
				for (int i = 0; i < 4; i++) {
					if (time[frame[i]] < time[oldPage]) oldPage = frame[i];
				}
				cout << cnt << "次：" << curIns << "不再内存中，请求调出" << oldPage << "页" << endl;
				vector<int>::iterator ele = find(frame.begin(), frame.end(), oldPage);
				*ele = curIns / 10;
				display();
			}
			else {//内存还没满 
				cout << cnt << "次：" << curIns << "不再内存中，内存未满，直接加入内存" << endl;
				frame.push_back(curIns / 10);
				display();
			}
		}
		curIns = getIns();
		cnt++;
	}
}

int main() {
	cout << "请选择页面置换算法，1代表FIFO，2代表LRU" << endl;
	cin >> alg;
	if (alg == 1) FIFO();
	else if(alg == 2) LRU();

	double miss = double(missing)  / 320;

	cout << "缺页次数为：" << missing << endl;
	cout << "缺页率为：" << setprecision(4) << miss << endl;

	system("pause");
	return 0;
}

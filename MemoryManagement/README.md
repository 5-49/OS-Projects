# 项目要求

假设每个页面可存放10条指令，分配给一个作业的内存块为4。模拟一个作业的执行过程，该作业有320条指令，即它的地址空间为32页，目前所有页还没有调入内存。

模拟过程：如果所访问指令在内存中，则显示其物理地址，并转到下一条指令；如果没有在内存中，则发生缺页，此时需要记录缺页次数，并将其调入内存。如果4个内存块中已装入作业，则需进行页面置换。所有320条指令执行完成后，计算并显示作业执行过程中发生的缺页率。

置换算法可以选用FIFO或者LRU算法

作业中指令访问次序可以按照下面原则形成：50%的指令是顺序执行的，25%是均匀分布在前地址部分，25％是均匀分布在后地址部分

# 调页算法

## 先进先出（FIFO）算法

### 算法描述：

每次选择淘汰界面将是以后永不使用，或者在最长时间内不再被访问的界面，这样可以保证最低的缺页率。

### 算法实现：

设置fifoQueue（大小为4），每次缺页发生时，如果队列未满，直接入队，如果队列满了，则弹出队首元素，再将页面纳入队列。同时在内存vector中寻找队首元素页面替换为当前界面。

```c++
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
```

## 最近最久使用（LRU）算法

### 算法描述：

每次淘汰的页面是最近最久未使用的界面。

### 算法实现：

对32个页面创建记录访问时间的数组time[32]，取值范围为1-320，用来记录此页面上一次被访问是什么时候。每次缺页发生时，如果内存未满，直接进入内存，如果内存已满，寻找time值最小的页面，将其替换。

```c++
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
```


# 其他函数说明：

## 指令序列生成函数

返回值为下一个指令的值（0-319），此函数保证了指令跳转50%为顺序跳转，25%为前序跳转，25%为后序跳转。

```c++
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
```

## 判断指令是否在内存中的函数

此函数返回一个布尔变量，用于表示存储当前指令的页面是否在内存中

```c++
//判断进行该页是否在内存中
bool isIn(int page) {
	if (frame.size() == 0) return false;
	for (int i = 0; i < frame.size(); i++) {
		if (frame[i] == page) return true;
	}
	return false;
}
```


## 打印当前内存情况的函数

此函数为打印函数，用于输出执行本次指令后内存中存储的页面。

```c++
void display() {
		cout << setw(10) << "此步骤执行后内存中存储页面为:";
		for (vector<int>::iterator m = frame.begin(); m != frame.end(); m++)  
		{ 
			cout << *m << " " ; 
		}
		cout << endl << endl;
}
```

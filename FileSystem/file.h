#include <iostream>
#include <string>
#include <vector>
using namespace std;


//文件分配表
class FatBlock {
private:
	bool use;
	int lengthInUse;
	int nextBlock;
public:
	FatBlock();
	bool getUse() { return use; }
	int getLengthInUse() { return lengthInUse; }
	int getNextBlock() { return nextBlock; }
	void setUse(bool u) { use = u; }
	void setLengthInUse(int l) { lengthInUse = l; }
	void setNextBlock(int n) { nextBlock = n; }
};

//总文件分配表（与内存块一一对应）
class FatTable {
public:
	FatBlock fatBlockList[1024];
	FatTable();
};

//内存块
class Block {
public:
	char content[1024];
	Block();
};

//内存（与FAT一一对应）
class Memory {
public:
	Block * blockList;
	Memory();
	~Memory();
};

//文件控制块
class FCB {
public:
	char name[60];
	int size;
	bool folderOrFile;
	int contentBlock;
	FCB();
	FCB(string name, int size, bool folderOrFile, int contentBlock);
};


//文件信息记录
class FileRecord {
public:
	char name[60];
	int FCBBlock;
	FileRecord();
	FileRecord(string name, int FCBBlock);
};

//文件夹信息记录
class FolderBlock {
public:
	FileRecord fileList[16];
	FolderBlock();
};

//文件系统（由内存和文件分配表组成）
class FileSystem {
private:
	FatTable fatTable;
	Memory memory;
	int currentFolder;
public:
	FileSystem();
	Memory & getMemory() { return memory; }
	void setMemory(Block * list) { memory.blockList = list; }
	bool checkName(string name);
	bool enoughBlock(int &blockForFCB, int &blockForContent);
	bool enoughFileBlock(int &blockForFCB, int &blockForContent);
	bool enoughInBlock(int &blockForRecord);
	bool createFolder(string foldName);
	bool createFile(string fileName);
	void dir();
	bool cd(string path);
	bool del(string fileName);
	bool deltree(string folderName);
	bool type(string fileName);
	bool edit(string fileName);
	void instru();
	string getCurrentPath();
};

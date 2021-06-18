#include "File.h"
#include <iostream>
#include <string>
#include <vector>
using namespace std;


FatBlock::FatBlock()
{
	this->use = 0;
	this->lengthInUse = 0;
	this->nextBlock = -1;
}

FatTable::FatTable()
{
}

Block::Block()
{
}

Memory::Memory()
{
	blockList = new Block[1024];

}

Memory::~Memory()
{
	delete[]blockList;
}

FCB::FCB()
{
	//	this->name = "";
	this->size = 0;
	this->folderOrFile = true;
	this->contentBlock = -1;
}

FCB::FCB(string name, int size, bool folderOrFile, int contentBlock)
{
	int i = 0;
	for ( i = 0; i < name.size(); i++)
		this->name[i] = name.at(i);
	this->name[i] = '\0';
	this->size = size;
	this->folderOrFile = folderOrFile;
	this->contentBlock = contentBlock;
}

FileRecord::FileRecord()
{
	//	this->name = "";
	this->FCBBlock = -1;
}

FileRecord::FileRecord(string name, int FCBBlock)
{
	int i = 0;
	for ( i = 0; i < name.size(); i++)
		this->name[i] = name.at(i);
	this->name[i] = '\0';
	this->FCBBlock = FCBBlock;
}

FolderBlock::FolderBlock()
{
}

FileSystem::FileSystem()
{
	FCB fcb("FileSystem:", 0, true, 1);
	::memcpy(this->memory.blockList[0].content, &fcb, sizeof(FCB));
	FileRecord fileRecord("FileSystem:", 0);
	::memcpy(this->memory.blockList[1].content, &fileRecord, sizeof(FileRecord));

	this->fatTable.fatBlockList[0].setUse(true);
	this->fatTable.fatBlockList[0].setLengthInUse(1024);
	this->fatTable.fatBlockList[0].setNextBlock(-1);

	this->fatTable.fatBlockList[1].setUse(true);
	this->fatTable.fatBlockList[1].setLengthInUse(sizeof(FileRecord));
	this->fatTable.fatBlockList[1].setNextBlock(-1);

	this->currentFolder = 0;
}

bool FileSystem::checkName(string name)
{
	FCB fcb;
	::memcpy(&fcb, this->memory.blockList[this->currentFolder].content, sizeof(FCB));
	int blockIndex = fcb.contentBlock;

	for (int i = 1; i < this->fatTable.fatBlockList[blockIndex].getLengthInUse() / 64; i++)
	{
		FileRecord fileRecord;
		::memcpy(&fileRecord, &(this->memory.blockList[blockIndex].content[i * 64]), sizeof(FileRecord));
		if (fileRecord.FCBBlock == -1)
			continue;
		if (fileRecord.name == name)
			return false;
	}
	while (this->fatTable.fatBlockList[blockIndex].getNextBlock() != -1)
	{
		blockIndex = this->fatTable.fatBlockList[blockIndex].getNextBlock();
		for (int i = 0; i < this->fatTable.fatBlockList[blockIndex].getLengthInUse() / 64; i++)
		{
			FileRecord fileRecord;
			::memcpy(&fileRecord, &(this->memory.blockList[blockIndex].content[i * 64]), sizeof(FileRecord));
			if (fileRecord.FCBBlock == -1)
				continue;
			if (fileRecord.name == name)
				return false;
		}
	}
	return true;
}

bool FileSystem::enoughBlock(int &blockForFCB, int &blockForContent)
{
	int i = 0;
	for (i = 0; i < 1024; i++)
	{
		if (this->fatTable.fatBlockList[i].getUse() == false)
		{
			blockForFCB = i;
			break;
		}
	}
	i++;
	for (; i < 1024; i++)
	{
		if (this->fatTable.fatBlockList[i].getUse() == false)
		{
			blockForContent = i;
			break;
		}
	}
	if (i >= 1024)
		return false;
	this->fatTable.fatBlockList[blockForFCB].setUse(true);
	this->fatTable.fatBlockList[blockForFCB].setLengthInUse(1024);
	this->fatTable.fatBlockList[blockForFCB].setNextBlock(-1);

	this->fatTable.fatBlockList[blockForContent].setUse(true);
	this->fatTable.fatBlockList[blockForContent].setLengthInUse(64);
	this->fatTable.fatBlockList[blockForContent].setNextBlock(-1);
	return true;
}

bool FileSystem::enoughFileBlock(int &blockForFCB, int &blockForContent)
{
	int i = 0;
	for ( i = 0; i < 1024; i++)
	{
		if (this->fatTable.fatBlockList[i].getUse() == false)
		{
			blockForFCB = i;
			break;
		}
	}
	i++;
	for (; i < 1024; i++)
	{
		if (this->fatTable.fatBlockList[i].getUse() == false)
		{
			blockForContent = i;
			break;
		}
	}
	if (i >= 1024)
		return false;
	this->fatTable.fatBlockList[blockForFCB].setUse(true);
	this->fatTable.fatBlockList[blockForFCB].setLengthInUse(1024);
	this->fatTable.fatBlockList[blockForFCB].setNextBlock(-1);

	this->fatTable.fatBlockList[blockForContent].setUse(true);
	this->fatTable.fatBlockList[blockForContent].setLengthInUse(0);
	this->fatTable.fatBlockList[blockForContent].setNextBlock(-1);
	return true;
}

bool FileSystem::enoughInBlock(int &blockForRecord)
{
	FCB fcb;
	::memcpy(&fcb, this->memory.blockList[this->currentFolder].content, sizeof(FCB));
	int fatBlockIndex = fcb.contentBlock;
	while (this->fatTable.fatBlockList[fatBlockIndex].getNextBlock() != -1)
	{
		fatBlockIndex = this->fatTable.fatBlockList[fatBlockIndex].getNextBlock();
	}
	blockForRecord = fatBlockIndex;
	if (this->fatTable.fatBlockList[fatBlockIndex].getLengthInUse() == 1024)
	{
		int i = 0;
		for ( i = 0; i < 1024; i++)
		{
			if (this->fatTable.fatBlockList[i].getUse() == false)
				break;
		}
		if (i >= 1024)
		{
			return false;
		}
		else
		{
			this->fatTable.fatBlockList[i].setUse(true);
			this->fatTable.fatBlockList[i].setLengthInUse(0);
			this->fatTable.fatBlockList[i].setNextBlock(-1);
			this->fatTable.fatBlockList[blockForRecord].setNextBlock(i);
			blockForRecord = i;

		}
	}
	return true;
}

bool FileSystem::createFolder(string folderName)
{
	int blockForFCB;
	int blockForContent;
	int blockForRecord;

	if (checkName(folderName) == false)
	{
		cout << "此路经中有重名的文件,不能执行此操作!  如需查看基本操作,请输入help";
		return false;
	}
	if (enoughBlock(blockForFCB, blockForContent) == false)
	{
		cout << "空间不足,不能执行此操作!  如需查看基本操作,请输入help";
		return false;
	}
	if (enoughInBlock(blockForRecord) == false)
	{
		cout << "空间不足,不能执行此操作!  如需查看基本操作,请输入help";
		return false;
	}
	FCB fcb(folderName, 0, true, blockForContent);
	::memcpy(this->memory.blockList[blockForFCB].content, &fcb, sizeof(FCB));
	FCB upFolder;
	::memcpy(&upFolder, this->memory.blockList[this->currentFolder].content, sizeof(FCB));
	FileRecord fileRecord(upFolder.name, this->currentFolder);
	::memcpy(this->memory.blockList[blockForContent].content, &fileRecord, sizeof(FileRecord));
	FileRecord currentRecord(folderName, blockForFCB);
	int startMemory = this->fatTable.fatBlockList[blockForRecord].getLengthInUse();
	::memcpy(&(this->memory.blockList[blockForRecord].content[startMemory]), &currentRecord, sizeof(FileRecord));
	this->fatTable.fatBlockList[blockForRecord].setLengthInUse(startMemory + 64);
	return true;
}

bool FileSystem::createFile(string fileName)
{
	int blockForFCB;
	int blockForContent;
	int blockForRecord;

	if (checkName(fileName) == false)
	{
		cout << "此路经中有重名的文件,不能执行此操作!  如需查看基本操作,请输入help";
		return false;
	}
	if (enoughFileBlock(blockForFCB, blockForContent) == false)
	{
		cout << "空间不足,不能执行此操作!  如需查看基本操作,请输入help";
		return false;
	}
	if (enoughInBlock(blockForRecord) == false)
	{
		cout << "空间不足,不能执行此操作!  如需查看基本操作,请输入help";
		return false;
	}
	FCB fcb(fileName, 0, false, blockForContent);
	::memcpy(this->memory.blockList[blockForFCB].content, &fcb, sizeof(FCB));
	FileRecord currentRecord(fileName, blockForFCB);
	int startMemory = this->fatTable.fatBlockList[blockForRecord].getLengthInUse();
	::memcpy(&(this->memory.blockList[blockForRecord].content[startMemory]), &currentRecord, sizeof(FileRecord));
	this->fatTable.fatBlockList[blockForRecord].setLengthInUse(startMemory + 64);
	return true;
}

void FileSystem::dir()
{
	int files = 0;
	int folders = 0;
	int i = 0;
	vector<string> v;
	FCB fcb;
	::memcpy(&fcb, this->memory.blockList[this->currentFolder].content, sizeof(FCB));
	int blockIndex = fcb.contentBlock;

	for ( i = 1; i < this->fatTable.fatBlockList[blockIndex].getLengthInUse() / 64; i++)
	{
		FileRecord fileRecord;
		::memcpy(&fileRecord, &(this->memory.blockList[blockIndex].content[i * 64]), sizeof(FileRecord));
		if (fileRecord.FCBBlock == -1)
			continue;
		FCB aFCB;
		::memcpy(&aFCB, &this->memory.blockList[fileRecord.FCBBlock].content, sizeof(FCB));
		string file = "";
		if (aFCB.folderOrFile == true)
		{
			file = fileRecord.name;
			file = "<DIR>          " + file;
			folders++;
		}
		else
		{
			file = fileRecord.name;
			file = "               " + file;
			files++;
		}
		v.push_back(file);
	}
	while (this->fatTable.fatBlockList[blockIndex].getNextBlock() != -1)
	{

		blockIndex = this->fatTable.fatBlockList[blockIndex].getNextBlock();
		for (i = 0; i < this->fatTable.fatBlockList[blockIndex].getLengthInUse() / 64; i++)
		{
			FileRecord fileRecord;
			::memcpy(&fileRecord, &(this->memory.blockList[blockIndex].content[i * 64]), sizeof(FileRecord));
			if (fileRecord.FCBBlock == -1)
				continue;
			FCB aFCB;
			::memcpy(&aFCB, &this->memory.blockList[fileRecord.FCBBlock].content, sizeof(FCB));
			string file = "";
			if (aFCB.folderOrFile == true)
			{
				file = fileRecord.name;
				file = "<DIR>          " + file;
				folders++;
			}
			else
			{
				file = fileRecord.name;
				file = "               " + file;
				files++;
			}
			v.push_back(file);
		}
	}

	for (i = 0; i < v.size(); i++)
	{
		cout << v.at(i) << endl;
	}
	cout << "此目录下有" << folders << "个文件夹和" << files << "个文件.";
}

bool FileSystem::cd(string path)
{
	if (path == "..")
	{
		FCB fcb;
		::memcpy(&fcb, this->memory.blockList[this->currentFolder].content, sizeof(FCB));
		int blockIndex = fcb.contentBlock;
		FileRecord fileRecord;
		::memcpy(&fileRecord, this->memory.blockList[blockIndex].content, sizeof(FileRecord));
		this->currentFolder = fileRecord.FCBBlock;
		return true;
	}
	else
	{
		if (path.find("\\") > path.length())
		{
			FCB fcb;
			::memcpy(&fcb, this->memory.blockList[this->currentFolder].content, sizeof(FCB));
			int blockIndex = fcb.contentBlock;

			for (int i = 1; i < this->fatTable.fatBlockList[blockIndex].getLengthInUse() / 64; i++)
			{
				FileRecord fileRecord;
				::memcpy(&fileRecord, &(this->memory.blockList[blockIndex].content[i * 64]), sizeof(FileRecord));
				if (fileRecord.FCBBlock == -1)
					continue;
				if (fileRecord.name == path)
				{
					FCB aFCB;
					::memcpy(&aFCB, this->memory.blockList[fileRecord.FCBBlock].content, sizeof(FCB));
					if (aFCB.folderOrFile == true)
					{
						this->currentFolder = fileRecord.FCBBlock;
						return true;
					}
					else
					{
						cout << path + "是文件,不能执行此操作!  如需查看基本操作,请输入help";
						return false;
					}
				}
			}
			while (this->fatTable.fatBlockList[blockIndex].getNextBlock() != -1)
			{
				blockIndex = this->fatTable.fatBlockList[blockIndex].getNextBlock();
				for (int i = 0; i < this->fatTable.fatBlockList[blockIndex].getLengthInUse() / 64; i++)
				{
					FileRecord fileRecord;
					::memcpy(&fileRecord, &(this->memory.blockList[blockIndex].content[i * 64]), sizeof(FileRecord));
					if (fileRecord.FCBBlock == -1)
						continue;
					if (fileRecord.name == path)
					{
						FCB aFCB;
						::memcpy(&aFCB, this->memory.blockList[fileRecord.FCBBlock].content, sizeof(FCB));
						if (aFCB.folderOrFile == true)
						{
							this->currentFolder = fileRecord.FCBBlock;
							return true;
						}
						else
						{
							cout << path + "是文件,不能执行此操作!  如需查看基本操作,请输入help";
							return false;
						}
					}
				}
			}
			cout << "当前目录下没有文件夹" + path + ",  不能执行此操作!  如需查看基本操作,请输入help";
			return false;
		}
		else
		{
			string s;
			string subPath;
			s = path.substr(0, path.find_first_of("\\"));
			subPath = path.substr(path.find_first_of("\\") + 1, path.size());

			FCB fcb;
			::memcpy(&fcb, this->memory.blockList[this->currentFolder].content, sizeof(FCB));
			int blockIndex = fcb.contentBlock;

			for (int i = 1; i < this->fatTable.fatBlockList[blockIndex].getLengthInUse() / 64; i++)
			{
				FileRecord fileRecord;
				::memcpy(&fileRecord, &(this->memory.blockList[blockIndex].content[i * 64]), sizeof(FileRecord));
				if (fileRecord.FCBBlock == -1)
					continue;
				if (fileRecord.name == s)
				{
					FCB aFCB;
					::memcpy(&aFCB, this->memory.blockList[fileRecord.FCBBlock].content, sizeof(FCB));
					if (aFCB.folderOrFile == true)
					{
						this->currentFolder = fileRecord.FCBBlock;
						if (cd(subPath))
							return true;
						else
						{
							cout << "路径 " + path + "错误,不能执行此操作!  如需查看基本操作,请输入help";
							return false;
						}
					}
					else
					{
						cout << "路径 " + path + "错误,不能执行此操作!  如需查看基本操作,请输入help";
						return false;
					}
				}
			}
			while (this->fatTable.fatBlockList[blockIndex].getNextBlock() != -1)
			{
				blockIndex = this->fatTable.fatBlockList[blockIndex].getNextBlock();
				for (int i = 0; i < this->fatTable.fatBlockList[blockIndex].getLengthInUse() / 64; i++)
				{
					FileRecord fileRecord;
					::memcpy(&fileRecord, &(this->memory.blockList[blockIndex].content[i * 64]), sizeof(FileRecord));
					if (fileRecord.FCBBlock == -1)
						continue;
					if (fileRecord.name == s)
					{
						FCB aFCB;
						::memcpy(&aFCB, this->memory.blockList[fileRecord.FCBBlock].content, sizeof(FCB));
						if (aFCB.folderOrFile == true)
						{
							this->currentFolder = fileRecord.FCBBlock;
							if (cd(subPath))
								return true;
							else
							{
								cout << "路径 " + path + "错误,不能执行此操作!  如需查看基本操作,请输入help";
								return false;
							}
						}
						else
						{
							cout << "路径 " + path + "错误,不能执行此操作!  如需查看基本操作,请输入help";
							return false;
						}
					}
				}
			}
			return false;
		}
	}
}

bool FileSystem::del(string fileName)
{
	FCB fcb;
	::memcpy(&fcb, this->memory.blockList[this->currentFolder].content, sizeof(FCB));
	int blockIndex = fcb.contentBlock;

	for (int i = 1; i < this->fatTable.fatBlockList[blockIndex].getLengthInUse() / 64; i++)
	{
		FileRecord fileRecord;
		::memcpy(&fileRecord, &(this->memory.blockList[blockIndex].content[i * 64]), sizeof(FileRecord));
		if (fileRecord.name == fileName)
		{
			FCB aFCB;
			::memcpy(&aFCB, this->memory.blockList[fileRecord.FCBBlock].content, sizeof(FCB));
			if (aFCB.folderOrFile == false)
			{
				int next = this->fatTable.fatBlockList[aFCB.contentBlock].getNextBlock();
				this->fatTable.fatBlockList[aFCB.contentBlock].setUse(false);

				while (next != -1)
				{
					this->fatTable.fatBlockList[next].setUse(false);
					next = this->fatTable.fatBlockList[next].getNextBlock();
				}
				this->fatTable.fatBlockList[fileRecord.FCBBlock].setUse(false);
				fileRecord.FCBBlock = -1;
				::memcpy(&(this->memory.blockList[blockIndex].content[i * 64]), &fileRecord, sizeof(FileRecord));
				return true;
			}
			else
			{
				cout << fileName + "是文件夹，不能进行此操作!  如需查看基本操作,请输入help";
				return false;
			}
		}
	}
	while (this->fatTable.fatBlockList[blockIndex].getNextBlock() != -1)
	{
		blockIndex = this->fatTable.fatBlockList[blockIndex].getNextBlock();
		for (int i = 0; i < this->fatTable.fatBlockList[blockIndex].getLengthInUse() / 64; i++)
		{
			FileRecord fileRecord;
			::memcpy(&fileRecord, &(this->memory.blockList[blockIndex].content[i * 64]), sizeof(FileRecord));
			if (fileRecord.name == fileName)
			{
				FCB aFCB;
				::memcpy(&aFCB, this->memory.blockList[fileRecord.FCBBlock].content, sizeof(FCB));
				if (aFCB.folderOrFile == false)
				{
					int next = this->fatTable.fatBlockList[aFCB.contentBlock].getNextBlock();
					this->fatTable.fatBlockList[aFCB.contentBlock].setUse(false);

					while (next != -1)
					{
						this->fatTable.fatBlockList[next].setUse(false);
						next = this->fatTable.fatBlockList[next].getNextBlock();
					}
					this->fatTable.fatBlockList[fileRecord.FCBBlock].setUse(false);
					fileRecord.FCBBlock = -1;
					::memcpy(&(this->memory.blockList[blockIndex].content[i * 64]), &fileRecord, sizeof(FileRecord));
					return true;
				}
				else
				{
					cout << fileName + "是文件夹，不能进行此操作!  如需查看基本操作,请输入help";
					return false;
				}
			}
		}
	}
	cout << "当前目录下不存在文件 " + fileName + " ，不能进行此操作!  如需查看基本操作,请输入help";
	return false;
}

bool FileSystem::deltree(string folderName)
{
	FCB fcb;
	::memcpy(&fcb, this->memory.blockList[this->currentFolder].content, sizeof(FCB));
	int blockIndex = fcb.contentBlock;

	for (int i = 1; i < this->fatTable.fatBlockList[blockIndex].getLengthInUse() / 64; i++)
	{
		FileRecord fileRecord;
		::memcpy(&fileRecord, &(this->memory.blockList[blockIndex].content[i * 64]), sizeof(FileRecord));
		if (fileRecord.name == folderName)
		{
			FCB aFCB;
			::memcpy(&aFCB, this->memory.blockList[fileRecord.FCBBlock].content, sizeof(FCB));
			if (aFCB.folderOrFile == true)
			{
				int next = this->fatTable.fatBlockList[aFCB.contentBlock].getNextBlock();
				this->fatTable.fatBlockList[aFCB.contentBlock].setUse(false);

				while (next != -1)
				{
					this->fatTable.fatBlockList[next].setUse(false);
					next = this->fatTable.fatBlockList[next].getNextBlock();
				}
				this->fatTable.fatBlockList[fileRecord.FCBBlock].setUse(false);
				fileRecord.FCBBlock = -1;
				::memcpy(&(this->memory.blockList[blockIndex].content[i * 64]), &fileRecord, sizeof(FileRecord));
				return true;
			}
			else
			{
				cout << folderName + "是文件，不能进行此操作!  如需查看基本操作,请输入help";
				return false;
			}
		}
	}
	while (this->fatTable.fatBlockList[blockIndex].getNextBlock() != -1)
	{
		blockIndex = this->fatTable.fatBlockList[blockIndex].getNextBlock();
		for (int i = 0; i < this->fatTable.fatBlockList[blockIndex].getLengthInUse() / 64; i++)
		{
			FileRecord fileRecord;
			::memcpy(&fileRecord, &(this->memory.blockList[blockIndex].content[i * 64]), sizeof(FileRecord));
			if (fileRecord.name == folderName)
			{
				FCB aFCB;
				::memcpy(&aFCB, this->memory.blockList[fileRecord.FCBBlock].content, sizeof(FCB));
				if (aFCB.folderOrFile == true)
				{
					int next = this->fatTable.fatBlockList[aFCB.contentBlock].getNextBlock();
					this->fatTable.fatBlockList[aFCB.contentBlock].setUse(false);

					while (next != -1)
					{
						this->fatTable.fatBlockList[next].setUse(false);
						next = this->fatTable.fatBlockList[next].getNextBlock();
					}
					this->fatTable.fatBlockList[fileRecord.FCBBlock].setUse(false);
					fileRecord.FCBBlock = -1;
					::memcpy(&(this->memory.blockList[blockIndex].content[i * 64]), &fileRecord, sizeof(FileRecord));
					return true;
				}
				else
				{
					cout << folderName + "是文件，不能进行此操作!  如需查看基本操作,请输入help";
					return false;
				}
			}
		}
	}
	cout << "当前目录下不存在文件夹 " + folderName + " ，不能进行此操作!  如需查看基本操作,请输入help";
	return false;
}

bool FileSystem::type(string fileName)
{
	FCB fcb;
	::memcpy(&fcb, this->memory.blockList[this->currentFolder].content, sizeof(FCB));
	int blockIndex = fcb.contentBlock;

	for (int i = 1; i < this->fatTable.fatBlockList[blockIndex].getLengthInUse() / 64; i++)
	{
		FileRecord fileRecord;
		::memcpy(&fileRecord, &(this->memory.blockList[blockIndex].content[i * 64]), sizeof(FileRecord));
		if (fileRecord.FCBBlock == -1)
			continue;
		if (fileRecord.name == fileName)
		{
			FCB aFCB;
			::memcpy(&aFCB, this->memory.blockList[fileRecord.FCBBlock].content, sizeof(FCB));
			if (aFCB.folderOrFile == false)
			{
				char content[1025];
				int next = this->fatTable.fatBlockList[aFCB.contentBlock].getNextBlock();

				::memcpy(content, this->memory.blockList[aFCB.contentBlock].content, this->fatTable.fatBlockList[aFCB.contentBlock].getLengthInUse());
				content[this->fatTable.fatBlockList[aFCB.contentBlock].getLengthInUse()] = '\0';
				cout << content;

				while (next != -1)
				{
					::memcpy(content, this->memory.blockList[next].content, this->fatTable.fatBlockList[next].getLengthInUse());
					content[this->fatTable.fatBlockList[next].getLengthInUse()] = '\0';
					cout << content;
					next = this->fatTable.fatBlockList[next].getNextBlock();
				}
				return true;
			}
			else
			{
				cout << fileName + "是文件夹，不能进行此操作!  如需查看基本操作,请输入help";
				return false;
			}
		}
	}
	while (this->fatTable.fatBlockList[blockIndex].getNextBlock() != -1)
	{
		blockIndex = this->fatTable.fatBlockList[blockIndex].getNextBlock();
		for (int i = 0; i < this->fatTable.fatBlockList[blockIndex].getLengthInUse() / 64; i++)
		{
			FileRecord fileRecord;
			::memcpy(&fileRecord, &(this->memory.blockList[blockIndex].content[i * 64]), sizeof(FileRecord));
			if (fileRecord.FCBBlock == -1)
				continue;
			if (fileRecord.name == fileName)
			{
				FCB aFCB;
				::memcpy(&aFCB, this->memory.blockList[fileRecord.FCBBlock].content, sizeof(FCB));
				if (aFCB.folderOrFile == false)
				{
					char content[1025];
					int next = this->fatTable.fatBlockList[aFCB.contentBlock].getNextBlock();

					::memcpy(content, this->memory.blockList[aFCB.contentBlock].content, this->fatTable.fatBlockList[aFCB.contentBlock].getLengthInUse());
					content[this->fatTable.fatBlockList[aFCB.contentBlock].getLengthInUse()] = '\0';
					cout << content;

					while (next != -1)
					{
						::memcpy(content, this->memory.blockList[next].content, this->fatTable.fatBlockList[next].getLengthInUse());
						content[this->fatTable.fatBlockList[next].getLengthInUse()] = '\0';
						cout << content;
						next = this->fatTable.fatBlockList[next].getNextBlock();
					}
					return true;
				}
				else
				{
					cout << fileName + "是文件夹，不能进行此操作!  如需查看基本操作,请输入help";
					return false;
				}
			}
		}
	}
	cout << "当前目录下不存在文件 " + fileName + " ，不能进行此操作!  如需查看基本操作,请输入help";
	return false;
}

bool FileSystem::edit(string fileName)
{
	if (!type(fileName))
		return false;
	char c;
	char s[2];
	string content = "";
	while ((c = getchar()) != EOF)
	{
		s[0] = c;
		s[1] = '\0';

		content.append(s);
	}

	int length = content.length();

	FCB fcb;
	::memcpy(&fcb, this->memory.blockList[this->currentFolder].content, sizeof(FCB));
	int blockIndex = fcb.contentBlock;

	for (int i = 1; i < this->fatTable.fatBlockList[blockIndex].getLengthInUse() / 64; i++)
	{
		FileRecord fileRecord;
		::memcpy(&fileRecord, &(this->memory.blockList[blockIndex].content[i * 64]), sizeof(FileRecord));
		if (fileRecord.FCBBlock == -1)
			continue;
		if (fileRecord.name == fileName)
		{
			FCB aFCB;
			::memcpy(&aFCB, this->memory.blockList[fileRecord.FCBBlock].content, sizeof(FCB));
			if (aFCB.folderOrFile == false)
			{
				int currentBlock = aFCB.contentBlock;
				int next = this->fatTable.fatBlockList[aFCB.contentBlock].getNextBlock();

				while (next != -1)
				{
					currentBlock = next;
					next = this->fatTable.fatBlockList[next].getNextBlock();
				}
				int l = this->fatTable.fatBlockList[currentBlock].getLengthInUse();

				if (length <= 1024 - l)
				{
					::memcpy(&this->memory.blockList[currentBlock].content[l], content.c_str(), length);
					this->fatTable.fatBlockList[currentBlock].setLengthInUse(l + length);
				}
				else
				{
					vector<int> indexList;
					indexList.push_back(currentBlock);
					int newBlock = (length - (1024 - l)) / 1024;
					if ((length - (1024 - l)) % 1024 != 0)
						newBlock++;

					::memcpy(&this->memory.blockList[currentBlock].content[l], content.c_str(), 1024 - l);
					this->fatTable.fatBlockList[currentBlock].setLengthInUse(1024);

					int startPos = 1024 - l;
					int i = 0; int j = 0;
					for ( i = 0; i < newBlock - 1; i++)
					{
						for (j = 0; j < 1024; j++)
						{
							if (this->fatTable.fatBlockList[j].getUse() == false)
							{
								break;
							}
						}
						if (j != 1024)
						{
							this->fatTable.fatBlockList[j].setUse(true);
							::memcpy(this->memory.blockList[j].content, content.substr(startPos, startPos + 1024).c_str(), 1024);
							this->fatTable.fatBlockList[j].setLengthInUse(1024);
							this->fatTable.fatBlockList[j].setNextBlock(-1);
							startPos = startPos + 1024;
							indexList.push_back(j);
						}
						if (j == 1024)
						{
							cout << "空间不足，不能执行此操作!  如需查看基本操作,请输入help";
							return false;
						}
					}
					for (int j = 0; j < 1024; j++)
					{
						if (this->fatTable.fatBlockList[j].getUse() == false)
						{
							break;
						}
					}
					if (j != 1024)
					{
						this->fatTable.fatBlockList[j].setUse(true);
						::memcpy(this->memory.blockList[j].content, content.substr(startPos, length).c_str(), length - startPos);
						this->fatTable.fatBlockList[j].setLengthInUse(length - startPos);
						this->fatTable.fatBlockList[j].setNextBlock(-1);
						indexList.push_back(j);
					}
					if (j == 1024)
					{
						cout << "空间不足，不能执行此操作!  如需查看基本操作,请输入help";
						return false;
					}
					for (i = 0; i < indexList.size() - 1; i++)
					{
						this->fatTable.fatBlockList[indexList.at(i)].setNextBlock(indexList.at(i + 1));
					}
				}
				return true;
			}
			else
			{
				cout << fileName + "是文件夹，不能进行此操作!  如需查看基本操作,请输入help";
				return false;
			}
		}
	}
	while (this->fatTable.fatBlockList[blockIndex].getNextBlock() != -1)
	{
		blockIndex = this->fatTable.fatBlockList[blockIndex].getNextBlock();
		for (int i = 0; i < this->fatTable.fatBlockList[blockIndex].getLengthInUse() / 64; i++)
		{
			FileRecord fileRecord;
			::memcpy(&fileRecord, &(this->memory.blockList[blockIndex].content[i * 64]), sizeof(FileRecord));
			if (fileRecord.FCBBlock == -1)
				continue;
			if (fileRecord.name == fileName)
			{
				FCB aFCB;
				::memcpy(&aFCB, this->memory.blockList[fileRecord.FCBBlock].content, sizeof(FCB));
				if (aFCB.folderOrFile == false)
				{
					int currentBlock = aFCB.contentBlock;
					int next = this->fatTable.fatBlockList[aFCB.contentBlock].getNextBlock();

					while (next != -1)
					{
						currentBlock = next;
						next = this->fatTable.fatBlockList[next].getNextBlock();
					}
					int l = this->fatTable.fatBlockList[currentBlock].getLengthInUse();

					if (length <= 1024 - l)
					{
						::memcpy(&this->memory.blockList[currentBlock].content[l], content.c_str(), length);
						this->fatTable.fatBlockList[currentBlock].setLengthInUse(l + length);
					}
					else
					{
						vector<int> indexList;
						indexList.push_back(currentBlock);
						int newBlock = (length - (1024 - l)) / 1024;
						if ((length - (1024 - l)) % 1024 != 0)
							newBlock++;

						::memcpy(&this->memory.blockList[currentBlock].content[l], content.c_str(), 1024 - l);
						this->fatTable.fatBlockList[currentBlock].setLengthInUse(1024);

						int i = 0;
						int j = 0;
						int startPos = 1024 - l;
						for ( i = 0; i < newBlock - 1; i++)
						{
							for ( j = 0; j < 1024; j++)
							{
								if (this->fatTable.fatBlockList[j].getUse() == false)
								{
									break;
								}
							}
							if (j != 1024)
							{
								this->fatTable.fatBlockList[j].setUse(true);
								::memcpy(this->memory.blockList[j].content, content.substr(startPos, startPos + 1024).c_str(), 1024);
								this->fatTable.fatBlockList[j].setLengthInUse(1024);
								this->fatTable.fatBlockList[j].setNextBlock(-1);
								startPos = startPos + 1024;
								indexList.push_back(j);
							}
							if (j == 1024)
							{
								cout << "空间不足，不能执行此操作!  如需查看基本操作,请输入help";
								return false;
							}
						}
						for (int j = 0; j < 1024; j++)
						{
							if (this->fatTable.fatBlockList[j].getUse() == false)
							{
								break;
							}
						}
						if (j != 1024)
						{
							this->fatTable.fatBlockList[j].setUse(true);
							::memcpy(this->memory.blockList[j].content, content.substr(startPos, length).c_str(), length - startPos);
							this->fatTable.fatBlockList[j].setLengthInUse(length - startPos);
							this->fatTable.fatBlockList[j].setNextBlock(-1);
							indexList.push_back(j);
						}
						if (j == 1024)
						{
							cout << "空间不足，不能执行此操作!  如需查看基本操作,请输入help";
							return false;
						}
						for (i = 0; i < indexList.size() - 1; i++)
						{
							this->fatTable.fatBlockList[indexList.at(i)].setNextBlock(indexList.at(i + 1));
						}
					}
					return true;
				}
				else
				{
					cout << fileName + "是文件夹，不能进行此操作!  如需查看基本操作,请输入help";
					return false;
				}
			}
		}
	}
	cout << "当前目录下不存在文件 " + fileName + " ，不能进行此操作!  如需查看基本操作,请输入help";
	return false;
}

void FileSystem::instru()
{
	cout << "基本指令：" << endl;
	cout << "    dir —————— 显示当前文件夹目录" << endl;
	cout << "    cd.. —————— 进入上一级文件夹" << endl;
	cout << "    cd 路径名 —————— 进入路径名的目录" << endl;
	cout << "    mkdir 文件夹名 —————— 在本目录下创建文件夹" << endl;
	cout << "    touch 文件名 —————— 在本目录下创建文件" << endl;
	cout << "    deltree 文件夹名 —————— 删除文件夹和其中的和文件" << endl;
	cout << "    del 文件名 —————— 删除文件" << endl;
	cout << "    type 文件名 —————— 打开文件" << endl;
	cout << "    edit 文件名 —————— 打开文件并进行内容追加" << endl;
	cout << "    format —————— 格式化文件系统" << endl;
	cout << "    help  ——————  操作说明" << endl;
	cout << "    exit —————— 退出文件系统" << endl;
}

string FileSystem::getCurrentPath()
{
	int currentBlock = this->currentFolder;
	vector<string> pathList;

	while (this->currentFolder != 0)
	{
		FCB fcb;
		::memcpy(&fcb, this->memory.blockList[this->currentFolder].content, sizeof(FCB));
		pathList.push_back(fcb.name);
		cd("..");
	}
	this->currentFolder = currentBlock;
	string path = "";
	for (int i = pathList.size() - 1; i >= 0; i--)
	{
		path = path + "\\" + pathList.at(i);
	}
	if (pathList.size() == 0)
		path = "FileSystem:\\>";
	else
		path = "FileSystem:" + path + ">";
	return path;
}
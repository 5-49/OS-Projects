#include <iostream>
#include <string>
#include <vector>
using namespace std;
#include "File.h"

int main(int argc, char *argv[], char *envp[])
{
	FileSystem fileSystem;

	FILE * stream;

	if ((stream = fopen("data.dat", "r+b")) != NULL)
	{
		fread(&fileSystem, sizeof(FileSystem), 1, stream);
		Block * blockList = new Block[1024];
		fread(blockList, sizeof(Block[1024]), 1, stream);
		fileSystem.setMemory(blockList);
		fclose(stream);
	}



	cout << "欢迎进入文件管理系统！" << endl;
	cout << "如需查看帮助,请在英文状态下输入help"<< endl<<endl;
	cout << " @copyright 1854025杨晶" << endl;

	char c;
	char s[2];
	while (true)
	{
		string in = "";
		cout << endl;
		cout << fileSystem.getCurrentPath();
		while ((c = getchar()) != '\n')
		{
			s[0] = c;
			s[1] = '\0';

			in = in.append(s);
		}
		if (in == "exit")
		{
			stream = NULL;
			if ((stream = fopen("data.dat", "w+b")) != NULL)
			{
				fwrite(&fileSystem, sizeof(FileSystem), 1, stream);
				fwrite(fileSystem.getMemory().blockList, sizeof(Block[1024]), 1, stream);
				fclose(stream);
			}
			break;
		}
		else if (in == "help")
			fileSystem.instru();
		else if (in == "dir")
			fileSystem.dir();
		else if (in == "cd..")
			fileSystem.cd("..");
		else if (in == "format")
			cout << "format" << endl;
		else
		{
			string cmd = in.substr(0, in.find_first_of(" "));
			string parameter = in.substr(in.find_first_of(" ") + 1, in.length());
			if (cmd == "cd")
				fileSystem.cd(parameter);
			else if (cmd == "mkdir")
				fileSystem.createFolder(parameter);
			else if (cmd == "touch")
				fileSystem.createFile(parameter);
			else if (cmd == "deltree")
				fileSystem.deltree(parameter);
			else if (cmd == "del")
				fileSystem.del(parameter);
			else if (cmd == "type")
				fileSystem.type(parameter);
			else if (cmd == "edit")
				fileSystem.edit(parameter);
			else
				cout << cmd << "操作不合法，如需查看帮助,请输入help";
		}
		cout << endl << endl;
	}

	return 0;
}
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include "utility.h"
#include "create_make.h"
using namespace std;

void GetOpt(int argc,char **argv,string &cfgFile,bool &onlyCreateMake,bool &clean)
{
	onlyCreateMake = false;
	clean = false;
	int c = 0;
	while((c = getopt(argc,argv,"cghf:")) != -1)
	{
		switch(c)
		{
		case 'c':
			clean = true;
			break;
		case 'f':
			cfgFile = optarg;
			break;
		case 'g':
			onlyCreateMake = true;
			break;
		case 'h':
			cout<<"usage:"<<argv[0]<<" [ -cg ] [ -f config file ]"<<endl;
			cout<<"  -c			 make clean"<<endl;
			cout<<"  -f FILENAME	read config files (default:"<<cfgFile.c_str()<<")"<<endl;
			cout<<"  -g			 only generate makefile"<<endl;
			cout<<"  -h			 print this help message"<<endl;
			exit (0);
		}
	}
}

int main(int argc,char **argv)
{
	string cfgFile = "makefile.in";
	
	bool onlyCreateMake;
	bool clean;
	
	GetOpt(argc,argv,cfgFile,onlyCreateMake,clean);
	CCreateMake make;
	if(!make.Init(cfgFile.c_str()))
		return 0;
	if(onlyCreateMake)
		make.WriteMake();
	else if(clean)
		make.Clean();
	else 
	{
		make.WriteMake();
		make.Make();
	}
	/*list<string> fileList;
	FindFile("/home/xzg",".hpp;.h",fileList);
	for(list<string>::iterator i = fileList.begin(); i != fileList.end(); i++)
	{
		cout<<*i<<endl;
	}
	if(StrEndCmp("xizhigang hello","hello"))
		cout<<"ok"<<endl;
	if(StrEndCmp("hello","xizhigang hello"))
		cout<<"ok"<<endl;*/
	return 0;
}

#include "utility.h"
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <iostream>

int SplitLine(char **templa, int templatecount, char *pkt,char space)
{
	int i = 0;
	while(*pkt == space)
		++pkt;
	while(*pkt != 0)
	{
		if((*pkt == '\r') || (*pkt == '\n') || (*pkt == '\t'))
		{
			memmove(pkt,pkt+1,strlen(pkt+1)+1);
		}
		else if(i == 0)
		{
			templa[i] = pkt;
			++i;
		}
		else if((*pkt == space) && (i < templatecount))
		{
			*pkt = 0;
			++pkt;
			while(*pkt == space)
				++pkt;
			templa[i] = pkt;
			++i;
		}
		else 
		{
			++pkt;
		}
	}
	return i;
}

/*struct dirent {
  ino_t          d_ino;        inode number 
  off_t          d_off;        offset to the next dirent 
  unsigned short d_reclen;     length of this record 
  unsigned char  d_type;       type of file; not supported
  by all file system types 
  char           d_name[256];  filename 
  };
  DT_BLK      This is a block device.

  DT_CHR      This is a character device.

  DT_DIR      This is a directory.

  DT_FIFO     This is a named pipe (FIFO).

  DT_LNK      This is a symbolic link.

  DT_REG      This is a regular file.

  DT_SOCK     This is a Unix domain socket.

  DT_UNKNOWN  The file type is unknown.*/
bool FindFile(const char *path,const char *fileType,list<string> &findFile)
{
	DIR *dirp = opendir(path);
	if(dirp == NULL)
		return false;

	struct dirent *dp;
	char *p[20];
	string file = fileType;
	int num = SplitLine(p,20,(char*)file.c_str(),';');
	do 
	{
		if((dp = readdir(dirp)) != NULL)
		{
			if((dp->d_type == DT_DIR) && (strcmp(dp->d_name,"..") != 0) && (strcmp(dp->d_name,".") != 0))
			{
				string file = path;
				file.append("/");
				file.append((char *)dp->d_name);
				FindFile(file.c_str(),fileType,findFile);
			}
			else if(dp->d_type == DT_REG)
			{
				for(int i = 0; i < num; i++)
				{
					if(StrEndCmp(dp->d_name,p[i]))
					{
						string file = path;
						file.append("/");
						file.append((char *)dp->d_name);
						findFile.push_back(file);
					}
				}
			}
		}
	}while (dp != NULL);
	//cout<<DT_DIR<<endl;
	closedir(dirp);
	return true;
}

bool FindFile(const char *path,list<string> &fileName,list<string> &fullName)
{
	DIR *dirp = opendir(path);
	if(dirp == NULL)
		return false;

	struct dirent *dp;
	do 
	{
		if((dp = readdir(dirp)) != NULL)
		{
			if((dp->d_type == DT_DIR) && (strcmp(dp->d_name,"..") != 0) && (strcmp(dp->d_name,".") != 0))
			{
				string file = path;
				file.append("/");
				file.append((char *)dp->d_name);
				FindFile(file.c_str(),fileName,fullName);
			}
			else if(dp->d_type == DT_REG)
			{
				for(list<string>::iterator i = fileName.begin(); i != fileName.end(); i++)
				{
					if(strcmp(dp->d_name,i->c_str()) == 0)
					{
						string file = path;
						file.append("/");
						file.append((char *)dp->d_name);
						fullName.push_back(file);
					}
				}
			}
		}
	}while (dp != NULL);

	closedir(dirp);
	return true;
}

bool StrEndCmp(const char *str1,const char *str2)
{
	if((str2 == NULL) || (str1 == NULL))
		return false;
	int len1 = strlen(str1);
	int len2 = strlen(str2);
	if(len1 > len2)
		return strcmp(str1+len1-len2,str2) == 0;
	else
		return strcmp(str1,str2+len2-len1) == 0;
}

void Trim(char *str,const char *pkt)
{
	char *p = str;
	while(*p != 0)
	{
		for(int i = 0; i < (int)strlen(pkt); i++)
		{
			if(*p == pkt[i])
			{
				memmove(p,p+1,strlen(p+1));
				if(*p == 0)
					break;
			}
		}
		p++;
	}
}

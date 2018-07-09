#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <algorithm>
#include "create_make.h"
#include "utility.h"

static const char *KEYS[] = 
{
    "cc","link","src_path","src_file","obj_flag","link_opt","target"
};

bool CCreateMake::Init(const char *cfgFile)
{
    m_confFileName = cfgFile;
    
    FILE *pFile = fopen(cfgFile,"r");
    if(pFile == NULL)
    {
        cout<<"can't open "<<cfgFile<<" file"<<endl;
        return false;
    }
    char buf[256];
    while(fgets(buf,sizeof(buf),pFile) != NULL)
    {
        char *p[2];
        Trim(buf,"\n\r\t");
        if(SplitLine(p,2,buf,'=') >= 2)
        {
            //cout<<p[0]<<endl;
            for(int i = 0; i < (int)(sizeof(KEYS)/sizeof(KEYS[0])); i++)
            {
                if(strcmp(KEYS[i],p[0]) == 0)
                {
                    m_keyVal.insert(make_pair<string,string>(KEYS[i],p[1]));
                    break;
                }
            }
        }
    }
    if(m_keyVal.find("cc") == m_keyVal.end())
    {
        cout<<"please define cc"<<endl;
        return false;
    }
    if(m_keyVal.find("link") == m_keyVal.end())
    {
        cout<<"please define link"<<endl;
        return false;
    }
    if(m_keyVal.find("src_path") == m_keyVal.end())
    {
        cout<<"please define src_path"<<endl;
        return false;
    }
    if(m_keyVal.find("target") == m_keyVal.end())
    {
        cout<<"please define target"<<endl;
        return false;
    }
    if(m_keyVal.find("src_file") == m_keyVal.end())
    {
        m_keyVal.insert(make_pair<string,string>("src_file",".cpp;.c"));
    }
    fclose(pFile);
    return true;
}

string CCreateMake::GetObjFile(string &file)
{
    string str;
    int pos = file.find_last_of("/");
    if(pos != -1)
        str = file.substr(pos+1);
    pos = str.find_last_of(".");
    if(pos != -1)
        str.erase(pos);
    str.append(".o");
    return str;
}

string CCreateMake::GetSrcFile(string &file)
{
    int pos = file.find_last_of("/");
    if(pos == -1)
        return file;
    return file.substr(pos);
}

void CCreateMake::FindDependFile(string &file,list<string> &depFileList)
{
    FILE *pFile = fopen(file.c_str(),"r");
    if(pFile == NULL)
        return;
    char buf[256];
    list<string> thisFileDep;
    while(fgets(buf,sizeof(buf),pFile) != NULL)
    {
        char *p[2];
        Trim(buf,"#\n\r\t\"");
        if(SplitLine(p,2,buf,' ') >= 2)
        {
            if(strcmp(p[0],"include") != 0)
                continue;
            if(p[1][0] == '<')
                continue;
            thisFileDep.push_back(p[1]);
        }
    }
    
    fclose(pFile);
    list<string> pathFile;
    map<string,string>::iterator iter = m_keyVal.find("src_path");
    if(iter == m_keyVal.end())
        return;
    FindFile(iter->second.c_str(),thisFileDep,pathFile);
    for(list<string>::iterator i = pathFile.begin(); i != pathFile.end(); i++)
    {
        if(std::find(depFileList.begin(),depFileList.end(),*i) == depFileList.end())
        {
            depFileList.push_back(*i);
            FindDependFile(*i,depFileList);
        }
    }
}

void CCreateMake::WriteMake()
{
    map<string,string>::iterator iter = m_keyVal.find("src_path");
    if(iter == m_keyVal.end())
        return;
    string path = iter->second;
    iter = m_keyVal.find("src_file");
    if(iter == m_keyVal.end())
        return;
    string file = iter->second;
    iter = m_keyVal.find("target");
    if(iter == m_keyVal.end())
        return;
    string target = iter->second;
    string makeFile = target;
    makeFile.append(".mak");
    
    /*iter = m_keyVal.find("src_file");
    if(iter == m_keyVal.end())
        return;
    string srcType = iter->second;*/
    
    FILE *pFile = fopen(makeFile.c_str(),"w");
    if(pFile == NULL)
        return;
    char buf[256];
    
    iter = m_keyVal.find("cc");
    if(iter != m_keyVal.end())
    {
    	snprintf(buf,sizeof(buf),"CC=%s\n",iter->second.c_str());
        fputs(buf,pFile);
    }
    iter = m_keyVal.find("link");    
    if(iter != m_keyVal.end())
    {
    	snprintf(buf,sizeof(buf),"LINK=%s\n",iter->second.c_str());
        fputs(buf,pFile);
    }
    
    iter = m_keyVal.find("obj_flag");
    
    if(iter != m_keyVal.end())
    {
        snprintf(buf,sizeof(buf),"OBGFLAG=%s\n",iter->second.c_str());
        fputs(buf,pFile);
    }
    iter = m_keyVal.find("link_opt");
    if(iter != m_keyVal.end())
    {
        snprintf(buf,sizeof(buf),"LINKOPT=%s\n",iter->second.c_str());
        fputs(buf,pFile);
    }
    
    list<string> srcFileList;
    FindFile(path.c_str(),file.c_str(),srcFileList);
    
    string allObjFile;
    string allSrcFile;
    list<pair<string,string> > depFileList;
    for(list<string>::iterator i = srcFileList.begin(); i != srcFileList.end(); i++)
    {
        string objFile = GetObjFile(*i);
        allSrcFile.append(*i);
        allSrcFile.append(" ");
        allObjFile.append(objFile);
        allObjFile.append(" ");
        
        list<string> depFile;
        string srcFile = *i;
        FindDependFile(*i,depFile);
        for(list<string>::iterator iter = depFile.begin(); iter != depFile.end(); iter++)
        {
            srcFile.append(" ");
            srcFile.append(*iter);
        }
        depFileList.push_back(make_pair<string,string>(*i,srcFile));
    }
    /*char *p[20];
    int num = SplitLine(p,20,(char*)srcType.c_str(),';');
    for(int i = 0; i < num; i++)
    {
        fputs("\n",pFile);
        fputs(p[i],pFile);
        fputs(".o:",pFile);
        fputs("\n\t",pFile);
        fputs("gcc  -c $< $(OBGFLAG)",pFile);
    }*/
    fputs("\n",pFile);
    fputs(target.c_str(),pFile);
    fputs(":",pFile);
    fputs(allObjFile.c_str(),pFile);
    fputs("\n\t",pFile);
    
    fputs("$(LINK) -o ",pFile);
    
    fputs(target.c_str(),pFile);
    fputs(" ",pFile);
    fputs(allObjFile.c_str(),pFile);
    fputs(" $(LINKOPT) ",pFile);
    
    for(list<pair<string,string> >::iterator i = depFileList.begin(); i != depFileList.end(); i++)
    {
        fputs("\n",pFile);
        string objFile = GetObjFile(i->first);
        fputs(objFile.c_str(),pFile);
        fputs(":",pFile);
        fputs(i->second.c_str(),pFile);
        fputs("\n\t$(CC) -c ",pFile);
        fputs(i->first.c_str(),pFile);
        fputs(" $(OBGFLAG)",pFile);
    }
    fputs("\n",pFile);
    fputs("clean:",pFile);
    fputs("\n\trm -rf ",pFile);
    fputs(allObjFile.c_str(),pFile);
    fclose(pFile);
}

void CCreateMake::Make()
{
    map<string,string>::iterator iter = m_keyVal.find("target");
    if(iter == m_keyVal.end())
    {
        return;
    }
    string makeFile = iter->second;
    char buf[128];
    snprintf(buf,sizeof(buf),"make -f %s.mak",makeFile.c_str());
    system(buf);
}

void CCreateMake::Clean()
{
    map<string,string>::iterator iter = m_keyVal.find("target");
    if(iter == m_keyVal.end())
    {
        return;
    }
    string makeFile = iter->second;
    char buf[128];
    snprintf(buf,sizeof(buf),"make -f %s.mak clean",makeFile.c_str());
    system(buf);
}

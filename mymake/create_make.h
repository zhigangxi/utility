#ifndef _CREATE_MAKE_H_
#define _CREATE_MAKE_H_
#include <string>
#include <map>
#include <list>
using namespace std;

class CCreateMake
{
public:
    bool Init(const char *cfgFile);
    void WriteMake();
    void Make();
    void Clean();
private:
    string GetObjFile(string &file);
    string GetSrcFile(string &file);
    void FindDependFile(string &file,list<string> &depFileList);
    string m_confFileName;
    map<string,string> m_keyVal;
};

#endif

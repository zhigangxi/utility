#include "log.h"

FILE *CLogs::m_pFile;

bool CLogs::Init()
{
	m_pFile = NULL;
	return true;
}

CLogs::~CLogs()
{
	if (m_pFile != NULL)
		fclose(m_pFile);
}

bool CLogs::SetLogFileName(string &filename)
{
  m_pFile = fopen(filename.c_str(),"a+");
  if(m_pFile == NULL)
      return false;
  return true;
}

void CLogs::WriteLog(const char *fmt, ...)
{
  time_t now;
  time(&now);
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt,ap);
  printf("  %s",ctime(&now));
  
  if(m_pFile != NULL)
  {
    vfprintf(m_pFile,fmt, ap);
    fprintf(m_pFile,"  %s",ctime(&now));
  }
  va_end (ap);
}

#include "stdafx.h"
#include "global_manager.h"

CGlobalValManager *CGlobalValManager::m_manager;

CGlobalValManager *CGlobalValManager::CreateInstance()
{
    if (m_manager == NULL)
    {
        m_manager = new CGlobalValManager;
    }
    return m_manager;
}
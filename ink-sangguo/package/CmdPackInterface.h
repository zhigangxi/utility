#pragma once

class CCommand;

class CCmdPackageInterface
{
public:
	virtual void Init(CCommand*) = 0;
};
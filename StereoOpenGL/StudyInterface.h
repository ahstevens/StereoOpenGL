#pragma once

#include "DataLogger.h"

class StudyInterface
{
public:
	StudyInterface();
	~StudyInterface();

	void init();

private:
	DataLogger * m_pDataLogger;
};


#include "StudyInterface.h"

#include "Renderer.h"


StudyInterface::StudyInterface()
{
}


StudyInterface::~StudyInterface()
{
}

void StudyInterface::init()
{
	m_pDataLogger->getInstance().setID("drewtest");
	m_pDataLogger->getInstance().setLogDirectory("logs");
}

#pragma once

#include "WinsockClient.h"

#include "GLFWInputBroadcaster.h"

class StudyInterface : public GLFWInputObserver
{
public:
	StudyInterface();
	~StudyInterface();

	void init();

	void reset();

	void update();

	void draw();

private:
	WinsockClient* m_pSocket;

	bool m_bAddressEntryMode;
	bool m_bPortEntryMode;

	float m_fCOPDistance;
	float m_fCOPAngle;
	float m_fViewDistance;
	float m_fViewAngle;

	float m_fStepSize;
	float m_fMinStep;

	std::string m_strAddressBuffer;
	std::string m_strPortBuffer;

private:
	void receive(void* data);
};


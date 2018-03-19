#pragma once

#include "WinsockClient.h"

#include "GLFWInputBroadcaster.h"

#include <glm.hpp>

#include <chrono>

class StudyInterface : public GLFWInputObserver
{
public:
	StudyInterface();
	~StudyInterface();

	void init(glm::ivec2 screenDims);

	void reset();

	void update();

	void draw();

	glm::vec3 getCOP();
	float getEyeSep();

private:
	WinsockClient* m_pSocket;

	glm::ivec2 m_ivec2Screen;

	bool m_bAddressEntryMode;
	bool m_bPortEntryMode;
	bool m_bEyeSepEntryMode;
	bool m_bViewDistEntryMode;
	bool m_bViewAngleEntryMode;

	float m_fCOPDist;
	float m_fCOPAngle;
	float m_fCOPEyeSep;
	float m_fViewDist;
	float m_fViewAngle;

	float m_fStepSize;
	float m_fMinStep;

	float m_fMoveTime;
	std::chrono::high_resolution_clock::time_point m_tMoveStart;

	std::string m_strAddressBuffer;
	std::string m_strPortBuffer;
	std::string m_strEyeSepBuffer;
	std::string m_strViewDistBuffer;
	std::string m_strViewAngleBuffer;

private:
	void receive(void* data);
};


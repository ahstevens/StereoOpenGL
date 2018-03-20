#pragma once

#include "WinsockClient.h"
#include "GLFWInputBroadcaster.h"
#include "Hinge.h"

#include <glm.hpp>
#include <chrono>

class StudyInterface : public GLFWInputObserver
{
public:
	StudyInterface();
	~StudyInterface();

	void init(glm::ivec2 screenDims, float screenDiag);

	void reset();

	void update();

	void draw();

	glm::vec3 getCOP();
	float getEyeSep();

private:
	WinsockClient* m_pSocket;
	Hinge* m_pHinge;

	glm::ivec2 m_ivec2Screen;
	glm::vec2 m_vec2Screen;

	bool m_bAddressEntryMode;
	bool m_bPortEntryMode;
	bool m_bEyeSepEntryMode;
	bool m_bViewDistEntryMode;
	bool m_bViewAngleEntryMode;
	bool m_bMoveTimeEntryMode;

	float m_fCOPDist;
	float m_fCOPAngle;
	float m_fEyeSep;
	float m_fViewDist;
	float m_fViewAngle;

	float m_fStepSize;
	float m_fMinStep;

	std::vector<float> m_vfAngleConditions;
	std::vector<float> m_vfDistanceConditions;

	float m_fMoveTime;
	std::chrono::high_resolution_clock::time_point m_tMoveStart;
	float m_fLastAngle;

	std::string m_strAddressBuffer;
	std::string m_strPortBuffer;
	std::string m_strEyeSepBuffer;
	std::string m_strViewDistBuffer;
	std::string m_strViewAngleBuffer;
	std::string m_strMoveTimeBuffer;

private:
	void receive(void* data);
};


#pragma once

#include "WinsockClient.h"
#include "GLFWInputBroadcaster.h"
#include "Hinge.h"

#include <glm.hpp>
#include <chrono>
#include <random>

class StudyInterface : public GLFWInputObserver
{
public:
	StudyInterface();
	~StudyInterface();

	void init(glm::ivec2 screenDims, float screenDiag);

	void reset();

	void update();

	void draw();

	void begin();
	void next(bool stimulusDetected);
	void end();

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
	bool m_bNameEntryMode;

	bool m_bStudyMode;

	bool m_bLockViewCOP;

	float m_fCOPDist;
	float m_fCOPAngle;
	float m_fEyeSep;
	float m_fViewDist;
	float m_fViewAngle;

	std::default_random_engine m_Generator;
	std::uniform_int_distribution<int> m_Distribution;

	float m_fStepSize;
	float m_fMinStep;

	std::vector<float> m_vfAngleConditions;
	std::vector<float> m_vfDistanceConditions;

	std::vector<std::tuple<float, float, int>> m_vExperimentConditions; // <view angle, view distance, hinge start angle>

	bool m_bStaircaseAscending;
	int m_nReversals;

	float m_fBlankTime;
	std::chrono::high_resolution_clock::time_point m_tBlankStart;

	float m_fMoveTime;
	std::chrono::high_resolution_clock::time_point m_tMoveStart;
	float m_fLastAngle;
	float m_fTargetAngle;

	std::string m_strAddressBuffer;
	std::string m_strPortBuffer;
	std::string m_strEyeSepBuffer;
	std::string m_strViewDistBuffer;
	std::string m_strViewAngleBuffer;
	std::string m_strMoveTimeBuffer;
	std::string m_strNameBuffer;

private:
	void receive(void* data);
};


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
	enum STUDYPARAMFORMAT {
		REAL,
		INTEGER,
		IP,
		STRING
	};

	struct StudyParam {
		std::string desc;
		std::string buf;
		STUDYPARAMFORMAT format;
	};

	struct StudyCondition {
		float viewAngle;
		float viewDist;
		int startAngle;
		float hingeLen;
		glm::vec3 hingePos;
	};

	WinsockClient* m_pSocket;
	Hinge* m_pHinge;

	glm::ivec2 m_ivec2Screen;
	glm::vec2 m_vec2Screen;

	std::string m_strServerAddress;
	unsigned m_uiServerPort;

	std::string m_strName;

	bool m_bStudyMode;
	bool m_bShowStimulus;

	bool m_bLockViewCOP;

	float m_fCOPDist;
	float m_fCOPAngle;
	float m_fEyeSep;
	float m_fViewDist;
	float m_fViewAngle;

	std::default_random_engine m_Generator;
	std::uniform_int_distribution<int> m_AngleDistribution;
	std::uniform_int_distribution<int> m_BoolDistribution;

	float m_fStepSize;
	float m_fMinStep;

	std::vector<float> m_vfAngleConditions;
	std::vector<float> m_vfDistanceConditions;

	std::vector<StudyCondition> m_vExperimentConditions;

	std::string m_strLastResponse;
	int m_nReversals;

	float m_fStimulusTime;
	float m_fStimulusDelay;
	std::chrono::high_resolution_clock::time_point m_tStimulusStart;

	float m_fMoveTime;
	std::chrono::high_resolution_clock::time_point m_tMoveStart;
	float m_fLastAngle;
	float m_fTargetAngle;

	std::vector<StudyParam> m_vParams;
	StudyParam* m_pEditParam;

private:
	void receive(void* data);
};


#pragma once

#include "WinsockClient.h"
#include "GLFWInputBroadcaster.h"
#include "Hinge.h"
#include "ViewingConditionsDiagram.h"

#include <glm.hpp>
#include <chrono>
#include <random>
#include <future>

#define STUDYPARAM_DECIMAL	1 << 0
#define STUDYPARAM_POSNEG	1 << 1
#define STUDYPARAM_ALPHA	1 << 2
#define STUDYPARAM_NUMERIC	1 << 3
#define STUDYPARAM_IP		1 << 4

class MagnitudeStudy : public GLFWInputObserver
{
public:
	MagnitudeStudy();
	~MagnitudeStudy();

	void init(glm::ivec2 screenRes, glm::mat4 worldToScreenTransform);

	void reset();

	void update();

	void draw();

	unsigned int generateTrials(bool randomOrder = true);
	unsigned int trialsRemaining();
	void loadNextCondition();

	void beginStudy();

	glm::vec3 getCOP();
	float getEyeSep();

	std::string conditionString();

	void outputTable();

	bool isStudyActive() { return m_bStudyMode; }

private:
	enum StudyResponse {
		NONE,
		ACUTE,
		OBTUSE
	};

	struct StudyParam {
		std::string desc;
		std::string buf;
		uint16_t format;
	};

	struct StudyCondition {
		float viewAngle;
		float viewDistFactor;
		float eyeSeparation;
		float angle;
		float len;
		bool fishtank;
	};

	struct Rod {
		float length;
		float angle;
		float diameter;
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec3 rotAxis;
		std::string textureName;
		std::string shaderName;
		bool originCenter;
	};

	std::vector<glm::vec3> m_vec3GridPoints;
	std::vector<glm::vec3> m_vec3DistortedGridPoints;
	float m_fMaxDistortionMag;

	std::future<bool> m_SocketFuture;
	WinsockClient* m_pSocket;
	Rod m_Vector;
	Rod m_MeasuringRod;

	glm::ivec2 m_ivec2Screen;
	glm::mat4 m_mat4Screen;

	std::string m_strServerAddress;
	unsigned m_uiServerPort;

	std::string m_strName;

	bool m_bDemoMode;
	bool m_bStudyMode;
	bool m_bPaused;
	bool m_bShowStimulus;
	bool m_bBlockInput;
	bool m_bFishtank;
	bool m_bShowDiagram;
	bool m_bWaitingForResponse;
	bool m_bDisplayCondition;
	bool m_bScreenshotConditions;

	float m_fCOPDist;
	float m_fCOPAngle;
	float m_fEyeSep;
	float m_fViewDist;
	float m_fViewAngle;

	float m_fHingeSize;

	unsigned int m_uiDemoRotCount;

	std::default_random_engine m_Generator;
	std::uniform_int_distribution<int> m_AngleDistribution;
	std::uniform_int_distribution<int> m_BoolDistribution;

	float m_fStepSize;
	float m_fMinStep;

	std::vector<StudyCondition> m_vExperimentConditions;
	unsigned m_nTrials;

	float m_fStimulusTime;
	float m_fStimulusDelay;
	std::chrono::high_resolution_clock::time_point m_tStimulusStart;

	float m_fMoveTime;
	std::chrono::high_resolution_clock::time_point m_tMoveStart;
	float m_fLastAngle;
	float m_fTargetAngle;

	std::vector<StudyParam> m_vParams;
	StudyParam* m_pEditParam;

	ViewingConditionsDiagram* m_pDiagram;

	std::string m_strCondition;

private:
	void endTrial();
	void endStudy();
	void writeToLog();
	void loadCondition(StudyCondition &c);
	void resetMeasuringRod();
	float calculateExpectedResponse(StudyCondition &c);
	bool moveScreen(float viewAngle, bool forceMove = false);
	void receive(void* data);
};

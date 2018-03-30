#include "StudyInterface.h"

#include "Renderer.h"
#include "DataLogger.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <gtc/matrix_transform.hpp>

StudyInterface::StudyInterface()
	: m_pSocket(NULL)
	, m_pHinge(NULL)
	, m_bAddressEntryMode(false)
	, m_bPortEntryMode(false)
	, m_bEyeSepEntryMode(false)
	, m_bViewDistEntryMode(false)
	, m_bViewAngleEntryMode(false)
	, m_bMoveTimeEntryMode(false)
	, m_bNameEntryMode(false)
	, m_bStudyMode(false)
	, m_bShowStimulus(true)
	, m_bLockViewCOP(false)
	, m_Generator(std::random_device()())
	, m_AngleDistribution(std::uniform_int_distribution<int>(10, 20))
	, m_BoolDistribution(std::uniform_int_distribution<int>(0, 1))
{
}


StudyInterface::~StudyInterface()
{
	if (m_pSocket)
		delete m_pSocket;

	if (m_pHinge)
		delete m_pHinge;
}

void StudyInterface::init(glm::ivec2 screenDims, float screenDiag)
{
	DataLogger::getInstance().setLogDirectory("logs");

	m_ivec2Screen = screenDims;

	float sizer = screenDiag / sqrt(glm::dot(glm::vec2(screenDims), glm::vec2(screenDims)));

	m_vec2Screen = glm::vec2(screenDims) * sizer;

	reset();

	if (m_pSocket == NULL)
		m_pSocket = new WinsockClient();

	if (m_pHinge == NULL)
		m_pHinge = new Hinge(5.f, 90.f);
}

void StudyInterface::reset()
{
	m_bAddressEntryMode = false;
	m_bPortEntryMode = false;
	m_bEyeSepEntryMode = false;
	m_bViewDistEntryMode = false;
	m_bViewAngleEntryMode = false;
	m_bMoveTimeEntryMode = false;
	m_bNameEntryMode = false;

	m_bStudyMode = false;

	m_bLockViewCOP = false;

	m_fViewAngle = 0.f;
	m_fViewDist = 57.f;
	m_fEyeSep = 6.7;

	m_fCOPAngle = m_fViewAngle;
	m_fCOPDist = m_fViewDist;

	m_fMinStep = 2.f;
	m_fStepSize = m_fMinStep;

	m_strLastResponse = std::string();
	m_nReversals = 11; // ignore the first

	m_vExperimentConditions.clear();

	m_vfAngleConditions = { 0.f, 10.f, 30.f, 60.f };
	m_vfDistanceConditions = { 1.f, 0.5f, 2.f };

	m_fStimulusTime = 1.5f;
	m_fStimulusDelay = 1.f;
	m_tStimulusStart = std::chrono::high_resolution_clock::time_point();

	m_fMoveTime = 5.f;
	m_tMoveStart = std::chrono::high_resolution_clock::time_point();

	m_fLastAngle = m_fTargetAngle = 0.f;

	m_strAddressBuffer = "192.168.";
	m_strPortBuffer = "5005";
	m_strEyeSepBuffer = "6.70";
	m_strViewDistBuffer = "57.0";
	m_strViewAngleBuffer = "0";
	m_strMoveTimeBuffer = "5.0";

	m_strNameBuffer = "no name";

}

void StudyInterface::update()
{
	using clock = std::chrono::high_resolution_clock;
	auto tick = clock::now();

	float elapsedMove = std::chrono::duration<float>(clock::now() - m_tMoveStart).count();
	float elapsedStim = std::chrono::duration<float>(clock::now() - m_tStimulusStart).count();

	if (elapsedMove > 0.f)
	{	
		if (elapsedMove < m_fMoveTime)
		{
			float ratio = elapsedMove / m_fMoveTime;
			m_fViewAngle = m_fLastAngle + (m_fTargetAngle - m_fLastAngle) * ratio;
		}
		else
		{
			m_fViewAngle = m_fTargetAngle;
		}
	}

	if (m_bStudyMode)
	{
		if (elapsedStim >= 0.f && elapsedStim <= m_fStimulusTime)
			m_bShowStimulus = true;
		else
			m_bShowStimulus = false;
	}
	else
		m_bShowStimulus = true;
}


void StudyInterface::draw()
{
	if (m_pHinge && m_bShowStimulus)
		m_pHinge->draw();

	if (m_bAddressEntryMode)
	{
		Renderer::getInstance().drawUIText(
			"Server IP: " + m_strAddressBuffer + "_",
			glm::vec4(1.f),
			glm::vec3(0.f),
			glm::quat(),
			100.f,
			Renderer::HEIGHT,
			Renderer::LEFT,
			Renderer::BOTTOM_LEFT
		);
	}

	if (m_bPortEntryMode)
	{
		Renderer::getInstance().drawUIText(
			"Server Port: " + m_strPortBuffer + "_",
			glm::vec4(1.f),
			glm::vec3(0.f),
			glm::quat(),
			100.f,
			Renderer::HEIGHT,
			Renderer::LEFT,
			Renderer::BOTTOM_LEFT
		);
	}

	if (m_bEyeSepEntryMode)
	{
		Renderer::getInstance().drawUIText(
			"Eye Separation (cm): " + m_strEyeSepBuffer + "_",
			glm::vec4(1.f),
			glm::vec3(0.f),
			glm::quat(),
			100.f,
			Renderer::HEIGHT,
			Renderer::LEFT,
			Renderer::BOTTOM_LEFT
		);
	}

	if (m_bViewDistEntryMode)
	{
		Renderer::getInstance().drawUIText(
			"View Distance (cm): " + m_strViewDistBuffer + "_",
			glm::vec4(1.f),
			glm::vec3(0.f),
			glm::quat(),
			100.f,
			Renderer::HEIGHT,
			Renderer::LEFT,
			Renderer::BOTTOM_LEFT
		);
	}

	if (m_bViewAngleEntryMode)
	{
		Renderer::getInstance().drawUIText(
			"View Angle (deg): " + m_strViewAngleBuffer + "_",
			glm::vec4(1.f),
			glm::vec3(0.f),
			glm::quat(),
			100.f,
			Renderer::HEIGHT,
			Renderer::LEFT,
			Renderer::BOTTOM_LEFT
		);
	}

	if (m_bMoveTimeEntryMode)
	{
		Renderer::getInstance().drawUIText(
			"Display Move Time (sec): " + m_strMoveTimeBuffer + "_",
			glm::vec4(1.f),
			glm::vec3(0.f),
			glm::quat(),
			100.f,
			Renderer::HEIGHT,
			Renderer::LEFT,
			Renderer::BOTTOM_LEFT
		);
	}

	if (m_bNameEntryMode)
	{
		Renderer::getInstance().drawUIText(
			"Name: " + m_strNameBuffer + "_",
			glm::vec4(1.f),
			glm::vec3(0.f),
			glm::quat(),
			100.f,
			Renderer::HEIGHT,
			Renderer::LEFT,
			Renderer::BOTTOM_LEFT
		);
	}
}

void StudyInterface::begin()
{
	for (auto a : m_vfAngleConditions)
		for (auto d : m_vfDistanceConditions)
		{
			m_vExperimentConditions.push_back(std::tuple<float, float, int, float, glm::vec3>(m_BoolDistribution(m_Generator) ? a : -a, d, 90.f + m_AngleDistribution(m_Generator), 10.f, glm::vec3(0.f, 0.f, -10.f)));
			m_vExperimentConditions.push_back(std::tuple<float, float, int, float, glm::vec3>(m_BoolDistribution(m_Generator) ? a : -a, d, 90.f - m_AngleDistribution(m_Generator), 10.f, glm::vec3(0.f, 0.f, -10.f)));
		}


	//m_vExperimentConditions.push_back(std::tuple<float, float, int, float, glm::vec3>(0.f, 1.f, 90 + m_AngleDistribution(m_Generator), 5.f, glm::vec3(0.f)));
	//m_vExperimentConditions.push_back(std::tuple<float, float, int, float, glm::vec3>(0.f, 1.f, 90 - m_AngleDistribution(m_Generator), 5.f, glm::vec3(0.f)));
	//m_vExperimentConditions.push_back(std::tuple<float, float, int, float, glm::vec3>(0.f, 1.f, 90 + m_AngleDistribution(m_Generator), 5.f, glm::vec3(0.f, 0.f, -5.f)));
	//m_vExperimentConditions.push_back(std::tuple<float, float, int, float, glm::vec3>(0.f, 1.f, 90 - m_AngleDistribution(m_Generator), 5.f, glm::vec3(0.f, 0.f, -5.f)));
	//m_vExperimentConditions.push_back(std::tuple<float, float, int, float, glm::vec3>(0.f, 1.f, 90 + m_AngleDistribution(m_Generator), 10.f, glm::vec3(0.f)));
	//m_vExperimentConditions.push_back(std::tuple<float, float, int, float, glm::vec3>(0.f, 1.f, 90 - m_AngleDistribution(m_Generator), 10.f, glm::vec3(0.f)));
	//m_vExperimentConditions.push_back(std::tuple<float, float, int, float, glm::vec3>(0.f, 1.f, 90 + m_AngleDistribution(m_Generator), 10.f, glm::vec3(0.f, 0.f, -10.f)));
	//m_vExperimentConditions.push_back(std::tuple<float, float, int, float, glm::vec3>(0.f, 1.f, 90 - m_AngleDistribution(m_Generator), 10.f, glm::vec3(0.f, 0.f, -10.f)));
	//m_vExperimentConditions.push_back(std::tuple<float, float, int, float, glm::vec3>(0.f, 1.f, 90 + m_AngleDistribution(m_Generator), 30.f, glm::vec3(0.f)));
	//m_vExperimentConditions.push_back(std::tuple<float, float, int, float, glm::vec3>(0.f, 1.f, 90 - m_AngleDistribution(m_Generator), 30.f, glm::vec3(0.f)));
	//m_vExperimentConditions.push_back(std::tuple<float, float, int, float, glm::vec3>(0.f, 1.f, 90 + m_AngleDistribution(m_Generator), 30.f, glm::vec3(0.f, 0.f, -30.f)));
	//m_vExperimentConditions.push_back(std::tuple<float, float, int, float, glm::vec3>(0.f, 1.f, 90 - m_AngleDistribution(m_Generator), 30.f, glm::vec3(0.f, 0.f, -30.f)));
		

	std::random_shuffle(m_vExperimentConditions.begin(), m_vExperimentConditions.end());

	m_strLastResponse = std::string();

	m_bLockViewCOP = false;

	m_pHinge->setAngle(std::get<2>(m_vExperimentConditions.back()));
	m_pHinge->setLength(std::get<3>(m_vExperimentConditions.back()));
	m_pHinge->setPos(std::get<4>(m_vExperimentConditions.back()));

	std::stringstream ss;
	ss.precision(1);

	ss << std::fixed << -std::get<0>(m_vExperimentConditions.back());
	m_pSocket->send(ss.str() + "," + std::to_string(m_fMoveTime));
	m_tMoveStart = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(250);

	m_tStimulusStart = m_tMoveStart + std::chrono::milliseconds(static_cast<int>(m_fMoveTime * 1000.f));

	DataLogger::getInstance().setID(m_strNameBuffer);
	DataLogger::getInstance().openLog(m_strNameBuffer);
	DataLogger::getInstance().setHeader("trial,view.angle,view.dist,start.angle,hinge.length,hinge.z.pos,hinge.angle,response");
	DataLogger::getInstance().start(); 
}

void StudyInterface::next(bool stimulusDetected)
{
	std::string logEntry;
	logEntry += std::to_string((m_vfAngleConditions.size() * m_vfDistanceConditions.size()) - m_vExperimentConditions.size());
	logEntry += ",";
	logEntry += std::to_string(std::get<0>(m_vExperimentConditions.back()));
	logEntry += ",";
	logEntry += std::to_string(std::get<1>(m_vExperimentConditions.back()));
	logEntry += ",";
	logEntry += std::to_string(std::get<2>(m_vExperimentConditions.back()));
	logEntry += ",";
	logEntry += std::to_string(std::get<3>(m_vExperimentConditions.back()));
	logEntry += ",";
	logEntry += std::to_string(std::get<4>(m_vExperimentConditions.back()).z);
	logEntry += ",";
	logEntry += std::to_string(m_pHinge->getAngle());
	logEntry += ",";


	if (stimulusDetected) // perceived as acute
	{
		m_pHinge->setAngle(m_pHinge->getAngle() + m_fStepSize);

		logEntry += "acute";
		logEntry += ",";

		if (m_strLastResponse.compare("obtuse") == 0)
			m_nReversals--;		

		m_strLastResponse = "acute";
	}
	else // perceived as obtuse
	{
		m_pHinge->setAngle(m_pHinge->getAngle() - m_fStepSize);

		logEntry += "obtuse";
		logEntry += ",";

		if (m_strLastResponse.compare("acute") == 0)		
			m_nReversals--;

		m_strLastResponse = "obtuse";
	}

	DataLogger::getInstance().logMessage(logEntry);

	if (m_nReversals == 0)
	{
		m_nReversals = 11; // ignore the first
		m_vExperimentConditions.pop_back();

		if (m_vExperimentConditions.size() == 0)
			end();
		else
		{
			m_pHinge->setAngle(std::get<2>(m_vExperimentConditions.back()));
			m_pHinge->setLength(std::get<3>(m_vExperimentConditions.back()));
			m_pHinge->setPos(std::get<4>(m_vExperimentConditions.back()));

			std::stringstream ss;
			ss.precision(1);

			ss << std::fixed << -std::get<0>(m_vExperimentConditions.back());
			m_pSocket->send(ss.str() + "," + std::to_string(m_fMoveTime));
			m_tMoveStart = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(250);
			m_tStimulusStart = m_tMoveStart + std::chrono::milliseconds(static_cast<int>(m_fStimulusDelay * 1000.f));
		}
	}
	else
	{
		m_tStimulusStart = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(static_cast<int>(m_fStimulusDelay * 1000.f));
	}
}

void StudyInterface::end()
{
	m_bStudyMode = false;
	DataLogger::getInstance().closeLog();
}

glm::vec3 StudyInterface::getCOP()
{
	if (m_bStudyMode)
		return glm::vec3(glm::rotate(glm::mat4(), glm::radians(m_fCOPAngle), glm::vec3(0.f, 1.f, 0.f)) * glm::vec4(0.f, 0.f, m_fViewDist * std::get<1>(m_vExperimentConditions.back()), 1.f));
	else if (m_bLockViewCOP)
		return glm::vec3(glm::rotate(glm::mat4(), glm::radians(m_fViewAngle), glm::vec3(0.f, 1.f, 0.f)) * glm::vec4(0.f, 0.f, m_fViewDist, 1.f));
	else
		return glm::vec3(glm::rotate(glm::mat4(), glm::radians(m_fCOPAngle), glm::vec3(0.f, 1.f, 0.f)) * glm::vec4(0.f, 0.f, m_fCOPDist, 1.f));
}

float StudyInterface::getEyeSep()
{
	return m_fEyeSep;
}

void StudyInterface::receive(void * data)
{
	int eventData[2];

	memcpy(&eventData, data, sizeof(int) * 2);

	if (eventData[0] == GLFWInputBroadcaster::EVENT::KEY_DOWN)
	{
		if (eventData[1] == GLFW_KEY_F1 && m_bAddressEntryMode == false)
		{
			m_bAddressEntryMode = true;
			m_bPortEntryMode = false;
			m_bEyeSepEntryMode = false;
			m_bViewDistEntryMode = false;
			m_bViewAngleEntryMode = false;
			m_bMoveTimeEntryMode = false;
			m_bNameEntryMode = false;
		}

		if (eventData[1] == GLFW_KEY_F2 && m_bPortEntryMode == false)
		{
			m_bPortEntryMode = true;
			m_bAddressEntryMode = false;
			m_bEyeSepEntryMode = false;
			m_bViewDistEntryMode = false;
			m_bViewAngleEntryMode = false;
			m_bMoveTimeEntryMode = false;
			m_bNameEntryMode = false;
		}

		if (eventData[1] == GLFW_KEY_F3 && m_bEyeSepEntryMode == false)
		{
			m_bEyeSepEntryMode = true;
			m_bAddressEntryMode = false;
			m_bPortEntryMode = false;
			m_bViewDistEntryMode = false;
			m_bViewAngleEntryMode = false;
			m_bMoveTimeEntryMode = false;
			m_bNameEntryMode = false;
		}

		if (eventData[1] == GLFW_KEY_F4 && m_bViewDistEntryMode == false)
		{
			m_bViewDistEntryMode = true;
			m_bAddressEntryMode = false;
			m_bPortEntryMode = false;
			m_bEyeSepEntryMode = false;
			m_bViewAngleEntryMode = false;
			m_bMoveTimeEntryMode = false;
			m_bNameEntryMode = false;
		}

		if (eventData[1] == GLFW_KEY_F5 && m_bViewAngleEntryMode == false)
		{
			m_bViewAngleEntryMode = true;
			m_bAddressEntryMode = false;
			m_bPortEntryMode = false;
			m_bEyeSepEntryMode = false;
			m_bViewDistEntryMode = false;
			m_bMoveTimeEntryMode = false;
			m_bNameEntryMode = false;
		}

		if (eventData[1] == GLFW_KEY_F6 && m_bMoveTimeEntryMode == false)
		{
			m_bMoveTimeEntryMode = true;
			m_bAddressEntryMode = false;
			m_bPortEntryMode = false;
			m_bEyeSepEntryMode = false;
			m_bViewAngleEntryMode = false;
			m_bViewDistEntryMode = false;
			m_bNameEntryMode = false;
		}

		if (eventData[1] == GLFW_KEY_F7 && m_bNameEntryMode == false)
		{
			m_bNameEntryMode = true;
			m_bAddressEntryMode = false;
			m_bPortEntryMode = false;
			m_bEyeSepEntryMode = false;
			m_bViewAngleEntryMode = false;
			m_bMoveTimeEntryMode = false;
			m_bViewDistEntryMode = false;
		}

		if (eventData[1] == GLFW_KEY_F8)
		{
			m_bLockViewCOP = !m_bLockViewCOP;
		}

		if (m_bAddressEntryMode)
		{
			if (eventData[1] >= GLFW_KEY_0 && eventData[1] <= GLFW_KEY_9)
				m_strAddressBuffer += std::to_string(eventData[1] - GLFW_KEY_0);

			if (eventData[1] == GLFW_KEY_PERIOD && std::count(m_strAddressBuffer.begin(), m_strAddressBuffer.end(), '.') < 3)
				m_strAddressBuffer += ".";

			if (eventData[1] == GLFW_KEY_BACKSPACE && m_strAddressBuffer.length() > 0)
				m_strAddressBuffer.pop_back();

			if (eventData[1] == GLFW_KEY_ENTER && m_strAddressBuffer.length() > 0)
				m_bAddressEntryMode = false;
		}

		if (m_bPortEntryMode)
		{
			if (eventData[1] >= GLFW_KEY_0 && eventData[1] <= GLFW_KEY_9)
				m_strPortBuffer += std::to_string(eventData[1] - GLFW_KEY_0);

			if (eventData[1] == GLFW_KEY_BACKSPACE && m_strPortBuffer.length() > 0)
				m_strPortBuffer.pop_back();

			if (eventData[1] == GLFW_KEY_ENTER && m_strPortBuffer.length() > 0)
				m_bPortEntryMode = false;
		}

		if (m_bEyeSepEntryMode)
		{
			if (eventData[1] >= GLFW_KEY_0 && eventData[1] <= GLFW_KEY_9)
				m_strEyeSepBuffer += std::to_string(eventData[1] - GLFW_KEY_0);

			if (eventData[1] == GLFW_KEY_PERIOD && m_strEyeSepBuffer.find('.') == std::string::npos)
				m_strEyeSepBuffer += ".";

			if (eventData[1] == GLFW_KEY_BACKSPACE && m_strEyeSepBuffer.length() > 0)
				m_strEyeSepBuffer.pop_back();

			if (eventData[1] == GLFW_KEY_ENTER && m_strEyeSepBuffer.length() > 0)
			{
				m_bEyeSepEntryMode = false;
				m_fEyeSep = std::stof(m_strEyeSepBuffer);
			}
		}

		if (m_bViewDistEntryMode)
		{
			if (eventData[1] >= GLFW_KEY_0 && eventData[1] <= GLFW_KEY_9)
				m_strViewDistBuffer += std::to_string(eventData[1] - GLFW_KEY_0);

			if (eventData[1] == GLFW_KEY_PERIOD && m_strViewDistBuffer.find('.') == std::string::npos)
				m_strViewDistBuffer += ".";

			if (eventData[1] == GLFW_KEY_BACKSPACE && m_strViewDistBuffer.length() > 0)
				m_strViewDistBuffer.pop_back();

			if (eventData[1] == GLFW_KEY_ENTER && m_strViewDistBuffer.length() > 0)
			{
				m_bViewDistEntryMode = false;
				m_fViewDist = m_fCOPDist = std::stof(m_strViewDistBuffer);
			}
		}

		if (m_bViewAngleEntryMode)
		{
			if (eventData[1] >= GLFW_KEY_0 && eventData[1] <= GLFW_KEY_9)
				m_strViewAngleBuffer += std::to_string(eventData[1] - GLFW_KEY_0);

			if (eventData[1] == GLFW_KEY_PERIOD && m_strViewAngleBuffer.find('.') == std::string::npos)
				m_strViewAngleBuffer += ".";

			if (eventData[1] == GLFW_KEY_MINUS)
			{
				if (m_strViewAngleBuffer[0] == '-')
						m_strViewAngleBuffer.erase(0, 1);
				else
					m_strViewAngleBuffer.insert(0, 1, '-');
			}

			if (eventData[1] == GLFW_KEY_BACKSPACE && m_strViewAngleBuffer.length() > 0)
				m_strViewAngleBuffer.pop_back();

			if (eventData[1] == GLFW_KEY_ENTER && m_strViewAngleBuffer.length() > 0)
			{
				m_bViewAngleEntryMode = false;
				m_fLastAngle = m_fViewAngle;
				m_fTargetAngle = std::stof(m_strViewAngleBuffer);

				// The display angle should be the negative of the viewing angle since our viewpoint is fixed
				std::string commandAngle = m_strViewAngleBuffer;
				if (commandAngle[0] == '-')
					commandAngle.erase(0, 1);
				else
					commandAngle.insert(0, 1, '-');

				m_pSocket->send(commandAngle + "," + std::to_string(m_fMoveTime));
				m_tMoveStart = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(250);
			}
		}

		if (m_bMoveTimeEntryMode)
		{
			if (eventData[1] >= GLFW_KEY_0 && eventData[1] <= GLFW_KEY_9)
				m_strMoveTimeBuffer += std::to_string(eventData[1] - GLFW_KEY_0);

			if (eventData[1] == GLFW_KEY_PERIOD && m_strMoveTimeBuffer.find('.') == std::string::npos)
				m_strMoveTimeBuffer += ".";

			if (eventData[1] == GLFW_KEY_BACKSPACE && m_strMoveTimeBuffer.length() > 0)
				m_strMoveTimeBuffer.pop_back();

			if (eventData[1] == GLFW_KEY_ENTER && m_strMoveTimeBuffer.length() > 0)
			{
				m_bMoveTimeEntryMode = false;
				m_fMoveTime = std::stof(m_strMoveTimeBuffer);
			}
		}

		if (m_bNameEntryMode)
		{
			if (eventData[1] >= GLFW_KEY_A && eventData[1] <= GLFW_KEY_Z)
				m_strNameBuffer += glfwGetKeyName(eventData[1], 0);

			if (eventData[1] == GLFW_KEY_BACKSPACE && m_strNameBuffer.length() > 0)
				m_strNameBuffer.pop_back();

			if (eventData[1] == GLFW_KEY_ENTER && m_strNameBuffer.length() > 0)
			{
				m_bNameEntryMode = false;
				DataLogger::getInstance().setID(m_strNameBuffer);
			}
		}

		if (eventData[1] == GLFW_KEY_C && m_strAddressBuffer.length() > 0 && m_strPortBuffer.length() > 0)
			if (m_pSocket->connect(m_strAddressBuffer, std::stoi(m_strPortBuffer)))
				m_pSocket->send("0,2.5");			
		
		if (eventData[1] == GLFW_KEY_R && !m_bNameEntryMode)
			reset();

		if (eventData[1] == GLFW_KEY_HOME && !m_bStudyMode)
		{
			m_bStudyMode = true;
			begin();
		}

		if (eventData[1] == GLFW_KEY_SPACE && m_bStudyMode)
			next(true);

		if (eventData[1] == GLFW_KEY_KP_ENTER && m_bStudyMode)
			next(false);
	}

	if (eventData[0] == GLFWInputBroadcaster::EVENT::KEY_DOWN || eventData[0] == GLFWInputBroadcaster::EVENT::KEY_HOLD)
	{
		float angleDelta = 1.f;
		float distDelta = 1.f;

		if (eventData[1] == GLFW_KEY_LEFT)
			m_fCOPAngle -= angleDelta;
		if (eventData[1] == GLFW_KEY_RIGHT)
			m_fCOPAngle += angleDelta;

		if (eventData[1] == GLFW_KEY_UP)
			m_fCOPDist = std::max(m_fCOPDist - distDelta, 0.f);
		if (eventData[1] == GLFW_KEY_DOWN)
			m_fCOPDist += distDelta; 
		
		if (eventData[1] == GLFW_KEY_LEFT_BRACKET)
			m_pHinge->setAngle(m_pHinge->getAngle() + 1.f);
		if (eventData[1] == GLFW_KEY_RIGHT_BRACKET)
			m_pHinge->setAngle(m_pHinge->getAngle() - 1.f);
	}
}

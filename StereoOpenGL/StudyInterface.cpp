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
{
}


StudyInterface::~StudyInterface()
{
	if (m_pSocket)
		delete m_pSocket;
}

void StudyInterface::init(glm::ivec2 screenDims, float screenDiag)
{
	DataLogger::getInstance().setID("drewtest");
	DataLogger::getInstance().setLogDirectory("logs");

	m_ivec2Screen = screenDims;

	float sizer = screenDiag / sqrt(glm::dot(glm::vec2(screenDims), glm::vec2(screenDims)));

	m_vec2Screen = glm::vec2(screenDims) * sizer;

	reset();

	if (m_pSocket == NULL)
		m_pSocket = new WinsockClient();

	if (m_pHinge == NULL)
		m_pHinge = new Hinge(m_vec2Screen.y, 90.f);
}

void StudyInterface::reset()
{
	m_bAddressEntryMode = false;
	m_bPortEntryMode = false;
	m_bEyeSepEntryMode = false;
	m_bViewDistEntryMode = false;
	m_bViewAngleEntryMode = false;
	m_bMoveTimeEntryMode = false;

	m_fViewAngle = 0.f;
	m_fViewDist = 57.f;
	m_fEyeSep = 6.7;

	m_fCOPAngle = m_fViewAngle;
	m_fCOPDist = m_fViewDist;

	m_fMinStep = 2.f;
	m_fStepSize = m_fMinStep;

	m_vfAngleConditions = { 0.f, 10.f, 30.f, 60.f };
	m_vfDistanceConditions = { 1.f, 0.5f, 2.f };

	m_fMoveTime = 5.f;

	m_strAddressBuffer = "192.168.";
	m_strPortBuffer = "5005";
	m_strEyeSepBuffer = "6.70";
	m_strViewDistBuffer = "57.0";
	m_strViewAngleBuffer = "0";
	m_strMoveTimeBuffer = "5.0";

	m_tMoveStart = std::chrono::high_resolution_clock::time_point();
}

void StudyInterface::update()
{
	using clock = std::chrono::high_resolution_clock;
	auto tick = clock::now();
	float elapsed = std::chrono::duration_cast<std::chrono::seconds>(clock::now() - m_tMoveStart).count();

	if (elapsed < m_fMoveTime)
	{
		float ratio = elapsed / m_fMoveTime;
		m_fCOPAngle = m_fLastAngle + (m_fViewAngle - m_fLastAngle) * ratio;
	}
	else
	{
		m_fCOPAngle = m_fViewAngle;
	}
}

void StudyInterface::draw()
{
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
}

glm::vec3 StudyInterface::getCOP()
{
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
		if (eventData[1] == GLFW_KEY_ESCAPE)
			;

		if (eventData[1] == GLFW_KEY_F1 && m_bAddressEntryMode == false)
		{
			m_bAddressEntryMode = true;
			m_bPortEntryMode = false;
			m_bEyeSepEntryMode = false;
			m_bViewDistEntryMode = false;
			m_bViewAngleEntryMode = false;
			m_bMoveTimeEntryMode = false;
		}

		if (eventData[1] == GLFW_KEY_F2 && m_bPortEntryMode == false)
		{
			m_bPortEntryMode = true;
			m_bAddressEntryMode = false;
			m_bEyeSepEntryMode = false;
			m_bViewDistEntryMode = false;
			m_bViewAngleEntryMode = false;
			m_bMoveTimeEntryMode = false;
		}

		if (eventData[1] == GLFW_KEY_F3 && m_bEyeSepEntryMode == false)
		{
			m_bEyeSepEntryMode = true;
			m_bAddressEntryMode = false;
			m_bPortEntryMode = false;
			m_bViewDistEntryMode = false;
			m_bViewAngleEntryMode = false;
			m_bMoveTimeEntryMode = false;
		}

		if (eventData[1] == GLFW_KEY_F4 && m_bViewDistEntryMode == false)
		{
			m_bViewDistEntryMode = true;
			m_bAddressEntryMode = false;
			m_bPortEntryMode = false;
			m_bEyeSepEntryMode = false;
			m_bViewAngleEntryMode = false;
			m_bMoveTimeEntryMode = false;
		}

		if (eventData[1] == GLFW_KEY_F5 && m_bViewAngleEntryMode == false)
		{
			m_bViewAngleEntryMode = true;
			m_bAddressEntryMode = false;
			m_bPortEntryMode = false;
			m_bEyeSepEntryMode = false;
			m_bViewDistEntryMode = false;
			m_bMoveTimeEntryMode = false;
		}

		if (eventData[1] == GLFW_KEY_F6 && m_bMoveTimeEntryMode == false)
		{
			m_bMoveTimeEntryMode = true;
			m_bAddressEntryMode = false;
			m_bPortEntryMode = false;
			m_bEyeSepEntryMode = false;
			m_bViewAngleEntryMode = false;
			m_bViewDistEntryMode = false;
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
				m_fViewDist = std::stof(m_strViewDistBuffer);
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
				m_fViewAngle = std::stof(m_strViewAngleBuffer);
				m_pSocket->send(m_strViewAngleBuffer + "," + std::to_string(m_fMoveTime));
				m_tMoveStart = std::chrono::high_resolution_clock::now();
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

		if (eventData[1] == GLFW_KEY_C && m_strAddressBuffer.length() > 0 && m_strPortBuffer.length() > 0)
			if (m_pSocket->connect(m_strAddressBuffer, std::stoi(m_strPortBuffer)))
				m_pSocket->send("0,2.5");			
		
		if (eventData[1] == GLFW_KEY_R)
			reset();
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

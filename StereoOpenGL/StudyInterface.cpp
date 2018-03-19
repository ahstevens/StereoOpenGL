#include "StudyInterface.h"

#include "Renderer.h"
#include "DataLogger.h"

#include <GLFW/glfw3.h>

#include <gtc/matrix_transform.hpp>

StudyInterface::StudyInterface()
	: m_pSocket(NULL)
	, m_bAddressEntryMode(false)
	, m_bPortEntryMode(false)
	, m_bEyeSepEntryMode(false)
	, m_bViewDistEntryMode(false)
	, m_bViewAngleEntryMode(false)
{
}


StudyInterface::~StudyInterface()
{
	if (m_pSocket)
		delete m_pSocket;
}

void StudyInterface::init(glm::ivec2 screenDims)
{
	DataLogger::getInstance().setID("drewtest");
	DataLogger::getInstance().setLogDirectory("logs");

	m_ivec2Screen = screenDims;

	reset();

	if (m_pSocket == NULL)
		m_pSocket = new WinsockClient();
}

void StudyInterface::reset()
{
	m_bAddressEntryMode = false;
	m_bPortEntryMode = false;
	m_bEyeSepEntryMode = false;
	m_bViewDistEntryMode = false;
	m_bViewAngleEntryMode = false;

	m_fCOPDist = 57.f;
	m_fCOPAngle = 0.f;
	m_fCOPEyeSep = 6.7;

	m_fViewAngle = 0.f;
	m_fViewDist = 57.f;

	m_fMinStep = 2.f;
	m_fStepSize = m_fMinStep;

	m_fMoveTime = 5.f;

	m_strAddressBuffer.clear();
	m_strPortBuffer.clear();

	std::stringstream ss;
	ss.precision(2);

	ss << std::fixed << m_fCOPEyeSep;

	m_strEyeSepBuffer = ss.str();
}

void StudyInterface::update()
{
}

void StudyInterface::draw()
{

	if (m_bAddressEntryMode)
	{
		Renderer::getInstance().drawUIText(
			"Server IP: " + m_strAddressBuffer + "_",
			glm::vec4(1.f),
			glm::vec3(m_ivec2Screen.x / 2.f, m_ivec2Screen.y / 2.f, 0.f),
			glm::quat(),
			100.f,
			Renderer::HEIGHT,
			Renderer::CENTER,
			Renderer::CENTER_MIDDLE
		);
	}

	if (m_bPortEntryMode)
	{
		Renderer::getInstance().drawUIText(
			"Server Port: " + m_strPortBuffer + "_",
			glm::vec4(1.f),
			glm::vec3(m_ivec2Screen.x / 2.f, m_ivec2Screen.y / 2.f, 0.f),
			glm::quat(),
			100.f,
			Renderer::HEIGHT,
			Renderer::CENTER,
			Renderer::CENTER_MIDDLE
		);
	}

	if (m_bEyeSepEntryMode)
	{
		Renderer::getInstance().drawUIText(
			"Eye Separation (cm): " + m_strEyeSepBuffer + "_",
			glm::vec4(1.f),
			glm::vec3(m_ivec2Screen.x / 2.f, m_ivec2Screen.y / 2.f, 0.f),
			glm::quat(),
			100.f,
			Renderer::HEIGHT,
			Renderer::CENTER,
			Renderer::CENTER_MIDDLE
		);
	}

	if (m_bViewDistEntryMode)
	{
		Renderer::getInstance().drawUIText(
			"View Distance (cm): " + m_strViewDistBuffer + "_",
			glm::vec4(1.f),
			glm::vec3(m_ivec2Screen.x / 2.f, m_ivec2Screen.y / 2.f, 0.f),
			glm::quat(),
			100.f,
			Renderer::HEIGHT,
			Renderer::CENTER,
			Renderer::CENTER_MIDDLE
		);
	}

	if (m_bViewAngleEntryMode)
	{
		Renderer::getInstance().drawUIText(
			"View Angle (deg): " + m_strViewAngleBuffer + "_",
			glm::vec4(1.f),
			glm::vec3(m_ivec2Screen.x / 2.f, m_ivec2Screen.y / 2.f, 0.f),
			glm::quat(),
			100.f,
			Renderer::HEIGHT,
			Renderer::CENTER,
			Renderer::CENTER_MIDDLE
		);
	}
}

glm::vec3 StudyInterface::getCOP()
{
	return glm::vec3(glm::rotate(glm::mat4(), glm::radians(m_fCOPAngle), glm::vec3(0.f, 1.f, 0.f)) * glm::vec4(0.f, 0.f, m_fCOPDist, 1.f));
}

float StudyInterface::getEyeSep()
{
	return m_fCOPEyeSep;
}

void StudyInterface::receive(void * data)
{
	int eventData[2];

	memcpy(&eventData, data, sizeof(int) * 2);

	if (eventData[0] == GLFWInputBroadcaster::EVENT::KEY_DOWN)
	{
		if (eventData[1] == GLFW_KEY_ESCAPE)
			;

		if (eventData[1] == GLFW_KEY_I && m_bAddressEntryMode == false)
		{
			m_bAddressEntryMode = true;
			m_bPortEntryMode = false;
			m_bEyeSepEntryMode = false;
			m_bViewDistEntryMode = false;
			m_bViewAngleEntryMode = false;
		}

		if (eventData[1] == GLFW_KEY_P && m_bPortEntryMode == false)
		{
			m_bPortEntryMode = true;
			m_bAddressEntryMode = false;
			m_bEyeSepEntryMode = false;
			m_bViewDistEntryMode = false;
			m_bViewAngleEntryMode = false;
		}

		if (eventData[1] == GLFW_KEY_E && m_bEyeSepEntryMode == false)
		{
			m_bEyeSepEntryMode = true;
			m_bAddressEntryMode = false;
			m_bPortEntryMode = false;
			m_bViewDistEntryMode = false;
			m_bViewAngleEntryMode = false;
		}

		if (eventData[1] == GLFW_KEY_D && m_bViewDistEntryMode == false)
		{
			m_bViewDistEntryMode = true;
			m_bAddressEntryMode = false;
			m_bPortEntryMode = false;
			m_bEyeSepEntryMode = false;
			m_bViewAngleEntryMode = false;
		}

		if (eventData[1] == GLFW_KEY_A && m_bViewAngleEntryMode == false)
		{
			m_bViewAngleEntryMode = true;
			m_bAddressEntryMode = false;
			m_bPortEntryMode = false;
			m_bEyeSepEntryMode = false;
			m_bViewDistEntryMode = false;
		}

		if (m_bAddressEntryMode)
		{
			if (eventData[1] >= GLFW_KEY_0 && eventData[1] <= GLFW_KEY_9)
				m_strAddressBuffer += std::to_string(eventData[1] - GLFW_KEY_0);

			if (eventData[1] == GLFW_KEY_PERIOD)
				m_strAddressBuffer += ".";

			if (eventData[1] == GLFW_KEY_BACKSPACE && m_strAddressBuffer.length() > 0)
				m_strAddressBuffer.pop_back();

			if (eventData[1] == GLFW_KEY_ENTER)
				m_bAddressEntryMode = false;
		}

		if (m_bPortEntryMode)
		{
			if (eventData[1] >= GLFW_KEY_0 && eventData[1] <= GLFW_KEY_9)
				m_strPortBuffer += std::to_string(eventData[1] - GLFW_KEY_0);

			if (eventData[1] == GLFW_KEY_BACKSPACE && m_strPortBuffer.length() > 0)
				m_strPortBuffer.pop_back();

			if (eventData[1] == GLFW_KEY_ENTER)
				m_bPortEntryMode = false;
		}

		if (m_bEyeSepEntryMode)
		{
			if (eventData[1] >= GLFW_KEY_0 && eventData[1] <= GLFW_KEY_9)
				m_strEyeSepBuffer += std::to_string(eventData[1] - GLFW_KEY_0);

			if (eventData[1] == GLFW_KEY_PERIOD)
				m_strEyeSepBuffer += ".";

			if (eventData[1] == GLFW_KEY_BACKSPACE && m_strEyeSepBuffer.length() > 0)
				m_strEyeSepBuffer.pop_back();

			if (eventData[1] == GLFW_KEY_ENTER)
			{
				m_bEyeSepEntryMode = false;
				m_fCOPEyeSep = std::stof(m_strEyeSepBuffer);
			}
		}

		if (m_bViewDistEntryMode)
		{
			if (eventData[1] >= GLFW_KEY_0 && eventData[1] <= GLFW_KEY_9)
				m_strViewDistBuffer += std::to_string(eventData[1] - GLFW_KEY_0);

			if (eventData[1] == GLFW_KEY_PERIOD)
				m_strViewDistBuffer += ".";

			if (eventData[1] == GLFW_KEY_BACKSPACE && m_strViewDistBuffer.length() > 0)
				m_strViewDistBuffer.pop_back();

			if (eventData[1] == GLFW_KEY_ENTER)
			{
				m_bViewDistEntryMode = false;
				m_fViewDist = std::stof(m_strViewDistBuffer);
			}
		}

		if (m_bViewAngleEntryMode)
		{
			if (eventData[1] >= GLFW_KEY_0 && eventData[1] <= GLFW_KEY_9)
				m_strViewAngleBuffer += std::to_string(eventData[1] - GLFW_KEY_0);

			if (eventData[1] == GLFW_KEY_PERIOD)
				m_strViewAngleBuffer += ".";

			if (eventData[1] >= GLFW_KEY_MINUS)
			{
				auto pos = m_strViewAngleBuffer.find('-');
				if (pos != std::string::npos)
						m_strViewAngleBuffer.erase(pos);
				else
					m_strViewAngleBuffer.insert(0, 1, '-');
			}

			if (eventData[1] == GLFW_KEY_BACKSPACE && m_strViewAngleBuffer.length() > 0)
				m_strViewAngleBuffer.pop_back();

			if (eventData[1] == GLFW_KEY_ENTER)
			{
				m_bViewAngleEntryMode = false;
				m_fViewAngle = std::stof(m_strViewAngleBuffer);
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
	}
}

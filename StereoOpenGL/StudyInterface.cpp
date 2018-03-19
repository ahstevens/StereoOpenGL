#include "StudyInterface.h"

#include "Renderer.h"
#include "DataLogger.h"

#include <GLFW/glfw3.h>

StudyInterface::StudyInterface()
	: m_pSocket(NULL)
	, m_bAddressEntryMode(false)
	, m_bPortEntryMode(false)
{
}


StudyInterface::~StudyInterface()
{
	if (m_pSocket)
		delete m_pSocket;
}

void StudyInterface::init()
{
	DataLogger::getInstance().setID("drewtest");
	DataLogger::getInstance().setLogDirectory("logs");

	reset();

	if (m_pSocket == NULL)
		m_pSocket = new WinsockClient();
}

void StudyInterface::reset()
{
	m_bAddressEntryMode = false;
	m_bPortEntryMode = false;

	m_fCOPDistance = 67.f;
	m_fCOPAngle = 0.f;
	m_fViewDistance = 67.f;
	m_fViewAngle = 0.f;

	m_fMinStep = 2.f;
	m_fStepSize = m_fMinStep;
}

void StudyInterface::update()
{
}

void StudyInterface::draw()
{

	if (m_bAddressEntryMode)
	{
		Renderer::getInstance().drawUIText(
			m_strAddressBuffer,
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
			m_strPortBuffer,
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

void StudyInterface::receive(void * data)
{
	int eventData[2];

	memcpy(&eventData, data, sizeof(int) * 2);

	if (eventData[0] == GLFWInputBroadcaster::EVENT::KEY_DOWN)
	{
		if (eventData[1] == GLFW_KEY_ESCAPE)
			;

		if (eventData[1] == GLFW_KEY_A && m_bAddressEntryMode == false)
		{
			m_bAddressEntryMode = true;
			m_bPortEntryMode = false;
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

		if (eventData[1] == GLFW_KEY_P && m_bPortEntryMode == false)
		{
			m_bPortEntryMode = true;
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

		if (eventData[1] == GLFW_KEY_C)
			m_pSocket->connect(m_strAddressBuffer, std::stoi(m_strPortBuffer));
	}
}

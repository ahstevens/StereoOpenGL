#include "StudyInterface.h"

#include "Renderer.h"
#include "DataLogger.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <gtc/matrix_transform.hpp>

StudyInterface::StudyInterface()
	: m_pSocket(NULL)
	, m_pHinge(NULL)
	, m_pEditParam(NULL)
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
	m_bStudyMode = false;

	m_bLockViewCOP = false;

	m_strServerAddress = "192.168.";
	m_uiServerPort = 5005;

	m_strName = "NONAME";

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

	m_pEditParam = NULL;

	m_vParams.push_back({ "Server IP" , m_strServerAddress, STUDYPARAM_NUMERIC | STUDYPARAM_DECIMAL | STUDYPARAM_IP });
	m_vParams.push_back({ "Server Port" , std::to_string(m_uiServerPort), STUDYPARAM_NUMERIC });
	m_vParams.push_back({ "Eye Separation (cm)" , "6.70", STUDYPARAM_NUMERIC | STUDYPARAM_DECIMAL });
	m_vParams.push_back({ "View Distance (cm)" , "57.0", STUDYPARAM_NUMERIC | STUDYPARAM_DECIMAL });
	m_vParams.push_back({ "View Angle (deg)" , "0", STUDYPARAM_NUMERIC | STUDYPARAM_POSNEG });
	m_vParams.push_back({ "Display Move Time (sec)" , "5.0", STUDYPARAM_NUMERIC | STUDYPARAM_DECIMAL });
	m_vParams.push_back({ "Name" , m_strName, STUDYPARAM_ALPHA });
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

	if (m_pEditParam)
	{
		Renderer::getInstance().drawUIText(
			m_pEditParam->desc + ": " + m_pEditParam->buf + "_",
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
			m_vExperimentConditions.push_back({ m_BoolDistribution(m_Generator) ? a : -a, d, 90 + m_AngleDistribution(m_Generator), 10.f, glm::vec3(0.f, 0.f, -10.f) });
			m_vExperimentConditions.push_back({ m_BoolDistribution(m_Generator) ? a : -a, d, 90 - m_AngleDistribution(m_Generator), 10.f, glm::vec3(0.f, 0.f, -10.f) });
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

	m_pHinge->setAngle(m_vExperimentConditions.back().startAngle);
	m_pHinge->setLength(m_vExperimentConditions.back().hingeLen);
	m_pHinge->setPos(m_vExperimentConditions.back().hingePos);

	std::stringstream ss;
	ss.precision(1);

	ss << std::fixed << -m_vExperimentConditions.back().viewAngle;
	m_pSocket->send(ss.str() + "," + std::to_string(m_fMoveTime));
	m_tMoveStart = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(250);

	m_tStimulusStart = m_tMoveStart + std::chrono::milliseconds(static_cast<int>((m_fMoveTime + m_fStimulusDelay) * 1000.f));

	DataLogger::getInstance().setID(m_strName);
	DataLogger::getInstance().openLog(m_strName);
	DataLogger::getInstance().setHeader("trial,view.angle,view.dist,start.angle,hinge.length,hinge.z.pos,hinge.angle,response");
	DataLogger::getInstance().start(); 
}

void StudyInterface::next(bool stimulusDetected)
{
	std::string logEntry;
	logEntry += std::to_string((m_vfAngleConditions.size() * m_vfDistanceConditions.size()) - m_vExperimentConditions.size());
	logEntry += ",";
	logEntry += std::to_string(m_vExperimentConditions.back().viewAngle);
	logEntry += ",";
	logEntry += std::to_string(m_vExperimentConditions.back().viewDist);
	logEntry += ",";
	logEntry += std::to_string(m_vExperimentConditions.back().startAngle);
	logEntry += ",";
	logEntry += std::to_string(m_vExperimentConditions.back().hingeLen);
	logEntry += ",";										 
	logEntry += std::to_string(m_vExperimentConditions.back().hingePos.z);
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
			m_pHinge->setAngle(m_vExperimentConditions.back().startAngle);
			m_pHinge->setLength(m_vExperimentConditions.back().hingeLen);
			m_pHinge->setPos(m_vExperimentConditions.back().hingePos);

			std::stringstream ss;
			ss.precision(1);

			ss << std::fixed << -m_vExperimentConditions.back().viewAngle;
			m_pSocket->send(ss.str() + "," + std::to_string(m_fMoveTime));
			m_tMoveStart = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(250);
			m_tStimulusStart = m_tMoveStart + std::chrono::milliseconds(static_cast<int>((m_fMoveTime + m_fStimulusDelay) * 1000.f));
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
		return glm::vec3(glm::rotate(glm::mat4(), glm::radians(m_fCOPAngle), glm::vec3(0.f, 1.f, 0.f)) * glm::vec4(0.f, 0.f, m_fViewDist * m_vExperimentConditions.back().viewDist, 1.f));
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
		if (m_bStudyMode)
		{
			if (eventData[1] == GLFW_KEY_SPACE)
				next(true);

			if (eventData[1] == GLFW_KEY_KP_ENTER)
				next(false);
		}
		else
		{
			if (m_pEditParam)
			{
				if (eventData[1] == GLFW_KEY_BACKSPACE && m_pEditParam->buf.length() > 0)
					m_pEditParam->buf.pop_back();

				if (m_pEditParam->format & STUDYPARAM_NUMERIC)
				{
					if (eventData[1] >= GLFW_KEY_0 && eventData[1] <= GLFW_KEY_9)
						m_pEditParam->buf += std::to_string(eventData[1] - GLFW_KEY_0);
				}

				if (m_pEditParam->format & STUDYPARAM_ALPHA)
				{
					if (eventData[1] >= GLFW_KEY_A && eventData[1] <= GLFW_KEY_Z)
						m_pEditParam->buf += glfwGetKeyName(eventData[1], 0);
				}

				if (m_pEditParam->format & STUDYPARAM_POSNEG)
				{
					if (eventData[1] == GLFW_KEY_MINUS)
					{
						if (m_pEditParam->buf[0] == '-')
							m_pEditParam->buf.erase(0, 1);
						else
							m_pEditParam->buf.insert(0, 1, '-');
					}
				}

				if (m_pEditParam->format & STUDYPARAM_DECIMAL)
				{
					if (eventData[1] == GLFW_KEY_PERIOD)
					{
						auto decimalCount = std::count(m_pEditParam->buf.begin(), m_pEditParam->buf.end(), '.');

						if (decimalCount == 0 || ((m_pEditParam->format & STUDYPARAM_IP) && decimalCount < 3))
							m_pEditParam->buf += ".";
					}
				}


				if (eventData[1] == GLFW_KEY_ENTER && m_pEditParam->buf.length() > 0)
				{
					Renderer::getInstance().showMessage(m_pEditParam->desc + " set to " + m_pEditParam->buf);

					if (m_pEditParam->desc.compare("Server IP") == 0)
					{
						m_strServerAddress = m_pEditParam->buf;
					}

					if (m_pEditParam->desc.compare("Server Port") == 0)
					{
						m_uiServerPort = std::stoi(m_pEditParam->buf);
					}

					if (m_pEditParam->desc.compare("Eye Separation (cm)") == 0)
					{
						m_fEyeSep = std::stof(m_pEditParam->buf);
					}

					if (m_pEditParam->desc.compare("View Distance (cm)") == 0)
					{
						m_fViewDist = m_fCOPDist = std::stof(m_pEditParam->buf);
					}

					if (m_pEditParam->desc.compare("View Angle (deg)") == 0)
					{
						m_fLastAngle = m_fViewAngle;
						m_fTargetAngle = std::stof(m_pEditParam->buf);

						// The display angle should be the negative of the viewing angle since our viewpoint is fixed
						std::string commandAngle = m_pEditParam->buf;
						if (commandAngle[0] == '-')
							commandAngle.erase(0, 1);
						else
							commandAngle.insert(0, 1, '-');

						m_pSocket->send(commandAngle + "," + std::to_string(m_fMoveTime));
						m_tMoveStart = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(250);
					}

					if (m_pEditParam->desc.compare("Display Move Time (sec)") == 0)
					{
						m_fMoveTime = std::stof(m_pEditParam->buf);
					}

					if (m_pEditParam->desc.compare("Name") == 0)
					{
						m_strName = m_pEditParam->buf;
					}

					m_pEditParam = NULL;
				}
			}

			if (eventData[1] == GLFW_KEY_F1 && !m_pEditParam)
			{
				m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
					return p.desc.compare("Server IP") == 0;
				}));
			}

			if (eventData[1] == GLFW_KEY_F2 && !m_pEditParam)
			{
				m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
					return p.desc.compare("Server Port") == 0;
				}));
			}

			if (eventData[1] == GLFW_KEY_F3 && !m_pEditParam)
			{
				m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
					return p.desc.compare("Eye Separation (cm)") == 0;
				}));
			}

			if (eventData[1] == GLFW_KEY_F4 && !m_pEditParam)
			{
				m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
					return p.desc.compare("View Distance (cm)") == 0;
				}));
			}

			if (eventData[1] == GLFW_KEY_F5 && !m_pEditParam)
			{
				m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
					return p.desc.compare("View Angle (deg)") == 0;
				}));
			}

			if (eventData[1] == GLFW_KEY_F6 && !m_pEditParam)
			{
				m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
					return p.desc.compare("Display Move Time (sec)") == 0;
				}));
			}

			if (eventData[1] == GLFW_KEY_F7 && !m_pEditParam)
			{
				m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
					return p.desc.compare("Name") == 0;
				}));
			}

			if (eventData[1] == GLFW_KEY_F8)
			{
				m_bLockViewCOP = !m_bLockViewCOP;
				std::string msg("COP and View Point ");
				msg += (m_bLockViewCOP ? "locked" : "unlocked");
				Renderer::getInstance().showMessage(msg);
			}

			if (eventData[1] == GLFW_KEY_F9)
			{
				if (m_pSocket->connect(m_strServerAddress, m_uiServerPort))
				{
					m_pSocket->send("0,2.5");
					Renderer::getInstance().showMessage("Connected to " + m_strServerAddress + " port " + std::to_string(m_uiServerPort) + " successfully!");
				}
				else
					Renderer::getInstance().showMessage("ERROR! Could not connect to server at " + m_strServerAddress + " port " + std::to_string(m_uiServerPort) + "!");
			}

			if (eventData[1] == GLFW_KEY_R && !m_pEditParam)
				reset();

			if (eventData[1] == GLFW_KEY_HOME)
			{
				m_bStudyMode = true;
				begin();
			}
		}
	}

	if (eventData[0] == GLFWInputBroadcaster::EVENT::KEY_DOWN || eventData[0] == GLFWInputBroadcaster::EVENT::KEY_HOLD)
	{
		float angleDelta = 1.f;
		float distDelta = 1.f;

		if (!m_bStudyMode)
		{
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
}

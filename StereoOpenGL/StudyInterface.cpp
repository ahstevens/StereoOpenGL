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
	, m_bPaused(false)
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
	{
		m_pSocket->send("0,1");
		delete m_pSocket;
	}

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
	m_bPaused = false;

	m_bLockViewCOP = false;

	m_strServerAddress = "192.168.";
	m_uiServerPort = 5005;

	m_strName = "NONAME";

	m_fCurrentScreenAngle = 0.f;

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

	m_vfAngleConditions = { 0.f, 15.f, 30.f };
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


	if (m_SocketFuture.valid() && m_SocketFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
	{
		if (m_SocketFuture.get())
		{
			if (m_pSocket->send("0,2.5"))
				m_fCurrentScreenAngle = 0.f;

			Renderer::getInstance().showMessage("Successfully connected to " + m_strServerAddress + " port " + std::to_string(m_uiServerPort) + " successfully!");
		}
		else
			Renderer::getInstance().showMessage("ERROR! Could not connect to server at " + m_strServerAddress + " port " + std::to_string(m_uiServerPort) + "!");
	}
}


void StudyInterface::draw()
{
	if (m_pHinge && m_bShowStimulus && !m_bPaused)
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

	if (m_bPaused)
	{
		std::stringstream ss;

		if (m_bStudyMode)
		{
			size_t trialsLeft = m_vExperimentConditions.size();

			if (trialsLeft == 1)
				ss << "Last Trial";
			else
				ss << trialsLeft << " Trials Remaining";

			Renderer::getInstance().drawUIText(
				ss.str(),
				glm::vec4(0.6f, 0.8f, 0.6f, 1.f),
				glm::vec3(glm::vec2(m_ivec2Screen) / 2.f, 0.f),
				glm::quat(),
				m_ivec2Screen.x / 3.f,
				Renderer::WIDTH,
				Renderer::CENTER,
				Renderer::CENTER_BOTTOM
			);

			ss.str(std::string());

			if (trialsLeft)
			ss << "Press SPACE to begin the " << (m_vExperimentConditions.size() == 1 ? "last" : "next") << " trial...";

			Renderer::getInstance().drawUIText(
				ss.str(),
				glm::vec4(1.f),
				glm::vec3(glm::vec2(m_ivec2Screen) / 2.f, 0.f),
				glm::quat(),
				m_ivec2Screen.x / 3.f,
				Renderer::WIDTH,
				Renderer::CENTER,
				Renderer::CENTER_TOP
			);
		}
		else
		{
			ss << "Study Complete!";

			Renderer::getInstance().drawUIText(
				ss.str(),
				glm::vec4(1.f, 1.f, 0.5f, 1.f),
				glm::vec3(glm::vec2(m_ivec2Screen) / 2.f, 0.f),
				glm::quat(),
				m_ivec2Screen.x / 3.f,
				Renderer::WIDTH,
				Renderer::CENTER,
				Renderer::CENTER_MIDDLE
			);
		}
	}
}

void StudyInterface::begin()
{
	for (auto a : m_vfAngleConditions)
		for (auto d : m_vfDistanceConditions)
		{
			m_vExperimentConditions.push_back({ m_BoolDistribution(m_Generator) ? a : -a, d, 90 + m_AngleDistribution(m_Generator), 10.f, glm::vec3(0.f, 0.f, -5.f), true });
			m_vExperimentConditions.push_back({ m_BoolDistribution(m_Generator) ? a : -a, d, 90 - m_AngleDistribution(m_Generator), 10.f, glm::vec3(0.f, 0.f, -5.f), true });
			//m_vExperimentConditions.push_back({ m_BoolDistribution(m_Generator) ? a : -a, d, 90 + m_AngleDistribution(m_Generator), 10.f, glm::vec3(0.f, 0.f, -5.f), false });
			//m_vExperimentConditions.push_back({ m_BoolDistribution(m_Generator) ? a : -a, d, 90 - m_AngleDistribution(m_Generator), 10.f, glm::vec3(0.f, 0.f, -5.f), false });
		}
	
	std::shuffle(m_vExperimentConditions.begin(), m_vExperimentConditions.end(), m_Generator);

	m_strLastResponse = std::string();

	DataLogger::getInstance().setID(m_strName);
	DataLogger::getInstance().openLog(m_strName);
	DataLogger::getInstance().setHeader("trial,view.angle,view.dist,fishtank,start.angle,hinge.length,hinge.z.pos,hinge.angle,response");
	DataLogger::getInstance().start();

	m_bPaused = true;
}

void StudyInterface::next(StudyResponse response)
{
	if (response == ACUTE) // perceived as acute
	{
		m_pHinge->setAngle(m_pHinge->getAngle() + m_fStepSize);

		if (m_strLastResponse.compare("obtuse") == 0)
			m_nReversals--;		

		m_strLastResponse = "acute";
	}
	else if (response == OBTUSE) // perceived as obtuse
	{
		m_pHinge->setAngle(m_pHinge->getAngle() - m_fStepSize);

		if (m_strLastResponse.compare("acute") == 0)		
			m_nReversals--;

		m_strLastResponse = "obtuse";
	}

	if (m_nReversals == 0)
	{
		m_nReversals = 11; // add an extra for initial mistakes, which are not recorded

		m_vExperimentConditions.pop_back();

		if (m_pSocket->send("0," + std::to_string(m_fMoveTime)))
		{
			m_fCurrentScreenAngle = 0.f;
			m_tMoveStart = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(250);
		}

		m_bPaused = true;

		if (m_vExperimentConditions.size() == 0)
			end();
	}
	else
	{
		m_tStimulusStart = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(static_cast<int>(m_fStimulusDelay * 1000.f));
	}
}

void StudyInterface::end()
{
	m_pSocket->send("0,2.5");
	m_fCurrentScreenAngle = 0.f;
	m_bStudyMode = false;
	DataLogger::getInstance().closeLog();
}

glm::vec3 StudyInterface::getCOP()
{
	if (m_bLockViewCOP)
		return glm::vec3(glm::rotate(glm::mat4(), glm::radians(m_fViewAngle), glm::vec3(0.f, 1.f, 0.f)) * glm::vec4(0.f, 0.f, m_fViewDist, 1.f));
	else
		return glm::vec3(glm::rotate(glm::mat4(), glm::radians(m_fCOPAngle), glm::vec3(0.f, 1.f, 0.f)) * glm::vec4(0.f, 0.f, m_fCOPDist, 1.f));
}

float StudyInterface::getEyeSep()
{
	return m_fEyeSep;
}

void StudyInterface::writeToLog(StudyResponse response)
{
	std::string logEntry;
	logEntry += std::to_string((m_vfAngleConditions.size() * m_vfDistanceConditions.size()) - m_vExperimentConditions.size());
	logEntry += ",";
	logEntry += std::to_string(m_vExperimentConditions.back().viewAngle);
	logEntry += ",";
	logEntry += std::to_string(m_vExperimentConditions.back().viewDist);
	logEntry += ",";
	logEntry += std::to_string(m_vExperimentConditions.back().matchedView);
	logEntry += ",";
	logEntry += std::to_string(m_vExperimentConditions.back().startAngle);
	logEntry += ",";
	logEntry += std::to_string(m_vExperimentConditions.back().hingeLen);
	logEntry += ",";
	logEntry += std::to_string(m_vExperimentConditions.back().hingePos.z);
	logEntry += ",";
	logEntry += std::to_string(m_pHinge->getAngle());
	logEntry += ",";
	logEntry += response == ACUTE ? "acute" : "obtuse";
	logEntry += ",";

	DataLogger::getInstance().logMessage(logEntry);
}

void StudyInterface::loadCondition()
{
	m_pHinge->setAngle(m_vExperimentConditions.back().startAngle);
	m_pHinge->setLength(m_vExperimentConditions.back().hingeLen);
	m_pHinge->setPos(m_vExperimentConditions.back().hingePos);

	m_fCOPDist = m_fViewDist * m_vExperimentConditions.back().viewDist;

	m_bLockViewCOP = m_vExperimentConditions.back().matchedView;

	if (m_fCurrentScreenAngle != -m_vExperimentConditions.back().viewAngle)
	{
		m_fCurrentScreenAngle = -m_vExperimentConditions.back().viewAngle;

		m_pSocket->send((int)-m_vExperimentConditions.back().viewAngle + "," + std::to_string(m_fMoveTime));
		m_tMoveStart = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(250);

		m_tStimulusStart = m_tMoveStart + std::chrono::milliseconds(static_cast<int>((m_fMoveTime + m_fStimulusDelay) * 1000.f));
	}
	else
	{
		m_tStimulusStart = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(static_cast<int>(m_fStimulusDelay * 1000.f));
	}

}

void StudyInterface::receive(void * data)
{
	int eventData[2];

	memcpy(&eventData, data, sizeof(int) * 2);

	if (eventData[0] == GLFWInputBroadcaster::EVENT::KEY_DOWN)
	{
		if (m_bStudyMode)
		{
			if (eventData[1] == GLFW_KEY_LEFT_SHIFT)
			{
				if (m_nReversals <= 10u) // ignore the first reversal
					writeToLog(ACUTE);
				next(ACUTE);
			}

			if (eventData[1] == GLFW_KEY_RIGHT_SHIFT)
			{
				if (m_nReversals <= 10u) // ignore the first reversal
					writeToLog(OBTUSE);
				next(OBTUSE);
			}

			if (eventData[1] == GLFW_KEY_SPACE && m_bPaused)
			{
				m_bPaused = false;
				loadCondition();
			}
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
			else
			{
				if (eventData[1] == GLFW_KEY_F1)
				{
					m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
						return p.desc.compare("Server IP") == 0;
					}));
				}

				if (eventData[1] == GLFW_KEY_F2)
				{
					m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
						return p.desc.compare("Server Port") == 0;
					}));
				}

				if (eventData[1] == GLFW_KEY_F3)
				{
					m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
						return p.desc.compare("Eye Separation (cm)") == 0;
					}));
				}

				if (eventData[1] == GLFW_KEY_F4)
				{
					m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
						return p.desc.compare("View Distance (cm)") == 0;
					}));
				}

				if (eventData[1] == GLFW_KEY_F5)
				{
					m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
						return p.desc.compare("View Angle (deg)") == 0;
					}));
				}

				if (eventData[1] == GLFW_KEY_F6)
				{
					m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
						return p.desc.compare("Display Move Time (sec)") == 0;
					}));
				}

				if (eventData[1] == GLFW_KEY_F7)
				{
					m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
						return p.desc.compare("Name") == 0;
					}));
				}
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
				if (!m_SocketFuture.valid())
				{
					Renderer::getInstance().showMessage("Connecting to " + m_strServerAddress + " port " + std::to_string(m_uiServerPort));
					m_SocketFuture = std::async(std::launch::async, &WinsockClient::connect, m_pSocket, m_strServerAddress, m_uiServerPort);
				}
				else
				{
					Renderer::getInstance().showMessage("Already attempting connection with " + m_strServerAddress + " port " + std::to_string(m_uiServerPort));
				}
			}

			if (eventData[1] == GLFW_KEY_R && !m_pEditParam && !m_bStudyMode)
				reset();

			if (eventData[1] == GLFW_KEY_HOME && !m_pEditParam && !m_bStudyMode)
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

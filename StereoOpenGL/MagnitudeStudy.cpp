#include "MagnitudeStudy.h"

#include "Renderer.h"
#include "DataLogger.h"
#include "DistortionUtils.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <iomanip> // for std::setprecision()
#include <gtc/matrix_transform.hpp>

MagnitudeStudy::MagnitudeStudy()
	: m_pSocket(NULL)
	, m_fHingeSize(10.f)
	, m_pEditParam(NULL)
	, m_pDiagram(NULL)
	, m_bStudyMode(false)
	, m_bPaused(false)
	, m_bShowStimulus(true)
	, m_bBlockInput(false)
	, m_bFishtank(false)
	, m_bShowDiagram(false)
	, m_bWaitingForResponse(false)
	, m_bDisplayCondition(false)
	, m_Generator(std::random_device()())
	, m_AngleDistribution(std::uniform_int_distribution<int>(10, 20))
	, m_BoolDistribution(std::uniform_int_distribution<int>(0, 1))
{
}


MagnitudeStudy::~MagnitudeStudy()
{
	if (m_pSocket)
	{
		m_pSocket->send("0,1");
		delete m_pSocket;
	}

	if (m_pDiagram)
		delete m_pDiagram;
}

void MagnitudeStudy::init(glm::ivec2 screenRes, glm::mat4 worldToScreenTransform)
{
	DataLogger::getInstance().setLogDirectory("logs");

	m_ivec2Screen = screenRes;
	m_mat4Screen = worldToScreenTransform;
	m_mat4Screen[2] *= 10.f;

	if (m_pSocket == NULL)
		m_pSocket = new WinsockClient();

	if (m_pDiagram == NULL)
		m_pDiagram = new ViewingConditionsDiagram(m_mat4Screen, m_ivec2Screen);

	Renderer::getInstance().addTexture(new GLTexture("noise1.png", false));

	reset();
}

void MagnitudeStudy::reset()
{
	m_bStudyMode = false;
	m_bPaused = false;
	m_bBlockInput = false;
	m_bFishtank = false;
	m_bWaitingForResponse = false;

	m_strServerAddress = "192.168.137.";
	m_uiServerPort = 5005;

	m_strName = "NONAME";

	m_fViewAngle = 0.f;
	m_fViewDist = 57.f;
	m_fEyeSep = 6.7;

	m_Vector.length = m_fHingeSize;
	m_Vector.angle = 0.f;
	m_Vector.diameter = 0.5f;
	m_Vector.rotAxis = glm::vec3(0.f, 1.f, 0.f);
	m_Vector.originCenter = true;
	m_Vector.pos = glm::vec3(0.f, glm::length(m_mat4Screen[1]) / 2.f, 0.f);
	m_Vector.color = glm::vec3(1.f, 0.f, 0.f);
	m_Vector.shaderName = "rings";
	m_Vector.textureName = "noise1.png";

	m_MeasuringRod.length = m_fHingeSize;
	m_MeasuringRod.angle = -90.f;
	m_MeasuringRod.diameter = 0.15f;
	m_MeasuringRod.rotAxis = glm::vec3(0.f, 0.f, 1.f);
	m_MeasuringRod.originCenter = false;
	m_MeasuringRod.pos = glm::vec3(0.f, glm::length(m_mat4Screen[1]) / 3.f, 0.f);
	m_MeasuringRod.color = glm::vec3(1.f);
	m_MeasuringRod.shaderName = "lighting";
	m_MeasuringRod.textureName = "white";

	m_fCOPAngle = m_fViewAngle;
	m_fCOPDist = m_fViewDist;

	m_fMinStep = 2.f;
	m_fStepSize = m_fMinStep;

	m_vExperimentConditions.clear();

	m_fStimulusTime = 10.f;
	m_fStimulusDelay = 1.f;
	m_tStimulusStart = std::chrono::high_resolution_clock::time_point();

	m_fMoveTime = 2.5f;
	m_tMoveStart = std::chrono::high_resolution_clock::time_point();

	m_fLastAngle = m_fTargetAngle = 0.f;

	m_pEditParam = NULL;

	m_vParams.push_back({ "Server IP" , m_strServerAddress, STUDYPARAM_NUMERIC | STUDYPARAM_DECIMAL | STUDYPARAM_IP });
	m_vParams.push_back({ "Server Port" , std::to_string(m_uiServerPort), STUDYPARAM_NUMERIC });
	m_vParams.push_back({ "Eye Separation (cm)" , "6.70", STUDYPARAM_NUMERIC | STUDYPARAM_DECIMAL });
	m_vParams.push_back({ "View Distance (cm)" , "57.0", STUDYPARAM_NUMERIC | STUDYPARAM_DECIMAL });
	m_vParams.push_back({ "View Angle (deg)" , "0", STUDYPARAM_NUMERIC | STUDYPARAM_POSNEG });
	m_vParams.push_back({ "Display Move Time (sec)" , "5.0", STUDYPARAM_NUMERIC | STUDYPARAM_DECIMAL });
	m_vParams.push_back({ "Name" , m_strName, STUDYPARAM_ALPHA | STUDYPARAM_NUMERIC | STUDYPARAM_DECIMAL });

	m_fMaxDistortionMag = 0.f;

	m_vec3GridPoints.clear();
	int gridRes = 10;
	for (int i = 0; i < gridRes + 1; ++i)
		for (int j = 0; j < gridRes + 1; ++j)
			for (int k = 0; k < gridRes + 1; ++k)
				m_vec3GridPoints.push_back(glm::vec3(m_mat4Screen * glm::vec4(i/(gridRes/2.f) - 1.f, j/(gridRes/2.f) - 1.f, k/(gridRes/2.f) - 1.f, 1.f)));
}

void MagnitudeStudy::update()
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


		if (elapsedStim > m_fStimulusTime && !m_bPaused)
		{
			m_bBlockInput = true;

			if (m_tStimulusStart != std::chrono::high_resolution_clock::time_point())
			{
				writeToLog();
				next();
			}
		}
	}
	else
		m_bShowStimulus = true;


	if (m_SocketFuture.valid() && m_SocketFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
	{
		if (m_SocketFuture.get())
		{
			moveScreen(0, true);
			Renderer::getInstance().showMessage("Successfully connected to " + m_strServerAddress + " port " + std::to_string(m_uiServerPort) + " successfully!");
		}
		else
			Renderer::getInstance().showMessage("ERROR! Could not connect to server at " + m_strServerAddress + " port " + std::to_string(m_uiServerPort) + "!");
	}
}


void MagnitudeStudy::draw()
{
	if (m_bShowDiagram)
	{
		m_pDiagram->draw();
	}
	else if ((m_bShowStimulus && !m_bPaused) || m_bDisplayCondition)
	{
		for (auto rod : { m_Vector, m_MeasuringRod })
		{
			Renderer::RendererSubmission rs;
			rs.glPrimitiveType = GL_TRIANGLES;
			rs.shaderName = rod.shaderName;
			rs.VAO = Renderer::getInstance().getPrimitiveVAO("cylinder");
			rs.vertCount = Renderer::getInstance().getPrimitiveIndexCount("cylinder");
			rs.indexType = GL_UNSIGNED_SHORT;
			rs.diffuseTexName = rod.textureName;
			rs.diffuseColor = glm::vec4(rod.color, 1.f);
			rs.hasTransparency = rs.diffuseColor.a != 1.f;
			rs.specularColor = glm::vec4(glm::vec3(0.f), 1.f);
			rs.specularExponent = 100.f;
			rs.modelToWorldTransform = glm::translate(glm::mat4(), rod.pos) * glm::rotate(glm::mat4(), glm::radians(rod.angle), rod.rotAxis) * glm::rotate(glm::mat4(), glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f)) * glm::scale(glm::mat4(), glm::vec3(rod.diameter, rod.diameter, rod.length));
			if (rod.originCenter)
				rs.modelToWorldTransform *= glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, -0.5f));

			Renderer::getInstance().addToDynamicRenderQueue(rs);
		}
	}
	
	for (int i = 0; i < m_vec3DistortedGridPoints.size(); ++i)
	{
		glm::vec3 dir(m_vec3DistortedGridPoints[i] - m_vec3GridPoints[i]);

		auto u = glm::normalize(glm::cross(glm::vec3(0.f, 1.f, 0.f), glm::normalize(dir)));
		auto v = glm::normalize(glm::cross(glm::normalize(dir), u));

		glm::mat4 xform;
		xform[0] = glm::vec4(u * 0.05f, 0.f);
		xform[1] = glm::vec4(v * 0.05f, 0.f);
		xform[2] = glm::vec4(dir, 0.f);
		xform[3] = glm::vec4(m_vec3GridPoints[i], 1.f);
		glm::vec4 color = glm::mix(glm::vec4(1.f, 1.f, 1.f, 0.2f), glm::vec4(1.f, 0.f, 0.f, 0.5f), glm::length(dir) / m_fMaxDistortionMag);
		
		Renderer::getInstance().drawPrimitive("icosphere", glm::translate(glm::mat4(), m_vec3GridPoints[i]) * glm::scale(glm::mat4(), glm::vec3(0.1f)), glm::vec4(0.f, 0.f, 1.f, 0.25f), glm::vec4(1.f), 32.f);
		Renderer::getInstance().drawPrimitive("icosphere", glm::translate(glm::mat4(), m_vec3DistortedGridPoints[i]) * glm::scale(glm::mat4(), glm::vec3(0.2f)), color, glm::vec4(1.f), 32.f);
		Renderer::getInstance().drawPrimitive("cylinder", xform, color, glm::vec4(1.f), 32.f);
	}

	if (m_pEditParam)
	{
		Renderer::getInstance().drawUIText(
			m_pEditParam->desc + ": " + m_pEditParam->buf + "_",
			glm::vec4(1.f),
			glm::vec3(0.f),
			glm::quat(),
			m_ivec2Screen.y / 50.f,
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
			ss.precision(2);

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

	if (m_bWaitingForResponse)
	{
	}

	if (m_bDisplayCondition && !m_bStudyMode)
	{
		std::stringstream ss;
		ss.precision(3);

		ss << "Viewing Angle: " << (m_bFishtank ? m_fViewAngle : m_fCOPAngle) << std::endl;
		ss << "Viewing Distance: " << (m_bFishtank ? m_fViewDist : m_fCOPDist) << std::endl;
		ss << "Eye Separation: " << m_fEyeSep << std::endl;
		ss << "Target Rod Angle: " << m_Vector.angle << std::endl;
		ss << "Target Rod Length: " << m_Vector.length << std::endl;
		ss << "Measuring Rod Length: " << m_MeasuringRod.length << std::endl;
		ss << (m_bFishtank ? "Fishtank Mode" : "Untracked Stereo Mode");		

		Renderer::getInstance().drawUIText(
			ss.str(),
			glm::vec4(1.f),
			glm::vec3(m_ivec2Screen.x, 0.f, 0.f),
			glm::quat(),
			m_ivec2Screen.x / 10.f,
			Renderer::HEIGHT,
			Renderer::RIGHT,
			Renderer::BOTTOM_RIGHT
		);
	}
}

void MagnitudeStudy::begin()
{
	generateTrials();

	DataLogger::getInstance().setID(m_strName);
	DataLogger::getInstance().openLog(m_strName);
	DataLogger::getInstance().setHeader("trial,ipd,view.dist,view.angle,view.dist.factor,fishtank,rod.angle,rod.length,response");
	DataLogger::getInstance().start();

	m_bPaused = true;
}

void MagnitudeStudy::generateTrials(bool randomOrder)
{
	for (auto a : { 0.f , 15.f, 30.f })						// view angles
		for (auto d : { 1.f })								// view distance factors
			for (auto s : { 0.f, m_fEyeSep })				// eye separations (for mono/stereo)
				for (auto r : { 0.f, 45.f, 90.f, 135.f })	// rod angles
					for (auto l : { 10.f, 20.f })			// rod lengths
						for (auto f : { true, false })		// fishtank mode
							m_vExperimentConditions.push_back(StudyCondition({ a, d, s, r, l, f }));
					

	m_nTrials = m_vExperimentConditions.size();

	if (randomOrder)
		std::shuffle(m_vExperimentConditions.begin(), m_vExperimentConditions.end(), m_Generator);
}

void MagnitudeStudy::next()
{
	m_bWaitingForResponse = false;

	m_vExperimentConditions.pop_back();

	moveScreen(0);

	m_bPaused = true;

	if (m_vExperimentConditions.size() == 0)
		end();
}

void MagnitudeStudy::end()
{
	moveScreen(0);
	m_bStudyMode = false;
	DataLogger::getInstance().closeLog();
}

glm::vec3 MagnitudeStudy::getCOP()
{
	if (m_bFishtank)
		return glm::vec3(glm::rotate(glm::mat4(), glm::radians(m_fViewAngle), glm::vec3(0.f, 1.f, 0.f)) * glm::vec4(0.f, 0.f, m_fViewDist, 1.f));
	else
		return glm::vec3(glm::rotate(glm::mat4(), glm::radians(m_fCOPAngle), glm::vec3(0.f, 1.f, 0.f)) * glm::vec4(0.f, 0.f, m_fCOPDist, 1.f));
}

float MagnitudeStudy::getEyeSep()
{
	return m_fEyeSep;
}

std::string MagnitudeStudy::conditionString()
{
	std::stringstream ss;

	ss << ((m_fEyeSep > 0.f) ? "stereo" : "mono");
	ss << "_";
	ss << (m_bFishtank ? "coupled" : "uncoupled");
	ss << "_";
	ss << "va" << std::fixed << std::setprecision(0) << m_fViewAngle;
	ss << "_";
	ss << "ra" << std::fixed << std::setprecision(0) << m_Vector.angle;
	ss << "_";
	ss << "rl" << std::fixed << std::setprecision(0) << m_Vector.length;

	return ss.str();
}

void MagnitudeStudy::writeToLog()
{
	DataLogger::getInstance().setHeader("trial,ipd,view.dist,view.dist.factor,view.angle,fishtank,rod.angle,rod.length,response");
	std::string logEntry;
	logEntry += std::to_string(m_nTrials - m_vExperimentConditions.size());
	logEntry += ",";
	logEntry += std::to_string(m_fEyeSep);
	logEntry += ",";
	logEntry += std::to_string(m_fViewDist);
	logEntry += ",";
	logEntry += std::to_string(m_vExperimentConditions.back().viewDistFactor);
	logEntry += ",";
	logEntry += std::to_string(m_vExperimentConditions.back().viewAngle);
	logEntry += ",";
	logEntry += std::to_string(m_vExperimentConditions.back().fishtank);
	logEntry += ",";
	logEntry += std::to_string(m_vExperimentConditions.back().angle);
	logEntry += ",";
	logEntry += std::to_string(m_vExperimentConditions.back().len);
	logEntry += ",";
	logEntry += std::to_string(m_MeasuringRod.length);

	DataLogger::getInstance().logMessage(logEntry);
}

void MagnitudeStudy::loadCondition(StudyCondition &c)
{
	m_Vector.angle = c.angle;
	m_Vector.length = c.len;

	m_MeasuringRod.length = 1.f;

	m_fCOPDist = m_fViewDist * c.viewDistFactor;

	m_bFishtank = c.fishtank;

	m_fEyeSep = c.eyeSeparation;

	if (moveScreen(c.viewAngle))
		m_tStimulusStart = m_tMoveStart + std::chrono::milliseconds(static_cast<int>(m_fMoveTime * 1000.f));
	else
		m_tStimulusStart =  std::chrono::high_resolution_clock::now();

	m_tStimulusStart += std::chrono::milliseconds(static_cast<int>(m_fStimulusDelay * 1000.f));

	m_bBlockInput = false;
}

bool MagnitudeStudy::moveScreen(float viewAngle, bool forceMove)
{
	if (m_fViewAngle == viewAngle && !forceMove)
		return false;

	m_fLastAngle = m_fViewAngle;
	m_fTargetAngle = viewAngle;

	// The display angle should be the negative of the viewing angle since our viewpoint is fixed
	std::stringstream ss;
	ss.precision(2);
	ss << -viewAngle << "," << m_fMoveTime;
	
	m_pSocket->send(ss.str());
	m_tMoveStart = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(250);

	return true;
}

void MagnitudeStudy::receive(void * data)
{
	//if (m_bBlockInput)
	//	return;

	int eventData[2];

	memcpy(&eventData, data, sizeof(int) * 2);

	if (eventData[0] == GLFWInputBroadcaster::EVENT::KEY_DOWN)
	{
		if (m_bStudyMode)
		{
			if (eventData[1] == GLFW_KEY_SPACE && m_bPaused)
			{
				m_bPaused = false;
				loadCondition(m_vExperimentConditions.back());
			}
		}
		else
		{
			if (eventData[1] == GLFW_KEY_BACKSPACE && m_pEditParam->buf.length() > 0)
				m_pEditParam->buf.pop_back();


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
						moveScreen(std::stof(m_pEditParam->buf));
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

				if (eventData[1] == GLFW_KEY_R)
					reset();

				if (eventData[1] == GLFW_KEY_HOME)
				{
					m_bStudyMode = true;
					begin();
				}
			}

			if (eventData[1] == GLFW_KEY_F8)
			{
				m_bFishtank = !m_bFishtank;
				std::string msg("Fishtank Mode ");
				msg += (m_bFishtank ? "on" : "off");
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

			if (eventData[1] == GLFW_KEY_F10)
			{
				m_bShowDiagram = !m_bShowDiagram;
			}

			if (eventData[1] == GLFW_KEY_I)
			{
				m_bDisplayCondition = !m_bDisplayCondition;
			}

			//if (eventData[1] == GLFW_KEY_X)
			//{
			//	glm::mat4 m_mat4ScreenBasisOrtho = glm::mat4(
			//		glm::normalize(m_mat4Screen[0]),
			//		glm::normalize(m_mat4Screen[1]),
			//		glm::normalize(m_mat4Screen[2]),
			//		m_mat4Screen[3]
			//	);
			//
			//	float projAngleOffset = glm::degrees(glm::asin(m_fEyeSep / (2.f * m_fCOPDist)));
			//
			//	glm::vec3 copLeft = (glm::rotate(glm::mat4(), glm::radians(m_fCOPAngle - projAngleOffset), glm::vec3(m_mat4ScreenBasisOrtho[1])) * glm::translate(m_mat4ScreenBasisOrtho, glm::vec3(0.f, 0.f, m_fCOPDist)))[3];
			//	glm::vec3 copRight = (glm::rotate(glm::mat4(), glm::radians(m_fCOPAngle + projAngleOffset), glm::vec3(m_mat4ScreenBasisOrtho[1])) * glm::translate(m_mat4ScreenBasisOrtho, glm::vec3(0.f, 0.f, m_fCOPDist)))[3];
			//	
			//	float viewAngleOffset = glm::degrees(glm::asin(m_fEyeSep / (2.f * m_fViewDist)));
			//
			//	glm::vec3 leftEyePos = (glm::rotate(glm::mat4(), glm::radians(m_fViewAngle - viewAngleOffset), glm::vec3(m_mat4ScreenBasisOrtho[1])) * glm::translate(m_mat4ScreenBasisOrtho, glm::vec3(0.f, 0.f, m_fViewDist)))[3];
			//	glm::vec3 rightEyePos = (glm::rotate(glm::mat4(), glm::radians(m_fViewAngle + viewAngleOffset), glm::vec3(m_mat4ScreenBasisOrtho[1])) * glm::translate(m_mat4ScreenBasisOrtho, glm::vec3(0.f, 0.f, m_fViewDist)))[3];
			//
			//	//m_vec3DistortedGridPoints.clear();
			//	m_vec3DistortedGridPoints = distutil::transformStereoscopicPoints(copLeft, copRight, leftEyePos, rightEyePos, glm::vec3(m_mat4ScreenBasisOrtho[3]), glm::normalize(glm::vec3(m_mat4ScreenBasisOrtho[2])), m_vec3GridPoints);
			//
			//	m_fMaxDistortionMag = 0.f;
			//	for (int i = 0; i < m_vec3DistortedGridPoints.size(); ++i)
			//	{
			//		float len = glm::length(m_vec3DistortedGridPoints[i] - m_vec3GridPoints[i]);
			//		if (len > m_fMaxDistortionMag)
			//			m_fMaxDistortionMag = len;
			//	}
			//
			//}
		}
	}

	if (eventData[0] == GLFWInputBroadcaster::EVENT::KEY_DOWN || eventData[0] == GLFWInputBroadcaster::EVENT::KEY_HOLD)
	{
		float angleDelta = 1.f;
		float distDelta = 1.f;

		if (!m_bStudyMode)
		{
			if (eventData[1] == GLFW_KEY_LEFT)
			{
			}
			if (eventData[1] == GLFW_KEY_RIGHT)
			{
			}

			if (eventData[1] == GLFW_KEY_KP_4)
			{
				m_Vector.angle += 1.f;
			}
			if (eventData[1] == GLFW_KEY_KP_6)
			{
				m_Vector.angle -= 1.f;
			}
			if (eventData[1] == GLFW_KEY_KP_8)
			{
				m_Vector.length += 0.1f;
			}
			if (eventData[1] == GLFW_KEY_KP_2)
			{
				m_Vector.length = std::max(m_Vector.length - 0.1f, 0.1f);
			}

			if (eventData[1] == GLFW_KEY_LEFT_BRACKET)
				m_fCOPAngle -= angleDelta;
			if (eventData[1] == GLFW_KEY_RIGHT_BRACKET)
				m_fCOPAngle += angleDelta;

			if (eventData[1] == GLFW_KEY_KP_SUBTRACT)
				m_fCOPDist = std::max(m_fCOPDist - distDelta, 0.f);
			if (eventData[1] == GLFW_KEY_KP_ADD)
				m_fCOPDist += distDelta;

			if (eventData[1] == GLFW_KEY_MINUS)
				m_pDiagram->setEyeSeparation(m_pDiagram->getEyeSeparation() - 0.1f);
			if (eventData[1] == GLFW_KEY_EQUAL)
				m_pDiagram->setEyeSeparation(m_pDiagram->getEyeSeparation() + 0.1f);

			if (eventData[1] == GLFW_KEY_9)
				m_pDiagram->setViewDistance(m_pDiagram->getViewDistance() - 0.1f);
			if (eventData[1] == GLFW_KEY_0)
				m_pDiagram->setViewDistance(m_pDiagram->getViewDistance() + 0.1f);

			if (eventData[1] == GLFW_KEY_COMMA)
				m_pDiagram->setProjectionAngle(m_pDiagram->getProjectionAngle() - 1.f);
			if (eventData[1] == GLFW_KEY_PERIOD)
				m_pDiagram->setProjectionAngle(m_pDiagram->getProjectionAngle() + 1.f);
		}

		if (eventData[1] == GLFW_KEY_UP)
			m_MeasuringRod.length += 0.1f;
		if (eventData[1] == GLFW_KEY_DOWN)
			m_MeasuringRod.length = std::max(m_MeasuringRod.length - 0.1f, 0.1f);

	}
}

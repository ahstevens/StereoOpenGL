#include "Engine.h"
#include "DebugDrawer.h"
#include "UntrackedStereoDiagram.h"

#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <filesystem>
#include <limits>

#include <gtc/matrix_transform.hpp>
#include <gtx/intersect.hpp>

#define INTOCM 2.54f

UntrackedStereoDiagram*			g_pDiagram;

float							g_fEyeSep = 6.3f;
float							g_fDisplayDiag = 13.3f * INTOCM; // physical display diagonal measurement, given in inches, usually
glm::vec3						g_vec3ScreenPos(0.f, 0.f, 10.f);
glm::vec3						g_vec3ScreenNormal(0.f, 0.f, 1.f);
glm::vec3						g_vec3ScreenUp(0.f, 1.f, 0.f);
bool							g_bStereo = false;

//-----------------------------------------------------------------------------
// Purpose: OpenGL Debug Callback Function
//-----------------------------------------------------------------------------
void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131184 || id == 131185 || id == 131218 || id == 131204 || id == 131076) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}



//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void dprintf(const char *fmt, ...)
{
	va_list args;
	char buffer[2048];

	va_start(args, fmt);
	vsprintf_s(buffer, fmt, args);
	va_end(args);

#ifdef DEBUG
	printf("%s", buffer);
#endif // DEBUG

	OutputDebugStringA(buffer);
}

void error_callback_glfw(int error, const char* desc) { dprintf("GLFW Error %d: %s\n", error, desc); }

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
Engine::Engine(int argc, char *argv[], int mode)
	: m_bGLInitialized(false)
	, m_pMainWindow(NULL)
	, m_pLeftEyeFramebuffer(NULL)
	, m_pRightEyeFramebuffer(NULL)
{
};


//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
Engine::~Engine()
{
	// work is done in Shutdown
	dprintf("Shutdown\n");
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool Engine::init()
{
	if (!glfwInit())
	{
		dprintf("%s - GLFW could not initialize!\n", __FUNCTION__);
		return false;
	}

	glfwSetErrorCallback(error_callback_glfw);

	listDisplayInfo();

	if (!(m_pMainWindow = createWindow(glfwGetPrimaryMonitor(), 0, 0, g_bStereo)))
	{
		dprintf("%s - Window could not be created!\n", __FUNCTION__);
		return false;
	}
	
	glfwGetWindowSize(m_pMainWindow, &m_ivec2MainWindowSize.x, &m_ivec2MainWindowSize.y);
	
	if (!initGL(g_bStereo))
	{
		dprintf("%s - Unable to initialize OpenGL!\n", __FUNCTION__);
		return false;
	}

	GLFWInputBroadcaster::getInstance().init(m_pMainWindow);

	GLFWInputBroadcaster::getInstance().addObserver(this);


	glfwSetInputMode(m_pMainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!Renderer::getInstance().init())
		return false;

	m_Head.pos = glm::vec3(0.f, 0.f, 67.f);
	m_Head.rot = glm::quat(glm::inverse(glm::lookAt(m_Head.pos, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f))));

	float sizer = g_fDisplayDiag / sqrt(glm::dot(glm::vec2(m_ivec2MainWindowSize), glm::vec2(m_ivec2MainWindowSize)));

	glm::vec2 screenSize_cm = glm::vec2(m_ivec2MainWindowSize) * sizer;
	
	glm::mat4 screenTrans(
		glm::vec4(glm::normalize(glm::cross(g_vec3ScreenUp, g_vec3ScreenNormal)) * screenSize_cm.x * 0.5f, 0.f),
		glm::vec4(g_vec3ScreenUp * screenSize_cm.y * 0.5f, 0.f),
		glm::vec4(g_vec3ScreenNormal, 0.f),
		glm::vec4(g_vec3ScreenPos, 1.f)
	);

	g_pDiagram = new UntrackedStereoDiagram(screenTrans, m_ivec2MainWindowSize);

	createUIView();
	createMonoView();
	createStereoViews();
	
	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool Engine::initGL(bool stereoContext)
{
	if (m_bGLInitialized)
		return true;
	
	if (!m_pMainWindow)
	{
		dprintf("%s - Error: Need a window and context to initialize OpenGL!\n", __FUNCTION__);
		return false;
	}

	glewExperimental = GL_TRUE;
	GLenum nGlewError = glewInit();
	if (nGlewError != GLEW_OK)
	{
		dprintf("%s - Error initializing GLEW! %s\n", __FUNCTION__, glewGetErrorString(nGlewError));
		return false;
	}

	glGetError(); // to clear the error caused deep in GLEW

#if _DEBUG
		glDebugMessageCallback((GLDEBUGPROC)DebugCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

		if (stereoContext)
		{
			GLint stereo;
			glGetIntegerv(GL_STEREO, &stereo);

			if (!stereo)
				dprintf("%s - Error: The OpenGL context has GL_STEREO set to GL_FALSE!\n", __FUNCTION__);
		}

	m_bGLInitialized = true;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Engine::Shutdown()
{
	GLFWInputBroadcaster::getInstance().removeObserver(this);
	 
	if (m_pLeftEyeFramebuffer)
		delete m_pLeftEyeFramebuffer;
	if (m_pRightEyeFramebuffer)
		delete m_pRightEyeFramebuffer;

	if (m_pMainWindow)
	{
		glfwDestroyWindow(m_pMainWindow);
		m_pMainWindow = NULL;
	}

	glfwTerminate();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Engine::HandleInput()
{
	GLFWInputBroadcaster::getInstance().poll();
}

void Engine::receive(void * data)
{
	int eventData[2];

	memcpy(&eventData, data, sizeof(int) * 2);

	if (eventData[0] == GLFWInputBroadcaster::EVENT::KEY_DOWN)
	{
		if (eventData[1] == GLFW_KEY_ESCAPE)
			glfwSetWindowShouldClose(m_pMainWindow, GLFW_TRUE);

		if (eventData[1] == GLFW_KEY_F)
		{
			std::cout << "Frame Time: " << m_msFrameTime.count() << "ms" << std::endl;
			std::cout << "\t" << m_msInputHandleTime.count() << "ms\tInput Handling" << std::endl;
			std::cout << "\t" << m_msUpdateTime.count() << "ms\tState Update" << std::endl;
			std::cout << "\t" << m_msDrawTime.count() << "ms\tScene Drawing" << std::endl;
			std::cout << "\t" << m_msRenderTime.count() << "ms\tRendering" << std::endl;
		}
	}

	if (eventData[0] == GLFWInputBroadcaster::EVENT::KEY_DOWN || eventData[0] == GLFWInputBroadcaster::EVENT::KEY_HOLD)
	{
		float delta = 0.1f;

		if (eventData[1] == GLFW_KEY_LEFT)
			m_Head.pos -= glm::vec3(1.f, 0.f, 0.f) * delta;
		if (eventData[1] == GLFW_KEY_RIGHT)
			m_Head.pos += glm::vec3(1.f, 0.f, 0.f) * delta;
		if (eventData[1] == GLFW_KEY_UP)
			m_Head.pos += glm::vec3(0.f, 1.f, 0.f) * delta;
		if (eventData[1] == GLFW_KEY_DOWN)
			m_Head.pos -= glm::vec3(0.f, 1.f, 0.f) * delta;

		if (eventData[1] == GLFW_KEY_MINUS)
			g_pDiagram->setEyeSeparation(g_pDiagram->getEyeSeparation() - 0.1f);
		if (eventData[1] == GLFW_KEY_EQUAL)
			g_pDiagram->setEyeSeparation(g_pDiagram->getEyeSeparation() + 0.1f);
		
		if (eventData[1] == GLFW_KEY_LEFT_BRACKET)
			g_pDiagram->setViewAngle(g_pDiagram->getViewAngle() - 1.f);
		if (eventData[1] == GLFW_KEY_RIGHT_BRACKET)
			g_pDiagram->setViewAngle(g_pDiagram->getViewAngle() + 1.f);

		if (eventData[1] == GLFW_KEY_9)
			g_pDiagram->setViewDistance(g_pDiagram->getViewDistance() - 0.1f);
		if (eventData[1] == GLFW_KEY_0)
			g_pDiagram->setViewDistance(g_pDiagram->getViewDistance() + 0.1f);

		if (eventData[1] == GLFW_KEY_COMMA)
			g_pDiagram->setProjectionAngle(g_pDiagram->getProjectionAngle() - 1.f);
		if (eventData[1] == GLFW_KEY_PERIOD)
			g_pDiagram->setProjectionAngle(g_pDiagram->getProjectionAngle() + 1.f);
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Engine::RunMainLoop()
{
	using clock = std::chrono::high_resolution_clock;

	clock::time_point start, lastTime;

	start = lastTime = clock::now();

	while (!glfwWindowShouldClose(m_pMainWindow))
	{
		auto currentTime = clock::now();
		m_msFrameTime = currentTime - lastTime;
		lastTime = currentTime;

		auto a = clock::now();
		HandleInput();
		m_msInputHandleTime = clock::now() - a;

		a = clock::now();
		update();
		m_msUpdateTime = clock::now() - a;

		a = clock::now();
		makeScene();
		m_msDrawTime = clock::now() - a;

		a = clock::now();
		render();
		m_msRenderTime = clock::now() - a;
	}
}


void Engine::update()
{
	float sizer = g_fDisplayDiag / sqrt(m_ivec2MainWindowSize.x * m_ivec2MainWindowSize.x + m_ivec2MainWindowSize.y * m_ivec2MainWindowSize.y);

	float width_cm = m_ivec2MainWindowSize.x * sizer;
	float height_cm = m_ivec2MainWindowSize.y * sizer;

	if (g_bStereo)
	{
		// Update eye positions using current head position
		glm::vec3 leftEyePos = m_Head.pos - glm::normalize(glm::mat3_cast(m_Head.rot)[0]) * g_fEyeSep * 0.5f;
		glm::vec3 rightEyePos = m_Head.pos + glm::normalize(glm::mat3_cast(m_Head.rot)[0]) * g_fEyeSep * 0.5f;

		m_sviLeftEyeInfo.view = glm::translate(glm::mat4(), -leftEyePos);
		m_sviLeftEyeInfo.projection = getViewingFrustum(leftEyePos, g_vec3ScreenPos, g_vec3ScreenNormal, g_vec3ScreenUp, glm::vec2(width_cm, height_cm));

		m_sviRightEyeInfo.view = glm::translate(glm::mat4(), -rightEyePos);
		m_sviRightEyeInfo.projection = getViewingFrustum(rightEyePos, g_vec3ScreenPos, g_vec3ScreenNormal, g_vec3ScreenUp, glm::vec2(width_cm, height_cm));
	}
	else
	{
		m_sviMonoInfo.view = glm::translate(glm::mat4(), -m_Head.pos);
		m_Head.rot = glm::inverse(m_sviMonoInfo.view);
		m_sviMonoInfo.projection = getViewingFrustum(m_Head.pos, g_vec3ScreenPos, g_vec3ScreenNormal, g_vec3ScreenUp, glm::vec2(width_cm, height_cm));
	}
}

void Engine::makeScene()
{
	float rotRadius = 0.f;
	float osc = 0.f;
	float rate_ms = 2500.f;
	double ratio = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - std::chrono::high_resolution_clock::time_point()).count() % static_cast<long long>(rate_ms)) / rate_ms;
	float angle = 360.f * static_cast<float>(ratio);
	float x = glm::cos(glm::radians(angle)) * rotRadius;
	float y = glm::sin(glm::radians(angle)) * rotRadius;
	float z = osc - (2.f * osc) * glm::cos(glm::radians(angle * 3.f));
	//Renderer::getInstance().drawPrimitive("torus", glm::translate(glm::mat4(), glm::vec3(x, y, z)) * glm::rotate(glm::mat4(), glm::radians(angle), glm::vec3(0.f, 1.f, 0.f)), glm::vec4(1.f, 0.f, 0.f, 1.f), glm::vec4(1.f), 10.f);
	//Renderer::getInstance().drawPrimitive("icosphere", glm::translate(glm::mat4(), glm::vec3(x, y, z)) * glm::scale(glm::mat4(), glm::vec3(0.5f)), glm::vec4(0.f, 1.f, 0.f, 1.f), glm::vec4(1.f), 10.f);

	g_pDiagram->draw();

	//Renderer::getInstance().drawPrimitive("bbox", glm::translate(glm::mat4(), -m_Head.pos - (g_vec3ScreenPos + g_vec3ScreenNormal * g_fDisplayDepth * 0.5f)) * glm::scale(glm::mat4(), glm::vec3(0.001f)), glm::vec4(1.f), glm::vec4(1.f), 10.f);

	{
		std::stringstream ss;
		ss.precision(2);

		ss << std::fixed << m_msFrameTime.count() << "ms/frame\n" << 1.f / std::chrono::duration_cast<std::chrono::duration<float>>(m_msFrameTime).count() << "fps";

		Renderer::getInstance().drawUIText(
			ss.str(),
			glm::vec4(1.f),
			glm::vec3(m_ivec2MainWindowSize.x, 0.f, 0.f),
			glm::quat(),
			40.f,
			Renderer::HEIGHT,
			Renderer::RIGHT,
			Renderer::BOTTOM_RIGHT
			);
	}

	// MUST be run last to xfer previous debug draw calls to opengl buffers
	DebugDrawer::getInstance().draw();
}

void Engine::render()
{
	Renderer::getInstance().sortTransparentObjects(m_Head.pos);

	if (g_bStereo)
	{
		Renderer::getInstance().RenderFrame(&m_sviLeftEyeInfo, &m_sviUIInfo, m_pLeftEyeFramebuffer);
		Renderer::getInstance().RenderFrame(&m_sviRightEyeInfo, &m_sviUIInfo, m_pRightEyeFramebuffer);
		Renderer::getInstance().RenderStereoTexture(m_ivec2MainWindowSize.x, m_ivec2MainWindowSize.y, m_pLeftEyeFramebuffer->m_nResolveTextureId, m_pRightEyeFramebuffer->m_nResolveTextureId);
	}
	else
	{
		Renderer::getInstance().RenderFrame(&m_sviMonoInfo, &m_sviUIInfo, m_pMonoFramebuffer);
		Renderer::getInstance().RenderFullscreenTexture(m_ivec2MainWindowSize.x, m_ivec2MainWindowSize.y, m_pMonoFramebuffer->m_nResolveTextureId);
	}
	
	glfwSwapBuffers(m_pMainWindow);
	
	Renderer::getInstance().clearDynamicRenderQueue();
	Renderer::getInstance().clearUIRenderQueue();
	DebugDrawer::getInstance().flushLines();
}

void Engine::listDisplayInfo()
{
	using namespace std::string_literals;

	int ndisp;
	GLFWmonitor** monitors = glfwGetMonitors(&ndisp);

	for (int i = 0; i < ndisp; ++i)
	{
		const std::string displayName(glfwGetMonitorName(monitors[i]));

		int wmm, hmm, x, y;
		glfwGetMonitorPhysicalSize(monitors[i], &wmm, &hmm);

		glfwGetMonitorPos(monitors[i], &x, &y);
		const GLFWvidmode* vidmode = glfwGetVideoMode(monitors[i]);

		std::cout << "Display " << i << ":" << std::endl;
		std::cout << "\tName:\t" << displayName << std::endl;
		std::cout << "\tBounds:\t" << wmm << "mmX" << hmm << "mm @ " << x << "," << y << std::endl;
		std::cout << "\tDisplay Mode:\t" << vidmode->width << "pxX" << vidmode->height << "px @ " << vidmode->refreshRate << "Hz" << std::endl;

		std::cout << std::endl;
	}
}

GLFWwindow * Engine::createWindow(GLFWmonitor* monitor, int width, int height, bool stereoContext)
{
	bool fullscreen = width == 0 && height == 0;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // remove deprecated funcs
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
	if (stereoContext)
	{
		glfwWindowHint(GLFW_STEREO, GL_TRUE);
		dprintf("%s - Requesting stereo OpenGL context...\n", __FUNCTION__);
	}
	
	const GLFWvidmode *mode = glfwGetVideoMode(monitor);

	int w = fullscreen ? mode->width : width;
	int h = fullscreen ? mode->height : height;

	GLFWwindow *ret = glfwCreateWindow(w, h, "CCOM VisLab", fullscreen ? monitor : NULL, NULL);

	if(stereoContext && !glfwGetWindowAttrib(ret, GLFW_STEREO))
		dprintf("%s - Error: The OpenGL window context is not stereo!\n", __FUNCTION__);

	glfwMakeContextCurrent(ret);

	glfwSwapInterval(0);

	return ret;
}

void Engine::createUIView()
{
	m_sviUIInfo.m_nRenderWidth = m_ivec2MainWindowSize.x;
	m_sviUIInfo.m_nRenderHeight = m_ivec2MainWindowSize.y;

	m_sviUIInfo.view = glm::mat4();
	m_sviUIInfo.projection = glm::ortho(0.f, static_cast<float>(m_sviUIInfo.m_nRenderWidth), 0.f, static_cast<float>(m_sviUIInfo.m_nRenderHeight), -1.f, 1.f);
}

// assumes that the center of the screen is the origin with +Z coming out of the screen
// all parameters are given in world space coordinates
glm::mat4 Engine::getViewingFrustum(glm::vec3 eyePos, glm::vec3 screenCenter, glm::vec3 screenNormal, glm::vec3 screenUp, glm::vec2 screenSize)
{
	glm::vec3 screenRight = glm::normalize(glm::cross(screenUp, screenNormal));

	float dist = -glm::dot(screenCenter - eyePos, screenNormal);

	float l, r, t, b, n, f;

	n = 1.f;
	f = dist + 100.f;

	// use similar triangles to scale to the near plane
	float nearScale = n / dist;

	l = ((screenCenter - screenRight * screenSize.x * 0.5f) - eyePos).x * nearScale;
	r = ((screenCenter + screenRight * screenSize.x * 0.5f) - eyePos).x * nearScale;
	b = ((screenCenter - screenUp * screenSize.y * 0.5f) - eyePos).y * nearScale;
	t = ((screenCenter + screenUp * screenSize.y * 0.5f) - eyePos).y * nearScale;

	return glm::frustum(l, r, b, t, n, f);
}

void Engine::createMonoView()
{
	float sizer = g_fDisplayDiag / sqrt(m_ivec2MainWindowSize.x * m_ivec2MainWindowSize.x + m_ivec2MainWindowSize.y * m_ivec2MainWindowSize.y);

	float width_cm = m_ivec2MainWindowSize.x * sizer; 
	float height_cm = m_ivec2MainWindowSize.y * sizer;

	m_sviMonoInfo.m_nRenderWidth = m_ivec2MainWindowSize.x;
	m_sviMonoInfo.m_nRenderHeight = m_ivec2MainWindowSize.y;

	m_pMonoFramebuffer = new Renderer::FramebufferDesc();

	if (!Renderer::getInstance().CreateFrameBuffer(m_sviMonoInfo.m_nRenderWidth, m_sviMonoInfo.m_nRenderHeight, *m_pMonoFramebuffer))
		dprintf("Could not create monocular framebuffer!\n");
}

void Engine::createStereoViews()
{
	// LEFT EYE
	m_sviLeftEyeInfo.m_nRenderWidth = m_ivec2MainWindowSize.x;
	m_sviLeftEyeInfo.m_nRenderHeight = m_ivec2MainWindowSize.y;

	// RIGHT EYE
	m_sviRightEyeInfo.m_nRenderWidth = m_ivec2MainWindowSize.x;
	m_sviRightEyeInfo.m_nRenderHeight = m_ivec2MainWindowSize.y;

	m_pLeftEyeFramebuffer = new Renderer::FramebufferDesc();
	m_pRightEyeFramebuffer = new Renderer::FramebufferDesc();

	if (!Renderer::getInstance().CreateFrameBuffer(m_sviLeftEyeInfo.m_nRenderWidth, m_sviLeftEyeInfo.m_nRenderHeight, *m_pLeftEyeFramebuffer))
		dprintf("Could not create left eye framebuffer!\n");
	if (!Renderer::getInstance().CreateFrameBuffer(m_sviRightEyeInfo.m_nRenderWidth, m_sviRightEyeInfo.m_nRenderHeight, *m_pRightEyeFramebuffer))
		dprintf("Could not create right eye framebuffer!\n");
}

#pragma once

#include <chrono>
#include <vector>

#include <GL/glew.h>
#include <Windows.h>
#include <GLFW/glfw3.h>
#include <gl/glu.h>
#include <glm.hpp>

#include "GLFWInputBroadcaster.h"
#include "Renderer.h"


//-----------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
class Engine : public GLFWInputObserver
{
public:
	Engine(int argc, char *argv[], int Mode);
	virtual ~Engine();

	bool init();
	bool initGL(bool stereoContext = false);

	void Shutdown();

	void RunMainLoop();
	void HandleInput();
	
private:
	std::chrono::duration<double, std::milli> m_msFrameTime, m_msInputHandleTime, m_msUpdateTime, m_msVRUpdateTime, m_msDrawTime, m_msRenderTime;

	bool m_bGLInitialized;

	void update();
	void makeScene();
	void makeDiagram();
	void render();

	void receive(void* data);

	Renderer::Eye m_Head;

private: // SDL bookkeeping
	void listDisplayInfo();
	// Use width = height = 0 for a fullscreen window
	GLFWwindow* createWindow(GLFWmonitor* monitor, int width = 800, int height = 600, bool stereoContext = false);

	GLFWwindow *m_pMainWindow;
	glm::ivec2 m_ivec2MainWindowSize;

private: // OpenGL bookkeeping
	void createMonoView();
	void createStereoViews();
	void createUIView();

	glm::mat4 getViewingFrustum(glm::vec3 eyePos, glm::vec3 screenCenter, glm::vec3 screenNormal, glm::vec3 screenUp, glm::vec2 screenSize);

	Renderer::FramebufferDesc *m_pMonoFramebuffer;
	Renderer::FramebufferDesc *m_pLeftEyeFramebuffer;
	Renderer::FramebufferDesc *m_pRightEyeFramebuffer;

	Renderer::SceneViewInfo m_sviMonoInfo;
	Renderer::SceneViewInfo m_sviLeftEyeInfo;
	Renderer::SceneViewInfo m_sviRightEyeInfo;
	Renderer::SceneViewInfo m_sviUIInfo;
};

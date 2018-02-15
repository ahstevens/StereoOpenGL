#pragma once
#include <vector>
#include <algorithm>
#include <iostream>

#include "GLFWInputBroadcaster.h"


GLFWInputBroadcaster::GLFWInputBroadcaster()
{
}

GLFWInputBroadcaster& GLFWInputBroadcaster::getInstance()
{
	static GLFWInputBroadcaster instance;
	return instance;
}

void GLFWInputBroadcaster::init(GLFWwindow * window)
{
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, mouse_position_callback);
	glfwSetScrollCallback(window, scroll_callback);

	memset(m_arrbActiveKeys, 0, sizeof m_arrbActiveKeys);
	m_bFirstMouse = true;
	m_bMousePressed = false;
	m_fLastMouseX = 0;
	m_fLastMouseY = 0;
}

void GLFWInputBroadcaster::addObserver(GLFWInputObserver * o)
{
	if (std::find(m_vpObservers.begin(), m_vpObservers.end(), o) == m_vpObservers.end())
		m_vpObservers.push_back(o);
}

void GLFWInputBroadcaster::removeObserver(GLFWInputObserver * o)
{
	m_vpObservers.erase(std::remove(m_vpObservers.begin(), m_vpObservers.end(), o), m_vpObservers.end());
}

void GLFWInputBroadcaster::notifyObservers(void* data)
{
	for (auto o : m_vpObservers)
		o->receive(data);
}

bool GLFWInputBroadcaster::keyPressed(const int glfwKeyCode)
{
	return m_arrbActiveKeys[glfwKeyCode];
}

bool GLFWInputBroadcaster::mousePressed()
{
	return m_bMousePressed;
}

void GLFWInputBroadcaster::poll()
{
	glfwPollEvents();
}

// Is called whenever a key is pressed/released via GLFW
void GLFWInputBroadcaster::key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{	
	if (key >= 0 && key < 1024)
	{
		int data[2];
		data[1] = key;

		if (action == GLFW_PRESS)
		{
			getInstance().m_arrbActiveKeys[key] = true;
			data[0] = EVENT::KEY_DOWN;
			getInstance().notifyObservers(data);
		}
		else if (action == GLFW_REPEAT)
		{
			data[0] = EVENT::KEY_HOLD;
			getInstance().notifyObservers(data);
		}
		else if (action == GLFW_RELEASE)
		{
			getInstance().m_arrbActiveKeys[key] = false;
			data[0] = EVENT::KEY_UP;
			getInstance().notifyObservers(data);
		}
	}
}

void GLFWInputBroadcaster::mouse_button_callback(GLFWwindow * window, int button, int action, int mods)
{
	int data[2];
	data[1] = button;

	if (action == GLFW_PRESS)
	{
		getInstance().m_bMousePressed = true;
		data[0] = EVENT::MOUSE_PRESS;
		getInstance().notifyObservers(data);
	}
	else if (action == GLFW_RELEASE)
	{
		getInstance().m_bMousePressed = false;
		data[0] = EVENT::MOUSE_UNPRESS;
		getInstance().notifyObservers(data);
	}
}

void GLFWInputBroadcaster::mouse_position_callback(GLFWwindow * window, double xpos, double ypos)
{
	if (getInstance().m_bFirstMouse)
	{
		getInstance().m_fLastMouseX = static_cast<GLfloat>(xpos);
		getInstance().m_fLastMouseY = static_cast<GLfloat>(ypos);
		getInstance().m_bFirstMouse = false;
	}

	GLfloat xoffset = static_cast<GLfloat>(xpos) - getInstance().m_fLastMouseX;
	GLfloat yoffset = getInstance().m_fLastMouseY - static_cast<GLfloat>(ypos);  // Reversed since y-coordinates go from bottom to left

	getInstance().m_fLastMouseX = static_cast<GLfloat>(xpos);
	getInstance().m_fLastMouseY = static_cast<GLfloat>(ypos);

	float offset[2] = { static_cast<float>(xoffset), static_cast<float>(yoffset) };

	struct mousedata {
		int eventType;
		float offsets[2];
	} data;
	data.eventType = EVENT::MOUSE_MOVE;
	data.offsets[0] = static_cast<float>(xoffset);
	data.offsets[1] = static_cast<float>(yoffset);
	getInstance().notifyObservers(&data);
}

void GLFWInputBroadcaster::scroll_callback(GLFWwindow * window, double xoffset, double yoffset)
{
	struct mousedata {
		int eventType;
		float offset;
	} data;
	data.eventType = EVENT::MOUSE_SCROLL;
	data.offset = static_cast<float>(yoffset);
	getInstance().notifyObservers(&data);
}

#pragma once
#include <vector>
#include <algorithm>

class GLFWwindow;

class GLFWInputObserver
{
	friend class GLFWInputBroadcaster;
	virtual void receive(void* data) = 0;
};

class GLFWInputBroadcaster
{
public:
	enum EVENT {
		MOUSE_PRESS,
		MOUSE_UNPRESS,
		MOUSE_MOVE,
		MOUSE_SCROLL,
		KEY_DOWN,
		KEY_UP,
		KEY_HOLD
	};

	static GLFWInputBroadcaster& getInstance();

	void init(GLFWwindow* window);

	void addObserver(GLFWInputObserver *o);
	void removeObserver(GLFWInputObserver *o);
	void notifyObservers(void* data);

	bool keyPressed(const int glfwKeyCode);

	bool mousePressed();

	void poll();

private:
	GLFWInputBroadcaster();

	std::vector<GLFWInputObserver*> m_vpObservers;

	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
	static void mouse_button_callback(GLFWwindow* window,int x, int y, int z);
	static void mouse_position_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

	bool m_arrbActiveKeys[1024];
	bool m_bFirstMouse, m_bMousePressed;
	float m_fLastMouseX, m_fLastMouseY;

	GLFWInputBroadcaster(GLFWInputBroadcaster const&) = delete; // no copies of singletons (C++11)
	void operator=(GLFWInputBroadcaster const&) = delete; // no assigning of singletons (C++11)
};

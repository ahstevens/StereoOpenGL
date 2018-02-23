#include "UntrackedStereoDiagram.h"
#include <GL/glew.h>
#include "DebugDrawer.h"
#include "Renderer.h"

UntrackedStereoDiagram::UntrackedStereoDiagram(glm::mat4 screenTransform, glm::ivec2 screenResolution)
	: m_mat4ScreenTransform(screenTransform)
	, m_ivec2ScreenRes(screenResolution)
{
}

UntrackedStereoDiagram::~UntrackedStereoDiagram()
{
}

void UntrackedStereoDiagram::draw()
{
	float screenHeight = glm::length(m_mat4ScreenTransform[1]) * 2.f;
	glm::vec3 screenRight = glm::normalize(m_mat4ScreenTransform[0]);
	glm::vec3 screenUp = glm::normalize(m_mat4ScreenTransform[1]);
	glm::vec3 screenNorm = glm::normalize(m_mat4ScreenTransform[2]);

	float viewingArcRad = screenHeight / 3.f;
	float eyerad = screenHeight / 30.f;
	float hingeLen = 2.f;
	float hingDist = 5.f;

	glm::vec3 origin = glm::vec3(m_mat4ScreenTransform[3]);

	glm::mat4 trans(glm::vec4(screenRight, 0.f),
		glm::vec4(screenUp, 0.f),
		glm::vec4(screenNorm, 0.f),
		glm::vec4(origin, 1.f));

	DebugDrawer::getInstance().setTransform(trans);
	//DebugDrawer::getInstance().drawTransform(1.f);
	
	// Screen
	DebugDrawer::getInstance().drawLine(glm::vec3(viewingArcRad, 0.f, 0.f), glm::vec3(-viewingArcRad, 0.f, 0.f));

	// Viewing Arc
	DebugDrawer::getInstance().drawArc(viewingArcRad, viewingArcRad, 180.f, 360.f, glm::vec4(0.f, 1.f, 1.f, 1.f), false);

	// Eye
	DebugDrawer::getInstance().setTransform(glm::translate(trans, glm::vec3(0.f, -viewingArcRad, 0.f)));
	DebugDrawer::getInstance().drawArc(eyerad, eyerad, 0.f, 360.f, glm::vec4(1.f, 1.f, 0.f, 1.f), false);

	// Hinge
	DebugDrawer::getInstance().setTransform(glm::translate(trans, glm::vec3(0.f, hingDist, 0.f)));
	DebugDrawer::getInstance().drawArc(0.1f, 0.1f, 0.f, 360.f, glm::vec4(1.f, 1.f, 1.f, 1.f), false);
	Renderer::getInstance().drawText(
		"P2",
		glm::vec4(1.f),
		origin + screenUp * (hingDist + 0.2f),
		glm::quat(trans),
		0.2f,
		Renderer::HEIGHT,
		Renderer::CENTER,
		Renderer::CENTER_BOTTOM);

}

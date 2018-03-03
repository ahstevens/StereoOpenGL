#include "Clapboard.h"
#include "Renderer.h"
#include <gtc/matrix_transform.hpp>

Clapboard::Clapboard(float length, float angle)
	: m_fLength(length)
	, m_fAngle(angle)
{
}

Clapboard::~Clapboard()
{
}

float Clapboard::getAngle()
{
	return m_fAngle;
}

void Clapboard::setAngle(float angle)
{
	m_fAngle = angle;
}

float Clapboard::getLength()
{
	return m_fLength;
}

void Clapboard::setLength(float length)
{
	m_fLength = length;
}

void Clapboard::draw()
{
	float xoffset = (m_fLength / 2.f) * glm::sin(glm::radians(m_fAngle / 2.f));
	float zoffset = (m_fLength / 2.f) * glm::cos(glm::radians(m_fAngle / 2.f));

	// Left Plane
	Renderer::getInstance().drawPrimitiveCustom(
		"quaddouble",
		glm::translate(glm::mat4(), glm::vec3(-xoffset, 0.f, zoffset)) * glm::rotate(glm::mat4(), glm::radians(270.f - m_fAngle / 2.f), glm::vec3(0.f, 1.f, 0.f)) * glm::scale(glm::mat4(), glm::vec3(m_fLength)),
		"grid");
	Renderer::getInstance().drawPrimitiveCustom(
		"quaddouble",
		glm::translate(glm::mat4(), glm::vec3(-xoffset, 0.f, zoffset)) * glm::rotate(glm::mat4(), glm::radians(270.f - m_fAngle / 2.f), glm::vec3(0.f, 1.f, 0.f)) * glm::scale(glm::mat4(), glm::vec3(m_fLength)),
		"shadow");

	// Right Plane
	Renderer::getInstance().drawPrimitiveCustom(
		"quaddouble",
		glm::translate(glm::mat4(), glm::vec3(xoffset, 0.f, zoffset)) * glm::rotate(glm::mat4(), glm::radians(270.f + m_fAngle / 2.f), glm::vec3(0.f, 1.f, 0.f)) * glm::scale(glm::mat4(), glm::vec3(m_fLength)),
		"grid");
	Renderer::getInstance().drawPrimitiveCustom(
		"quaddouble",
		glm::translate(glm::mat4(), glm::vec3(xoffset, 0.f, zoffset)) * glm::rotate(glm::mat4(), glm::radians(270.f + m_fAngle / 2.f), glm::vec3(0.f, 1.f, 0.f)) * glm::scale(glm::mat4(), glm::vec3(m_fLength)),
		"shadow");
}

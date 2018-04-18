#include "Rod.h"
#include "Renderer.h"
#include <gtc/matrix_transform.hpp>

Rod::Rod(float length, float angle)
	: m_vec3Pos(glm::vec3(0.f))
	, m_fLength(length)
	, m_fAngle(angle)
{
	Renderer::getInstance().addTexture(new GLTexture("wood.png", false));
	Renderer::getInstance().addTexture(new GLTexture("noise1.png", false));
	Renderer::getInstance().addTexture(new GLTexture("noise2.png", false));
}

Rod::~Rod()
{
}

glm::vec3 Rod::getPos()
{
	return m_vec3Pos;
}

void Rod::setPos(glm::vec3 pos)
{
	m_vec3Pos = pos;
}

float Rod::getAngle()
{
	return m_fAngle;
}

void Rod::setAngle(float angle)
{
	m_fAngle = angle;
}

float Rod::getLength()
{
	return m_fLength;
}

void Rod::setLength(float length)
{
	m_fLength = length;
}

void Rod::draw()
{
	Renderer::RendererSubmission rs;
	rs.glPrimitiveType = GL_TRIANGLES;
	rs.shaderName = "gridflat";
	rs.VAO = Renderer::getInstance().getPrimitiveVAO("cylinder");
	rs.vertCount = Renderer::getInstance().getPrimitiveIndexCount("cylinder");
	rs.indexType = GL_UNSIGNED_SHORT;
	rs.diffuseTexName = "wood.png";
	rs.diffuseColor = glm::vec4(glm::vec3(0.75f), 1.f);
	rs.hasTransparency = rs.diffuseColor.a != 1.f;
	rs.specularColor = glm::vec4(glm::vec3(0.25f), 1.f);

	float xoffset = (m_fLength / 2.f) * glm::sin(glm::radians(m_fAngle / 2.f));
	float zoffset = (m_fLength / 2.f) * glm::cos(glm::radians(m_fAngle / 2.f));

	// Left Plane
	rs.modelToWorldTransform = glm::rotate(glm::mat4(), glm::radians(m_fAngle), glm::vec3(0.f, 0.f, 1.f)) * glm::rotate(glm::mat4(), glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f)) * glm::scale(glm::mat4(), glm::vec3(1.f, 1.f, m_fLength));
	Renderer::getInstance().addToDynamicRenderQueue(rs);
}

void Rod::drawShadow()
{	
	Renderer::RendererSubmission rs;
	rs.glPrimitiveType = GL_TRIANGLES;
	rs.shaderName = "shadow";
	rs.VAO = Renderer::getInstance().getPrimitiveVAO("quaddouble");
	rs.vertCount = Renderer::getInstance().getPrimitiveIndexCount("quaddouble");
	rs.indexType = GL_UNSIGNED_SHORT;
	rs.diffuseTexName = "white";
	rs.diffuseColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.f);
	rs.hasTransparency = rs.diffuseColor.a != 1.f;
	rs.specularTexName = "white";
	rs.specularExponent = 110.f;

	float xoffset = (m_fLength / 2.f) * glm::sin(glm::radians(m_fAngle / 2.f));
	float zoffset = (m_fLength / 2.f) * glm::cos(glm::radians(m_fAngle / 2.f));

	// Left Plane
	rs.modelToWorldTransform = glm::translate(glm::mat4(), m_vec3Pos + glm::vec3(-xoffset, 0.f, zoffset)) * glm::rotate(glm::mat4(), glm::radians(270.f - m_fAngle / 2.f), glm::vec3(0.f, 1.f, 0.f)) * glm::scale(glm::mat4(), glm::vec3(m_fLength));
	Renderer::getInstance().addToDynamicRenderQueue(rs);

	// Right Plane
	rs.modelToWorldTransform = glm::translate(glm::mat4(), m_vec3Pos + glm::vec3(xoffset, 0.f, zoffset)) * glm::rotate(glm::mat4(), glm::radians(90.f + m_fAngle / 2.f), glm::vec3(0.f, 1.f, 0.f)) * glm::scale(glm::mat4(), glm::vec3(m_fLength));
	Renderer::getInstance().addToDynamicRenderQueue(rs);
}

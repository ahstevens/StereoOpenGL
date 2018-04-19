#include "ViewingConditionsDiagram.h"
#include <GL/glew.h>
#include "DebugDrawer.h"
#include "Renderer.h"
#include "DistortionUtils.h"
#include <sstream>
#include <gtx/vector_angle.hpp>

ViewingConditionsDiagram::ViewingConditionsDiagram(glm::mat4 screenBasis, glm::ivec2 screenResolution)
	: m_mat4ScreenBasis(screenBasis)
	, m_ivec2ScreenRes(screenResolution)
	, m_fOriginScreenDist(5.f)
	, m_fHingeAngle(90.f)
	, m_fHingeLength(2.5f)
	, m_fProjectionAngle(0.f)
	, m_fProjectionDistance(glm::length(screenBasis[1]) * (2.f / 3.f))
	, m_fViewingAngle(0.f)
	, m_fViewingDistance(glm::length(screenBasis[1]) * (2.f / 3.f))
	, m_fEyeSeparation(2.f)
	, m_fScreenWidth(glm::length(screenBasis[1]))
{
	m_mat4ScreenBasisOrtho = glm::mat4(
		glm::normalize(m_mat4ScreenBasis[0]),
		glm::normalize(m_mat4ScreenBasis[1]),
		glm::normalize(m_mat4ScreenBasis[2]),
		m_mat4ScreenBasis[3]
	);

	m_vec3ObjOrigin = glm::translate(m_mat4ScreenBasisOrtho, glm::vec3(0.f, m_fOriginScreenDist, 0.f))[3];
}

ViewingConditionsDiagram::~ViewingConditionsDiagram()
{
}

void ViewingConditionsDiagram::draw()
{
	glm::vec3 screenOrigin = glm::vec3(m_mat4ScreenBasis[3]);

	DebugDrawer::getInstance().setTransform(m_mat4ScreenBasisOrtho);
	
	// Screen
	DebugDrawer::getInstance().drawLine(glm::vec3(m_fScreenWidth, 0.f, 0.f), glm::vec3(-m_fScreenWidth, 0.f, 0.f));

	// Viewing Arc
	DebugDrawer::getInstance().drawArc(m_fViewingDistance, m_fViewingDistance, 180.f, 360.f, glm::vec4(0.f, 1.f, 1.f, 1.f), false);

	float projAngleOffset = glm::degrees(glm::asin(m_fEyeSeparation / (2.f * m_fProjectionDistance)));

	// Center of Projection
	glm::mat4 copBasis = glm::rotate(glm::mat4(), glm::radians(m_fProjectionAngle), glm::vec3(m_mat4ScreenBasisOrtho[2])) * glm::translate(m_mat4ScreenBasisOrtho, glm::vec3(0.f, -m_fProjectionDistance, 0.f));
	glm::vec3 copPos(copBasis[3]);
	glm::vec3 copLeft = (glm::rotate(glm::mat4(), glm::radians(m_fProjectionAngle - projAngleOffset), glm::vec3(m_mat4ScreenBasisOrtho[2])) * glm::translate(m_mat4ScreenBasisOrtho, glm::vec3(0.f, -m_fProjectionDistance, 0.f)))[3];
	glm::vec3 copRight = (glm::rotate(glm::mat4(), glm::radians(m_fProjectionAngle + projAngleOffset), glm::vec3(m_mat4ScreenBasisOrtho[2])) * glm::translate(m_mat4ScreenBasisOrtho, glm::vec3(0.f, -m_fProjectionDistance, 0.f)))[3];

	glm::mat4 viewBasis = glm::rotate(glm::mat4(), glm::radians(m_fViewingAngle), glm::vec3(m_mat4ScreenBasisOrtho[2])) * glm::translate(m_mat4ScreenBasisOrtho, glm::vec3(0.f, -m_fViewingDistance, 0.f));
	glm::vec3 viewPos(viewBasis[3]);

	glm::vec3 screenViewVec = screenOrigin - viewPos;

	glm::vec3 screenViewVecPerp = glm::cross(glm::vec3(m_mat4ScreenBasisOrtho[2]), glm::normalize(screenViewVec));

	float viewAngleOffset = glm::degrees(glm::asin(m_fEyeSeparation / (2.f * m_fViewingDistance)));

	glm::vec3 leftEyePos = (glm::rotate(glm::mat4(), glm::radians(m_fViewingAngle - viewAngleOffset), glm::vec3(m_mat4ScreenBasisOrtho[2])) * glm::translate(m_mat4ScreenBasisOrtho, glm::vec3(0.f, -m_fViewingDistance, 0.f)))[3];
	glm::vec3 rightEyePos = (glm::rotate(glm::mat4(), glm::radians(m_fViewingAngle + viewAngleOffset), glm::vec3(m_mat4ScreenBasisOrtho[2])) * glm::translate(m_mat4ScreenBasisOrtho, glm::vec3(0.f, -m_fViewingDistance, 0.f)))[3];

	// Hinge
	auto hingePts = getHinge();

	if (false)
	{
		drawEye(copPos, glm::vec4(1.f, 1.f, 1.f, 0.5f), glm::vec4(1.f, 1.f, 1.f, 0.8f), hingePts);
		drawOBJ(hingePts, glm::vec4(1.f));
		drawHingeAngle(hingePts, glm::vec4(1.f));

		

		auto xformed = distutil::transformMonoscopicPoints(copPos, viewPos, glm::vec3(m_mat4ScreenBasisOrtho[3]), -glm::vec3(m_mat4ScreenBasisOrtho[1]), hingePts);
		drawEye(leftEyePos, glm::vec4(1.f, 0.f, 0.f, 0.5f), glm::vec4(1.f, 0.f, 0.f, 1.f), xformed);
		drawEye(rightEyePos, glm::vec4(0.f, 1.f, 0.f, 0.5f), glm::vec4(0.f, 1.f, 0.f, 1.f), xformed);
		drawOBJ(xformed, glm::vec4(0.8f));
		drawHingeAngle(xformed, glm::vec4(1.f));

		// Calc intersection points
		for (auto i : distutil::getScreenIntersections(copPos, glm::vec3(m_mat4ScreenBasisOrtho[3]), -glm::vec3(m_mat4ScreenBasisOrtho[1]), hingePts))
		{
			DebugDrawer::getInstance().setTransform(glm::translate(glm::mat4(), i));
			DebugDrawer::getInstance().drawArc(0.1f, 0.1f, 0.f, 360.f, glm::vec4(1.f, 0.f, 1.f, 1.f), false);
		}
	}
	else
	{
		drawEye(copLeft, glm::vec4(1.f, 1.f, 1.f, 0.5f), glm::vec4(0.5f, 0.f, 0.f, 0.5f), hingePts);
		drawEye(copRight, glm::vec4(1.f, 1.f, 1.f, 0.5f), glm::vec4(0.f, 0.5f, 0.f, 0.5f), hingePts);
		drawOBJ(hingePts, glm::vec4(1.f));
		drawHingeAngle(hingePts, glm::vec4(1.f));

		auto xformed = distutil::transformStereoscopicPoints(copLeft, copRight, leftEyePos, rightEyePos, glm::vec3(m_mat4ScreenBasisOrtho[3]), -glm::vec3(m_mat4ScreenBasisOrtho[1]), hingePts);
		drawEye(leftEyePos, glm::vec4(1.f, 0.f, 0.f, 0.5f), glm::vec4(1.f, 0.f, 0.f, 1.f), xformed);
		drawEye(rightEyePos, glm::vec4(0.f, 1.f, 0.f, 0.5f), glm::vec4(0.f, 1.f, 0.f, 1.f), xformed);
		drawOBJ(xformed, glm::vec4(1.f, 1.f, 0.f, 0.8f));
		drawHingeAngle(xformed, glm::vec4(1.f, 1.f, 0.f, 1.f));

		auto iL = distutil::getScreenIntersections(copLeft, glm::vec3(m_mat4ScreenBasisOrtho[3]), -glm::vec3(m_mat4ScreenBasisOrtho[1]), hingePts);
		auto iR = distutil::getScreenIntersections(copRight, glm::vec3(m_mat4ScreenBasisOrtho[3]), -glm::vec3(m_mat4ScreenBasisOrtho[1]), hingePts);

		// Calc intersection points
		for (int i = 0; i < hingePts.size(); ++i)
		{
			DebugDrawer::getInstance().setTransform(glm::translate(glm::mat4(), iL[i]));
			DebugDrawer::getInstance().drawArc(0.1f, 0.1f, 0.f, 360.f, glm::vec4(1.f, 0.f, 1.f, 1.f), false);

			DebugDrawer::getInstance().setTransform(glm::translate(glm::mat4(), iR[i]));
			DebugDrawer::getInstance().drawArc(0.1f, 0.1f, 0.f, 360.f, glm::vec4(1.f, 0.f, 1.f, 1.f), false);
		}
	}
}

float ViewingConditionsDiagram::getViewAngle()
{
	return m_fViewingAngle;
}

void ViewingConditionsDiagram::setViewAngle(float angle)
{
	if (angle > -90.f && angle < 90.f)
		m_fViewingAngle = angle;
}

float ViewingConditionsDiagram::getViewDistance()
{
	return m_fViewingDistance;
}

void ViewingConditionsDiagram::setViewDistance(float dist)
{
	if (dist > 0.f)
		m_fViewingDistance = dist;
}

float ViewingConditionsDiagram::getProjectionAngle()
{
	return m_fProjectionAngle;
}

void ViewingConditionsDiagram::setProjectionAngle(float angle)
{
	if (angle > -90.f && angle < 90.f)
		m_fProjectionAngle = angle;
}

float ViewingConditionsDiagram::getEyeSeparation()
{
	return m_fEyeSeparation;
}

void ViewingConditionsDiagram::setEyeSeparation(float dist)
{
	if (dist >= 0.f )
		m_fEyeSeparation = dist;
}

float ViewingConditionsDiagram::getHingeAngle()
{
	return m_fHingeAngle;
}

void ViewingConditionsDiagram::setHingeAngle(float angle)
{
	m_fHingeAngle = angle;
}

void ViewingConditionsDiagram::drawOBJ(std::vector<glm::vec3> obj, glm::vec4 col)
{
	unsigned int i = 0;
	for (auto pt : obj)
	{
		DebugDrawer::getInstance().setTransform(glm::translate(glm::mat4(), pt));
		DebugDrawer::getInstance().drawArc(0.1f, 0.1f, 0.f, 360.f, col, false);
		Renderer::getInstance().drawText(
			std::to_string(i++),
			col,
			pt + glm::vec3(0.f, 0.2f, 0.f),
			glm::quat(m_mat4ScreenBasisOrtho),
			0.5f,
			Renderer::HEIGHT,
			Renderer::CENTER,
			Renderer::CENTER_BOTTOM
		);
	}

	DebugDrawer::getInstance().setTransformDefault();

	for (i = 0; i < obj.size() - 1; ++i)
		DebugDrawer::getInstance().drawLine(obj[i], obj[i + 1], col);
}

void ViewingConditionsDiagram::drawEye(glm::vec3 eyePos, glm::vec4 eyeCol, glm::vec4 rayCol, std::vector<glm::vec3> obj)
{
	float eyerad = glm::length(m_mat4ScreenBasis[1]) / 15.f;

	DebugDrawer::getInstance().setTransform(glm::translate(glm::mat4(), eyePos));
	//DebugDrawer::getInstance().drawArc(eyerad, eyerad, 0.f, 360.f, eyeCol, false);
	Renderer::getInstance().drawPrimitive("icosphere", glm::translate(glm::mat4(), eyePos) * glm::scale(glm::vec3(eyerad)), eyeCol, glm::vec4(1.f), 10.f);

	DebugDrawer::getInstance().setTransformDefault();
	for (auto pt : obj)
		DebugDrawer::getInstance().drawLine(eyePos, pt, rayCol);
}

std::vector<glm::vec3> ViewingConditionsDiagram::getHinge()
{
	float s = 2.f * m_fHingeLength * glm::tan(glm::radians(m_fHingeAngle) / 2.f);
	glm::vec3 p1 = m_vec3ObjOrigin - glm::vec3(m_mat4ScreenBasisOrtho[1]) * m_fHingeLength - glm::vec3(m_mat4ScreenBasisOrtho[0]) * (s / 2.f);
	glm::vec3 p2 = m_vec3ObjOrigin;
	glm::vec3 p3 = p1 + glm::vec3(m_mat4ScreenBasisOrtho[0]) * s;

	return std::vector<glm::vec3>({ p1, p2, p3 });
}

void ViewingConditionsDiagram::drawHingeAngle(std::vector<glm::vec3> pts, glm::vec4 col)
{
	float rad = 0.3f;

	glm::vec3 a(glm::normalize(pts[0] - pts[1]));
	glm::vec3 b(glm::normalize(pts[2] - pts[1]));
	
	float arcstart = 360.f - glm::degrees(glm::angle(a, glm::vec3(m_mat4ScreenBasisOrtho[0])));
	float arcend = 360.f - glm::degrees(glm::angle(b, glm::vec3(m_mat4ScreenBasisOrtho[0])));
	float angle = glm::degrees(glm::acos(glm::dot(a, b)));
	
	glm::vec3 halfVec = glm::normalize(a + b);

	std::stringstream ss;
	ss.precision(1);

	ss << std::fixed << angle;

	DebugDrawer::getInstance().setTransform(glm::mat4(m_mat4ScreenBasisOrtho[0], m_mat4ScreenBasisOrtho[1], m_mat4ScreenBasisOrtho[2], glm::vec4(pts[1], 1.f)));
	DebugDrawer::getInstance().drawArc(rad, rad, arcstart, arcend, col, false);

	glm::mat4 textTrans(
		glm::vec4(glm::normalize(b - a), 0.f),
		glm::vec4(-halfVec, 0.f),
		m_mat4ScreenBasisOrtho[2],
		glm::vec4(pts[1] + halfVec * (rad + 0.1f), 1.f)
	);

	Renderer::getInstance().drawText(
		ss.str(),
		col,
		glm::vec3(textTrans[3]),
		glm::quat(textTrans),
		0.5f,
		Renderer::HEIGHT,
		Renderer::CENTER,
		Renderer::CENTER_TOP
	);
}

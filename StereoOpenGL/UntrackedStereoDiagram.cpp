#include "UntrackedStereoDiagram.h"
#include <GL/glew.h>
#include "DebugDrawer.h"
#include "Renderer.h"
#include <gtx/intersect.hpp>
#include <gtx/vector_angle.hpp>
#include <sstream>

UntrackedStereoDiagram::UntrackedStereoDiagram(glm::mat4 screenBasis, glm::ivec2 screenResolution)
	: m_mat4ScreenBasis(screenBasis)
	, m_ivec2ScreenRes(screenResolution)
	, m_fOriginScreenDist(5.f)
	, m_fHingeAngle(90.f)
	, m_fHingeLength(2.5f)
	, m_fProjectionAngle(0.f)
	, m_fViewingAngle(0.f)
	, m_fViewingArcRadius(glm::length(screenBasis[1]) * (2.f / 3.f))
	, m_fEyeSeparation(2.f)
{
	m_mat4ScreenBasisOrtho = glm::mat4(
		glm::normalize(m_mat4ScreenBasis[0]),
		glm::normalize(m_mat4ScreenBasis[1]),
		glm::normalize(m_mat4ScreenBasis[2]),
		m_mat4ScreenBasis[3]
	);

	m_vec3ObjOrigin = glm::translate(m_mat4ScreenBasisOrtho, glm::vec3(0.f, m_fOriginScreenDist, 0.f))[3];
}

UntrackedStereoDiagram::~UntrackedStereoDiagram()
{
}

void UntrackedStereoDiagram::draw()
{
	glm::vec3 screenOrigin = glm::vec3(m_mat4ScreenBasis[3]);

	DebugDrawer::getInstance().setTransform(m_mat4ScreenBasisOrtho);
	
	// Screen
	DebugDrawer::getInstance().drawLine(glm::vec3(m_fViewingArcRadius, 0.f, 0.f), glm::vec3(-m_fViewingArcRadius, 0.f, 0.f));

	// Viewing Arc
	DebugDrawer::getInstance().drawArc(m_fViewingArcRadius, m_fViewingArcRadius, 180.f, 360.f, glm::vec4(0.f, 1.f, 1.f, 1.f), false);

	// Center of Projection
	glm::mat4 copBasis = glm::rotate(glm::translate(m_mat4ScreenBasisOrtho, glm::vec3(0.f, -m_fViewingArcRadius, 0.f)), glm::radians(m_fProjectionAngle), glm::vec3(m_mat4ScreenBasisOrtho[2]));
	glm::vec3 copPos(copBasis[3]);

	glm::mat4 viewBasis = glm::rotate(glm::mat4(), glm::radians(m_fViewingAngle), glm::vec3(m_mat4ScreenBasisOrtho[2])) * glm::translate(m_mat4ScreenBasisOrtho, glm::vec3(0.f, -m_fViewingArcRadius, 0.f));
	glm::vec3 viewPos(viewBasis[3]);

	glm::vec3 screenViewVec = screenOrigin - viewPos;

	glm::vec3 screenViewVecPerp = glm::cross(glm::vec3(m_mat4ScreenBasisOrtho[2]), glm::normalize(screenViewVec));

	glm::vec3 leftEyePos = viewPos + screenViewVecPerp * m_fEyeSeparation / 2.f;
	glm::vec3 rightEyePos = viewPos - screenViewVecPerp * m_fEyeSeparation / 2.f;

	// Hinge
	auto hingePts = getHinge();

	drawEye(copPos, glm::vec4(1.f, 1.f, 0.f, 1.f), hingePts, glm::vec4(1.f), glm::vec4(1.f, 1.f, 1.f, 0.8f));

	drawEye(leftEyePos, glm::vec4(1.f, 0.f, 0.f, 1.f), transformMonoscopicPoints(copPos, hingePts, viewPos), glm::vec4(1.f), glm::vec4(1.f, 0.f, 0.f, 1.f));
	drawEye(rightEyePos, glm::vec4(0.f, 1.f, 0.f, 1.f), transformMonoscopicPoints(copPos, hingePts, viewPos), glm::vec4(1.f), glm::vec4(0.f, 1.f, 0.f, 1.f));

	drawHingeAngle(hingePts, glm::vec4(1.f));
	drawHingeAngle(transformMonoscopicPoints(copPos, hingePts, viewPos), glm::vec4(1.f));

	// Calc intersection points
	for (auto i : getScreenIntersections(copPos, hingePts))
	{
		DebugDrawer::getInstance().setTransform(glm::translate(glm::mat4(), i));
		DebugDrawer::getInstance().drawArc(0.1f, 0.1f, 0.f, 360.f, glm::vec4(1.f, 0.f, 1.f, 1.f), false);
	}
}

float UntrackedStereoDiagram::getViewAngle()
{
	return m_fViewingAngle;
}

void UntrackedStereoDiagram::setViewAngle(float angle)
{
	if (angle > -90.f && angle < 90.f)
		m_fViewingAngle = angle;
}

void UntrackedStereoDiagram::drawOBJ(std::vector<glm::vec3> obj, glm::vec4 col)
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
			0.2f,
			Renderer::HEIGHT,
			Renderer::CENTER,
			Renderer::CENTER_BOTTOM
		);
	}

	DebugDrawer::getInstance().setTransformDefault();

	for (i = 0; i < obj.size() - 1; ++i)
		DebugDrawer::getInstance().drawLine(obj[i], obj[i + 1]);
}

void UntrackedStereoDiagram::drawEye(glm::vec3 eyePos, glm::vec4 eyeCol, std::vector<glm::vec3> obj, glm::vec4 objCol, glm::vec4 rayCol)
{
	float eyerad = glm::length(m_mat4ScreenBasis[1]) / 15.f;

	DebugDrawer::getInstance().setTransform(glm::translate(glm::mat4(), eyePos));
	DebugDrawer::getInstance().drawArc(eyerad, eyerad, 0.f, 360.f, eyeCol, false);

	drawOBJ(obj, objCol);

	DebugDrawer::getInstance().setTransformDefault();
	for (auto pt : obj)
		DebugDrawer::getInstance().drawLine(eyePos, pt, rayCol);
}

std::vector<glm::vec3> UntrackedStereoDiagram::getHinge()
{
	float s = 2.f * m_fHingeLength * glm::tan(glm::radians(m_fHingeAngle) / 2.f);
	glm::vec3 p1 = m_vec3ObjOrigin - glm::vec3(m_mat4ScreenBasisOrtho[1]) * m_fHingeLength - glm::vec3(m_mat4ScreenBasisOrtho[0]) * (s / 2.f);
	glm::vec3 p2 = m_vec3ObjOrigin;
	glm::vec3 p3 = p1 + glm::vec3(m_mat4ScreenBasisOrtho[0]) * s;

	return std::vector<glm::vec3>({ p1, p2, p3 });
}

void UntrackedStereoDiagram::drawHingeAngle(std::vector<glm::vec3> pts, glm::vec4 col)
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
		0.2f,
		Renderer::HEIGHT,
		Renderer::CENTER,
		Renderer::CENTER_TOP
	);
}

std::vector<glm::vec3> UntrackedStereoDiagram::transformMonoscopicPoints(glm::vec3 centerOfProj, std::vector<glm::vec3> obj, glm::vec3 viewPos)
{
	auto intPts = getScreenIntersections(centerOfProj, obj);

	std::vector<glm::vec3> ret;

	for (int i = 0; i < obj.size(); ++i)
	{
		float ratio = glm::length(obj[i] - intPts[i]) / glm::length(intPts[i] - centerOfProj);
		glm::vec3 newPosToInt = intPts[i] - viewPos;
		float transformedOffset = ratio * glm::length(newPosToInt);

		ret.push_back(intPts[i] + glm::normalize(newPosToInt) * transformedOffset);
	}

	return ret;
}

std::vector<glm::vec3> UntrackedStereoDiagram::getScreenIntersections(glm::vec3 centerOfProjection, std::vector<glm::vec3> pts)
{
	std::vector<glm::vec3> ret;

	for (auto pt : pts)
	{
		float ptDist;
		glm::intersectRayPlane(centerOfProjection, pt - centerOfProjection, glm::vec3(m_mat4ScreenBasisOrtho[3]), -glm::vec3(m_mat4ScreenBasisOrtho[1]), ptDist);
		ret.push_back(centerOfProjection + (pt - centerOfProjection) * ptDist);
	}
	return ret;
}

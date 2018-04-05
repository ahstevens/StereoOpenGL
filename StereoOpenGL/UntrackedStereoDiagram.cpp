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

UntrackedStereoDiagram::~UntrackedStereoDiagram()
{
}

void UntrackedStereoDiagram::draw()
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

		auto xformed = transformMonoscopicPoints(copPos, viewPos, hingePts);
		drawEye(leftEyePos, glm::vec4(1.f, 0.f, 0.f, 0.5f), glm::vec4(1.f, 0.f, 0.f, 1.f), xformed);
		drawEye(rightEyePos, glm::vec4(0.f, 1.f, 0.f, 0.5f), glm::vec4(0.f, 1.f, 0.f, 1.f), xformed);
		drawOBJ(xformed, glm::vec4(0.8f));
		drawHingeAngle(xformed, glm::vec4(1.f));

		// Calc intersection points
		for (auto i : getScreenIntersections(copPos, hingePts))
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

		auto xformed = transformStereoscopicPoints(copLeft, copRight, leftEyePos, rightEyePos, hingePts);
		drawEye(leftEyePos, glm::vec4(1.f, 0.f, 0.f, 0.5f), glm::vec4(1.f, 0.f, 0.f, 1.f), xformed);
		drawEye(rightEyePos, glm::vec4(0.f, 1.f, 0.f, 0.5f), glm::vec4(0.f, 1.f, 0.f, 1.f), xformed);
		drawOBJ(xformed, glm::vec4(1.f, 1.f, 0.f, 0.8f));
		drawHingeAngle(xformed, glm::vec4(1.f, 1.f, 0.f, 1.f));

		auto iL = getScreenIntersections(copLeft, hingePts);
		auto iR = getScreenIntersections(copRight, hingePts);

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

float UntrackedStereoDiagram::getViewAngle()
{
	return m_fViewingAngle;
}

void UntrackedStereoDiagram::setViewAngle(float angle)
{
	if (angle > -90.f && angle < 90.f)
		m_fViewingAngle = angle;
}

float UntrackedStereoDiagram::getViewDistance()
{
	return m_fViewingDistance;
}

void UntrackedStereoDiagram::setViewDistance(float dist)
{
	if (dist > 0.f)
		m_fViewingDistance = dist;
}

float UntrackedStereoDiagram::getProjectionAngle()
{
	return m_fProjectionAngle;
}

void UntrackedStereoDiagram::setProjectionAngle(float angle)
{
	if (angle > -90.f && angle < 90.f)
		m_fProjectionAngle = angle;
}

float UntrackedStereoDiagram::getEyeSeparation()
{
	return m_fEyeSeparation;
}

void UntrackedStereoDiagram::setEyeSeparation(float dist)
{
	if (dist >= 0.f )
		m_fEyeSeparation = dist;
}

float UntrackedStereoDiagram::getHingeAngle()
{
	return m_fHingeAngle;
}

void UntrackedStereoDiagram::setHingeAngle(float angle)
{
	m_fHingeAngle = angle;
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

void UntrackedStereoDiagram::drawEye(glm::vec3 eyePos, glm::vec4 eyeCol, glm::vec4 rayCol, std::vector<glm::vec3> obj)
{
	float eyerad = glm::length(m_mat4ScreenBasis[1]) / 15.f;

	DebugDrawer::getInstance().setTransform(glm::translate(glm::mat4(), eyePos));
	//DebugDrawer::getInstance().drawArc(eyerad, eyerad, 0.f, 360.f, eyeCol, false);
	Renderer::getInstance().drawPrimitive("icosphere", glm::translate(glm::mat4(), eyePos) * glm::scale(glm::vec3(eyerad)), eyeCol, glm::vec4(1.f), 10.f);

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
		0.5f,
		Renderer::HEIGHT,
		Renderer::CENTER,
		Renderer::CENTER_TOP
	);
}

std::vector<glm::vec3> UntrackedStereoDiagram::transformMonoscopicPoints(glm::vec3 centerOfProj, glm::vec3 viewPos, std::vector<glm::vec3> obj)
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

std::vector<glm::vec3> UntrackedStereoDiagram::transformStereoscopicPoints(glm::vec3 centerOfProjL, glm::vec3 centerOfProjR, glm::vec3 viewPosL, glm::vec3 viewPosR, std::vector<glm::vec3> obj)
{
	std::vector<glm::vec3> iL(getScreenIntersections(centerOfProjL, obj));
	std::vector<glm::vec3> iR(getScreenIntersections(centerOfProjR, obj));

	std::vector<glm::vec3> ret;
	for (int i = 0; i < obj.size(); ++i)
	{
		glm::vec3 pa, pb;
		double mua, mub;
		LineLineIntersect(viewPosL, iL[i], viewPosR, iR[i], &pa, &pb, &mua, &mub);
		ret.push_back((pa + pb) / 2.f);
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

/*
from http://paulbourke.net/geometry/pointlineplane/lineline.c

Calculate the line segment PaPb that is the shortest route between
two lines P1P2 and P3P4. Calculate also the values of mua and mub where
Pa = P1 + mua (P2 - P1)
Pb = P3 + mub (P4 - P3)
Return false if no solution exists.
*/
bool UntrackedStereoDiagram::LineLineIntersect(	glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, glm::vec3 *pa, glm::vec3 *pb, double *mua, double *mub)
{
	glm::vec3 p13, p43, p21;
	double d1343, d4321, d1321, d4343, d2121;
	double numer, denom;

	p13.x = p1.x - p3.x;
	p13.y = p1.y - p3.y;
	p13.z = p1.z - p3.z;
	p43.x = p4.x - p3.x;
	p43.y = p4.y - p3.y;
	p43.z = p4.z - p3.z;
	if (glm::abs(p43.x) < glm::epsilon<float>() && glm::abs(p43.y) < glm::epsilon<float>() && glm::abs(p43.z) < glm::epsilon<float>())
		return false;
	p21.x = p2.x - p1.x;
	p21.y = p2.y - p1.y;
	p21.z = p2.z - p1.z;
	if (glm::abs(p21.x) < glm::epsilon<float>() && glm::abs(p21.y) < glm::epsilon<float>() && glm::abs(p21.z) < glm::epsilon<float>())
		return false;

	d1343 = p13.x * p43.x + p13.y * p43.y + p13.z * p43.z;
	d4321 = p43.x * p21.x + p43.y * p21.y + p43.z * p21.z;
	d1321 = p13.x * p21.x + p13.y * p21.y + p13.z * p21.z;
	d4343 = p43.x * p43.x + p43.y * p43.y + p43.z * p43.z;
	d2121 = p21.x * p21.x + p21.y * p21.y + p21.z * p21.z;

	denom = d2121 * d4343 - d4321 * d4321;
	if (glm::abs(denom) < glm::epsilon<float>())
		return false;
	numer = d1343 * d4321 - d1321 * d4343;

	*mua = numer / denom;
	*mub = (d1343 + d4321 * (*mua)) / d4343;

	pa->x = p1.x + *mua * p21.x;
	pa->y = p1.y + *mua * p21.y;
	pa->z = p1.z + *mua * p21.z;
	pb->x = p3.x + *mub * p43.x;
	pb->y = p3.y + *mub * p43.y;
	pb->z = p3.z + *mub * p43.z;

	return true;
}

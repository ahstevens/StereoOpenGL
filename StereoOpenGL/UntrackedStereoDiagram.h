#pragma once

#include <glm.hpp>
#include <vector>

class UntrackedStereoDiagram
{
public:	
	UntrackedStereoDiagram(glm::mat4 screenBasis, glm::ivec2 screenResolution);
	~UntrackedStereoDiagram();

	void draw();

	float getViewAngle();
	void setViewAngle(float angle);

private:
	void drawOBJ(std::vector<glm::vec3> obj, glm::vec4 col);
	void drawEye(glm::vec3 eyePos, glm::vec4 eyeCol, std::vector<glm::vec3> obj, glm::vec4 objCol, glm::vec4 rayCol);

	std::vector<glm::vec3> getHinge();

	void drawHingeAngle(std::vector<glm::vec3> pts, glm::vec4 col);

	std::vector<glm::vec3> transformMonoscopicPoints(glm::vec3 centerOfProj, std::vector<glm::vec3> obj, glm::vec3 viewPos);

	std::vector<glm::vec3> getScreenIntersections(glm::vec3 centerOfProjection, std::vector<glm::vec3> pts);
private:	
	glm::mat4 m_mat4ScreenBasis; // non-normalized
	glm::mat4 m_mat4ScreenBasisOrtho; // orthonormal
	glm::ivec2 m_ivec2ScreenRes;

	float m_fOriginScreenDist;

	float m_fHingeLength;
	float m_fHingeAngle;

	glm::vec3 m_vec3ObjOrigin;

	float m_fProjectionAngle; // Range (-90, 90); 0 degrees is orthogonal to screen plane
	float m_fViewingAngle; // Range (-90, 90); 0 degrees is orthogonal to screen plane
	float m_fViewingArcRadius;
	float m_fEyeSeparation;
};


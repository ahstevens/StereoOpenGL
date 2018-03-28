#pragma once

#include <glm.hpp>
#include <lodepng.h>
#include <vector>

class Hinge
{
public:	
	Hinge(float length, float angle);
	~Hinge();
	
	glm::vec3 getPos();
	void setPos(glm::vec3 pos);

	float getAngle();
	void setAngle(float angle);

	float getLength();
	void setLength(float length);

	void draw();
	void drawShadow();

private:

	glm::vec3 m_vec3Pos;

	float m_fLength;
	float m_fAngle;
};


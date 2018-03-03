#pragma once

#include <glm.hpp>
#include <vector>

class Clapboard
{
public:	
	Clapboard(float length, float angle);
	~Clapboard();

	float getAngle();
	void setAngle(float angle);

	float getLength();
	void setLength(float length);

	void draw();

private:
	glm::vec3 m_vec3Pos;

	float m_fLength;
	float m_fAngle;
};


#pragma once

#include <glm.hpp>


class UntrackedStereoDiagram
{
public:	
	UntrackedStereoDiagram(glm::mat4 screenTransform, glm::ivec2 screenResolution);
	~UntrackedStereoDiagram();

	void draw();

private:	
	glm::mat4 m_mat4ScreenTransform; // non-normalized
	glm::ivec2 m_ivec2ScreenRes;
};


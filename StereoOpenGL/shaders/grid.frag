layout(location = DIFFUSE_COLOR_UNIFORM_LOCATION)
	uniform vec4 diffColor;
layout(location = SPECULAR_COLOR_UNIFORM_LOCATION)
	uniform vec4 specColor;

layout(std140, binding = SCENE_UNIFORM_BUFFER_LOCATION) 
	uniform FrameUniforms
	{
		vec4 v4Viewport;
		mat4 m4View;
		mat4 m4Projection;
		mat4 m4ViewProjection;
	};

in vec4 v4Color;
in vec3 v3FragPos;
in vec2 v2TexCoords;
out vec4 outputColor;

void main()
{
	if (v4Color.a * diffColor.a == 0.f)
		discard;
	
	float gridLineWidth = 0.01f;
	float falloff = 0.1;

	float remainderX = mod(v2TexCoords.x, 1.f);
	float remainderY = mod(v2TexCoords.y, 1.f);
	
	float blend = smoothstep(gridLineWidth, gridLineWidth + falloff, remainderX) * (1 - smoothstep(1.f - gridLineWidth - falloff, 1.f - gridLineWidth, remainderX)) *
        smoothstep(gridLineWidth, gridLineWidth + falloff, remainderY) * (1 - smoothstep(1.f - gridLineWidth - falloff, 1.f - gridLineWidth, remainderY));
		
	outputColor = blend * v4Color * diffColor;	
}
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

float pseudorand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
	if (v4Color.a * diffColor.a == 0.f)
		discard;
	
	vec2 gridCells = vec2(5, 5);
	float gridLineWidth = 0.05f;
	float falloff = 0.01f;

	float remainderX = mod(v2TexCoords.x * gridCells.x, 1.f);
	float remainderY = mod(v2TexCoords.y * gridCells.y, 1.f);
	
	float blend = smoothstep(gridLineWidth, gridLineWidth + falloff, remainderX) * (1 - smoothstep(1.f - gridLineWidth - falloff, 1.f - gridLineWidth, remainderX)) *
        smoothstep(gridLineWidth, gridLineWidth + falloff, remainderY) * (1 - smoothstep(1.f - gridLineWidth - falloff, 1.f - gridLineWidth, remainderY));
		
	blend *= pseudorand(v2TexCoords);

	outputColor = v4Color * diffColor;
	outputColor.xyz = mix(vec3(0.1f), outputColor.xyz, blend);
}
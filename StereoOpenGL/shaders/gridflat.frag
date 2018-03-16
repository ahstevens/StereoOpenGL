layout(binding = DIFFUSE_TEXTURE_BINDING)
	uniform sampler2D diffuseTex;
layout(location = DIFFUSE_COLOR_UNIFORM_LOCATION)
	uniform vec4 diffColor;
layout(location = SPECULAR_COLOR_UNIFORM_LOCATION)
	uniform vec4 gridColor;

in vec3 v3Normal;
in vec3 v3FragPos;
in vec2 v2TexCoords;

out vec4 color;

void main()
{
	// GRID CODE
	vec2 gridCells = vec2(10, 10);
	float gridLineWidth = 0.02f;
	float falloff = 0.01f;

	float remainderX = fract(v2TexCoords.x * gridCells.x);
	float remainderY = fract(v2TexCoords.y * gridCells.y);
	
	float blend = smoothstep(gridLineWidth, gridLineWidth + falloff, remainderX) * (1.f - smoothstep(1.f - gridLineWidth - falloff, 1.f - gridLineWidth, remainderX)) *
        smoothstep(gridLineWidth, gridLineWidth + falloff, remainderY) * (1.f - smoothstep(1.f - gridLineWidth - falloff, 1.f - gridLineWidth, remainderY));

	vec4 surfaceDiffColor = texture(diffuseTex, v2TexCoords) * diffColor;
	surfaceDiffColor = mix(gridColor, surfaceDiffColor, blend);

	if (surfaceDiffColor.a == 0.f)
	    discard;

    color = surfaceDiffColor;
}
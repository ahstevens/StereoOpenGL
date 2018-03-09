layout(location = DIFFUSE_COLOR_UNIFORM_LOCATION)
	uniform vec4 diffColor;

out vec4 color;

void main()
{
	color = diffColor;
}
layout(location = POSITION_ATTRIB_LOCATION)
	in vec3 v3Position;

layout(location = LIGHT_COUNT_UNIFORM_LOCATION)
	uniform int numLights;
	 
struct Light {
    vec4 position;
    vec4 direction;
    vec4 color;
	float ambientCoeff;
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;
	float isOn;
	float isSpotLight;
};

uniform Light lights[MAX_LIGHTS];

layout(location = MODEL_MAT_UNIFORM_LOCATION)
	uniform mat4 m4Model;
	
layout(std140, binding = SCENE_UNIFORM_BUFFER_LOCATION) 
	uniform FrameUniforms
	{
		vec4 v4Viewport;
		mat4 m4View;
		mat4 m4Projection;
		mat4 m4ViewProjection;
	};

// Adapted from OpenGL Red Book Ch. 14, pg. 583-584
mat4 makeShadowMatrix(vec4 plane, vec4 L)
{
    float  dist;
    mat4  shadowMat;

	// plane is given as a normal vector (to the plane), so distance is simply
	// the dot product of the plane normal with the light vector
	// distance = ( light.x * plane.normal.x ) + 
	//            ( light.y * plane.normal.y ) + 
	//            ( light.z * plane.normal.z ) + 
	//            ( light.w * plane.distance )
    dist = dot( L, plane );

	float plane_normal_x = plane[ 0 ];
	float plane_normal_y = plane[ 1 ];
	float plane_normal_z = plane[ 2 ];
	float plane_distance = plane[ 3 ];
    
	// The following is a cleaned-up version of the matrix presented on pg. 584
	// of the OpenGL Red Book, 3rd Ed.

	// Column 1
    shadowMat[ 0 ].x = dist - L.x * plane_normal_x;
    shadowMat[ 0 ].y =      - L.y * plane_normal_x;
    shadowMat[ 0 ].z =      - L.z * plane_normal_x;
    shadowMat[ 0 ].w =      - L.w * plane_normal_x;

	// Column 2
    shadowMat[ 1 ].x =      - L.x * plane_normal_y;
    shadowMat[ 1 ].y = dist - L.y * plane_normal_y;
    shadowMat[ 1 ].z =      - L.z * plane_normal_y;
    shadowMat[ 1 ].w =      - L.w * plane_normal_y;

	// Column 3
    shadowMat[ 2 ].x =      - L.x * plane_normal_z;
    shadowMat[ 2 ].y =      - L.y * plane_normal_z;
    shadowMat[ 2 ].z = dist - L.z * plane_normal_z;
    shadowMat[ 2 ].w =      - L.w * plane_normal_z;

	// Column 4
    shadowMat[ 3 ].x =      - L.x * plane_distance;    
    shadowMat[ 3 ].y =      - L.y * plane_distance;    
    shadowMat[ 3 ].z =      - L.z * plane_distance;    
    shadowMat[ 3 ].w = dist - L.w * plane_distance;

    return shadowMat;
}

void main()
{
	vec4 plane;
	plane.xyz = normalize(vec3(0.f, 1.f, 0.f));
	plane.w = ((length(m4Model[0]) / 2.f) - m4Model[3].y) * 0.999f;
	mat4 shadowMat = makeShadowMatrix(plane, normalize(-lights[0].direction));
	gl_Position = m4ViewProjection * shadowMat * m4Model * vec4(v3Position, 1.0);
}
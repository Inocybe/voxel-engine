#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vUV;


void main()
{
	 // World position of the vertex
    vec4 worldPos = model * vec4(aPos, 1.0);
    vWorldPos = worldPos.xyz;

    // Transform normal to world space
    vNormal = aNormal;

    // Pass UV
    vUV = aUV;


	gl_Position = projection * view * model * vec4(aPos, 1.0f);
}
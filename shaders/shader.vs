#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in int aNormal;
layout (location = 2) in vec2 aUV;

uniform vec3 chunkPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vUV;


const vec3 normals[6] = vec3[](
    vec3( 1, 0, 0), vec3(-1, 0, 0),
    vec3( 0, 1, 0), vec3( 0,-1, 0),
    vec3( 0, 0, 1), vec3( 0, 0,-1)
);


void main()
{
	 // World position of the vertex
    vec4 worldPos = model * vec4(aPos + chunkPos, 1.0);
    vWorldPos = worldPos.xyz;

    // Transform normal to world space
    vNormal = normals[aNormal];

    // Pass UV
    vUV = aUV;


	gl_Position = projection * view * model * vec4(aPos, 1.0f);
}
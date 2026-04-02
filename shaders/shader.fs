#version 450 core

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;

out vec4 FragColor;


void main()
{
	FragColor = vec4(0.2 * vNormal.x, 0.9 * vNormal.y, 0.2*vNormal.z, 1.0);  
}
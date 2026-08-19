#version 330 core

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;

out vec4 FragColor;

uniform vec3 viewPos;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 baseColor;
uniform vec3 fogColor;
uniform float fogDensity;

void main()
{
	vec3 n = normalize(vNormal);
	vec3 l = normalize(-lightDir);

	float ambientStrength = 0.35;
	vec3 ambient = ambientStrength * lightColor;

	float diff = max(dot(n, l), 0.0);
	vec3 diffuse = diff * lightColor;

	float faceShade = 1.0;
	if (n.z > 0.5) faceShade = 1.0;
	//else if (n.z > 0.0) faceShade = 0.8;
	else if (n.z < -0.5) faceShade = 0.5;
	else faceShade = 0.8;

	vec3 color = baseColor * (ambient + diffuse) * faceShade;

	float dist = length(viewPos - vWorldPos);
	float fogFactor = clamp(1.0 - exp(-pow(dist * fogDensity, 2.0)), 0.0, 1.0);
	color = mix(color, fogColor, fogFactor);


	FragColor = vec4(color, 1.0);  
}
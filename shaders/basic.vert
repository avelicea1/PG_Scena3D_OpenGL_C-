#version 410 core

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vTexCoords;

out vec3 fPosition;
out vec3 fNormal;
out vec2 fTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightPos[13];
uniform mat4 lightSpaceTrMatrix;
uniform int pornesteLuminaDir;
uniform int pornesteLuminaPos;

out vec4 fPosEye;
out vec4 lightPosEye[13];
out vec4 fragPosLightSpace;
uniform int fog;

void main() 
{
	gl_Position = projection * view * model * vec4(vPosition, 1.0f);
	fPosition = vPosition;
	fNormal = vNormal;
	fTexCoords = vTexCoords;
	fragPosLightSpace = lightSpaceTrMatrix * model * vec4(vPosition, 1.0f);

	
	fPosEye = view * model * vec4(fPosition, 1.0f);
	for(int i=0;i<13;i++){
    		lightPosEye[i] = view * model * vec4(lightPos[i], 1.0f);
	}
}

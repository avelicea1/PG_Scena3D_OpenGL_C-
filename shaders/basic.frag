#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;


out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;

//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;

// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

//components
vec3 ambient;
vec3 ambientPos;
float ambientStrength = 0.2f;
float ambientStrengthPos = 0.0002f;
vec3 diffuse;
vec3 diffusePos;
vec3 specular;
vec3 specularPos;
float specularStrength = 0.05f;
float specularStrengthPos = 0.5f;

float constant = 1.0f;
float linear = 0.100045f;
float quadratic = 0.100075f;
float shininess = 32.0f;

//lightPos
uniform vec3 lightPos[13];
uniform int pornesteLuminaDir;
uniform int pornesteLuminaPos;
vec3 lightColorPunct = vec3(0.933f,0.933f,0.988f);

in vec4 fPosEye;
in vec4 lightPosEye[13];

//shadow
in vec4 fragPosLightSpace;
uniform sampler2D shadowMap;

//fog
uniform float fogDensity;

float computeShadow()
{

	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	normalizedCoords = normalizedCoords * 0.5 + 0.5;
	if (normalizedCoords.z > 1.0f)
		return 0.0f;
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
	float currentDepth = normalizedCoords.z;
	float bias = max(0.05f * (1.0f - dot(fNormal, lightDir)), 0.001f);
	float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;
	return shadow;
}
float computeFog()
{
	float fragmentDistance = length(fPosEye);
	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
	return clamp(fogFactor, 0.0f, 1.0f);
}

vec3 computeDirLight()
{
    //compute eye space coordinates
    //vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(-fPosEye.xyz);

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;
   
    float shadow = computeShadow();

    return min((ambient + (1.0f - shadow)*diffuse) * texture(diffuseTexture, fTexCoords).rgb + (1.0f - shadow)*specular * texture(specularTexture, fTexCoords).rgb, 1.0f);
		
}
vec3 computePointLight(int index)
{
	
	float dist = length(lightPosEye[index].xyz - vec3(fPosEye));
    float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));
	
	//pozitia camerei
	vec3 cameraPosEye = vec3(0.0f);
	
	vec3 normalEye = normalize(fNormal);
	
	vec3 lightDirN = normalize(lightPosEye[index].xyz - vec3(fPosEye.x,fPosEye.y,fPosEye.z));
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
	
	
	//compute half vector
	vec3 halfVector = normalize(lightDirN + viewDirN);
	
	ambientPos += att * ambientStrengthPos * lightColorPunct;
	
	diffusePos += att * max(dot(normalEye, lightDirN), 0.0f) * lightColorPunct;
	
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
	specularPos += att * specularStrengthPos * specCoeff * lightColorPunct;	

	return min((ambientPos + diffusePos) * texture(diffuseTexture, fTexCoords).rgb + specularPos * texture(specularTexture, fTexCoords).rgb, 1.0f);

}

void main() 
{
    vec3 color;
    if(pornesteLuminaDir == 1){
	color += computeDirLight();
    }else{
	ambient = vec3(0.05, 0.05, 0.05);
        color = ambient * texture(diffuseTexture, fTexCoords).rgb;
    }
    if(pornesteLuminaPos == 1){
    	for (int i=0;i<13;i++){
    	   color += computePointLight(i);
	}
    }
    float fogFactor = computeFog();
    vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    fColor = mix(fogColor, vec4(color, 1.0f), fogFactor);
}

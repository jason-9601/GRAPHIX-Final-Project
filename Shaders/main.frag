#version 330 core
out vec4 FragColor;

in vec2 texCoord;
in vec3 normCoord;
in vec3 fragPos;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float constant; // light constant
uniform float linear; // light linear
uniform float quadratic; // light quadratic

uniform float ambientStr;
uniform vec3 ambientColor;

uniform vec3 cameraPos;
uniform float specStr;
uniform float specPhong;

// dir light
uniform vec3 direction;
uniform vec3 dirlightColor;

uniform float dirambientStr;
uniform vec3 dirambientColor;

uniform float dirspecStr;
uniform float dirspecPhong;

uniform vec3 ourColor;

uniform sampler2D tex0;

vec3 CalcDirLight(vec3 normal, vec3 viewDir);
vec3 CalcPointLight(vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	vec3 normal = normalize(normCoord);
    vec3 viewDir = normalize(cameraPos - fragPos);

    vec3 result = CalcDirLight(normal, viewDir);
    result += CalcPointLight(normal, fragPos, viewDir);
	
	FragColor = vec4(result, 1.0f) * texture(tex0, texCoord);
}

vec3 CalcDirLight(vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(direction - fragPos);

    float diff = max(dot(normal, lightDir), 0.0f);

    vec3 diffuse = diff * dirlightColor;

    vec3 ambientCol = dirambientStr * dirambientColor;

    vec3 reflectDir = reflect(-lightDir, normal);

    float spec = pow(max(dot(reflectDir, viewDir), 0.1f), dirspecPhong);

    vec3 specCol = spec * dirspecStr * dirlightColor;

    return (specCol + diffuse + ambientCol);
}

vec3 CalcPointLight(vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(lightPos - fragPos);

    float diff = max(dot(normal, lightDir), 0.0f);

    vec3 diffuse = diff * lightColor;

    vec3 ambientCol = ambientStr * ambientColor;

    vec3 reflectDir = reflect(-lightDir, normal);

    float spec = pow(max(dot(reflectDir, viewDir), 0.1f), specPhong);

    vec3 specCol = spec * specStr * lightColor;

    // get distance and point light attenuation.
    float distance = length(lightPos - fragPos);
    // modified version of intensity (1 / (distance^2)). includes light constant, linear and quadratic.
    float attenuation = 1.0f / (constant + linear * distance + quadratic * (distance * distance)); 

    // combining results
    ambientCol *= attenuation;
    diffuse *= attenuation;
    specCol *= attenuation;

    return (specCol + diffuse + ambientCol) * ourColor;
}
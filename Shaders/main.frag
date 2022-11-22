#version 330 core
out vec4 FragColor;

in vec2 texCoord;
in vec3 normCoord;
in vec3 fragPos;

uniform vec3 sampleColor;

void main()
{
	FragColor = vec4(sampleColor, 1.0f);
}

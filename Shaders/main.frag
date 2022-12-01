#version 330 core
out vec4 FragColor;

in vec2 texCoord;
in vec3 normCoord;
in vec3 fragPos;

uniform sampler2D tex0;

void main()
{
	FragColor = vec4(0.5f, 0.5f, 0.5f, 1.0f) * texture(tex0, texCoord);
}

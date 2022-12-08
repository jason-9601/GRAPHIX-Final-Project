#version 330 core

out vec4 FragColor;

in vec3 texCoord;

uniform samplerCube skybox;
uniform bool firstPerson;

void main() {

	FragColor = texture(skybox, texCoord);
	if (firstPerson){
        FragColor.r = 0.0f;
        FragColor.b = 0.0f;
    }

}
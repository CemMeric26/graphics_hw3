#version 330 core


// Matrices for transformations
uniform mat4 modelingMatrix;
uniform mat4 viewingMatrix;
uniform mat4 projectionMatrix;
uniform vec3 eyePos;

layout(location=0) in vec3 inVertex;

out vec4 fragWorldPos;

void main()
{
    // Transform vertex position into clip space
	fragWorldPos = modelingMatrix * vec4(inVertex, 1);
    gl_Position = projectionMatrix * viewingMatrix * modelingMatrix * vec4(inVertex, 1.0);
}

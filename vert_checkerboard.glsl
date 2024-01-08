#version 330 core

layout(location = 0) in vec3 aPos; // Vertex position

// Matrices for transformations
uniform mat4 modelingMatrix;
uniform mat4 viewingMatrix;
uniform mat4 projectionMatrix;
uniform vec3 eyePos;

void main()
{
    // Transform vertex position into clip space
    gl_Position = projectionMatrix * viewingMatrix * modelingMatrix * vec4(aPos, 1.0);
}

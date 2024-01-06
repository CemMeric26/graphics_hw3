#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

void main()
{
    // Define the size of each checker tile
    float size = 1.0; // This can be adjusted to scale the checkerboard pattern

    // Calculate checkerboard pattern
    vec2 checker = floor(TexCoord / size);
    float checkerPattern = mod(checker.x + checker.y, 2.0);

    // Set color to black or white based on the pattern
    vec3 color = checkerPattern < 1.0 ? vec3(0.0) : vec3(1.0);

    // FragColor = vec4(color, 1.0);
    FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Red color, fully opaque
}

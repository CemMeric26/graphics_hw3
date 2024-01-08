#version 330 core

out vec4 FragColor;
in vec4 fragWorldPos;

uniform float offset; // offset for the checkerboard
uniform float scale; // scale for the checkerboard

void main()
{

    vec4 pos = fragWorldPos;

    bool x = (int ((pos.x + offset) * scale)) %2 != 0;
    bool y = (int ((pos.y + offset ) * scale)) % 2 != 0;
    bool z = (int ((pos.z + offset) * scale)) % 2 != 0;
    
    bool xorXY = x != y;
    
    if (xorXY != z)
        FragColor = vec4(0.0, 0.0, 0.15, 1.0); // black for one condition
    else
        FragColor = vec4(0.0, 0.0, 0.85, 1.0); // white for the other condition
    
    // FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}

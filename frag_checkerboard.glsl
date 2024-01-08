#version 330 core

out vec4 FragColor;

uniform float offset; // offset for the checkerboard
uniform float scale; // scale for the checkerboard

void main()
{

    vec3 pos = gl_FragCoord.xyz / scale;
    pos.x += offset; // Adding offset to x component as an example

    bool x = (int(pos.x) % 2) != 0;
    bool y = (int(pos.y) % 2) != 0;
    bool z = (int(pos.z) % 2) != 0;
    
    bool xorXY = x != y;
    
    if (xorXY != z)
        FragColor = vec4(0.0, 0.0, 0.0, 1.0); // black for one condition
    else
        FragColor = vec4(1.0, 1.0, 1.0, 1.0); // white for the other condition
    
    // FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}

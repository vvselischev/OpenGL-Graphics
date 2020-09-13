#version 330 core

uniform vec2 z0;
uniform sampler1D palette;
uniform int maxIterations;
uniform float depthAffect;
uniform mat4 transform;

in vec4 position;


void main()
{
    vec2 c = (transform * position).xy;
    
    vec2 z = z0;
    int i;
    for (i = 0; i < maxIterations; i++)
    {
        float nextX = z.x * z.x - z.y * z.y + c.x;
        float nextY = 2 * z.x * z.y + c.y;

        z.x = nextX;
        z.y = nextY;
        
        if (z.x * z.x + z.y * z.y > 4.0) 
        {
            break;
        }
    }

    float resultColorLerp = 0;
    if (i != maxIterations) 
    {
        resultColorLerp = float(i) / (depthAffect * maxIterations);
    }
    
    gl_FragColor = texture1D(palette, resultColorLerp);
}
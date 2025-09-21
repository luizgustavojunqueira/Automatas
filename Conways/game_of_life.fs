#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;

uniform sampler2D texture0;
uniform vec2 resolution;
uniform float time;

void main(){
    vec2 texelSize = vec2(1.0) / resolution;
    vec2 pos = fragTexCoord;

    float current = texture(texture0, pos).r;

    float neighbors = 
        texture(texture0, pos + vec2(-texelSize.x, -texelSize.y)).r + // top-left
        texture(texture0, pos + vec2(0.0, -texelSize.y)).r +         // top
        texture(texture0, pos + vec2(texelSize.x, -texelSize.y)).r +  // top-right
        texture(texture0, pos + vec2(-texelSize.x, 0.0)).r +         // left
        texture(texture0, pos + vec2(texelSize.x, 0.0)).r +          // right
        texture(texture0, pos + vec2(-texelSize.x, texelSize.y)).r +  // bottom-left
        texture(texture0, pos + vec2(0.0, texelSize.y)).r +          // bottom
        texture(texture0, pos + vec2(texelSize.x, texelSize.y)).r;    // bottom-right
    
    int iCurrent = int(current + 0.5);
    int iNeighbors = int(neighbors + 0.5);


    float alive = float(iCurrent);
    float survives = step(1.5, neighbors) * step(neighbors, 3.5) * alive;
    float born = step(2.5, neighbors) * step(neighbors, 3.5) * (1.0 - alive);

    float nextState = survives + born;

   
    nextState *= step(0.5, neighbors + current);

    finalColor = vec4(nextState, nextState, nextState, 1.0);
}

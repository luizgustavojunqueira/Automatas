#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;
uniform sampler2D texture0;
uniform vec2 resolution;
uniform float time;

void main(){
    vec2 texCoord = fragTexCoord;
    
    float cellSize = 1.0;
    vec2 cellCoord = floor(texCoord * resolution / cellSize) * cellSize / resolution;
    
    float currentCell = texture(texture0, cellCoord + vec2(cellSize/2.0) / resolution).r;
    
    int liveNeighbors = 0;
    
    for(int dx = -1; dx <= 1; dx++) {
        for(int dy = -1; dy <= 1; dy++) {
            if(dx == 0 && dy == 0) continue;
            
            vec2 neighborCoord = cellCoord + vec2(dx, dy) * cellSize / resolution;
            
            if(neighborCoord.x >= 0.0 && neighborCoord.x <= 1.0 && 
               neighborCoord.y >= 0.0 && neighborCoord.y <= 1.0) {
                
                float neighborCell = texture(texture0, neighborCoord + vec2(cellSize/2.0) / resolution).r;
                if(neighborCell > 0.5) {
                    liveNeighbors++;
                }
            }
        }
    }
    
    vec2 cellCenter = cellCoord + vec2(cellSize/2.0) / resolution;
    
    float newState = 0.0;
    if(currentCell > 0.5) {
        if(liveNeighbors == 2 || liveNeighbors == 3) {
            newState = 1.0;
        }
    } else {
        if(liveNeighbors == 3) {
            newState = 1.0;
        }
    }
    
    finalColor = vec4(newState, newState, newState, 1.0);
}

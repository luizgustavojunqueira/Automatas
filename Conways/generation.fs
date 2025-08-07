#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;

uniform vec2 resolution;
uniform float seed;
uniform float density;
uniform int pattern;
uniform float time;

// Função de hash para geração de números pseudo-aleatórios
float hash(vec2 p) {
    p = 50.0 * fract(p * 0.3183099 + vec2(0.71, 0.113));
    return -1.0 + 2.0 * fract(p.x * p.y * (p.x + p.y));
}

// Noise suave
float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    
    return mix(mix(hash(i + vec2(0.0, 0.0)), 
                   hash(i + vec2(1.0, 0.0)), u.x),
               mix(hash(i + vec2(0.0, 1.0)), 
                   hash(i + vec2(1.0, 1.0)), u.x), u.y);
}

// FBM (Fractional Brownian Motion) para ruído mais complexo
float fbm(vec2 p) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 0.0;
    
    for (int i = 0; i < 4; i++) {
        value += amplitude * noise(p);
        p *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

// Função de hash melhorada para coordenadas
float hash21(vec2 p) {
    p = fract(p * vec2(234.34, 435.345));
    p += dot(p, p + 34.23);
    return fract(p.x * p.y + seed);
}

// Função para gerar valores baseados em posição e seed
float random(vec2 coord, float seedOffset) {
    return hash21(coord + seedOffset + seed);
}

void main() {
    vec2 uv = fragTexCoord;
    vec2 pixel = uv * resolution;
    
    float result = 0.0;
    
    if (pattern == 0) {
        // Padrão 1: Random simples
        float r = random(pixel, 0.0);
        result = (r < density) ? 1.0 : 0.0;
        
    } else if (pattern == 1) {
        // Padrão 2: Patterns em blocos
        vec2 blockCoord = floor(pixel / 5.0);
        float blockRandom = random(blockCoord, 1.0);
        
        if (blockRandom < density) {
            vec2 localCoord = mod(pixel, 5.0);
            float localRandom = random(pixel, 2.0);
            result = (localRandom < 0.6) ? 1.0 : 0.0;
        }
        
    } else if (pattern == 2) {
        // Padrão 3: Noise baseado em Perlin
        vec2 noiseCoord = pixel * 0.01 + seed;
        float noiseValue = fbm(noiseCoord) * 0.5 + 0.5;
        
        // Adiciona variação local
        float localNoise = random(pixel, 3.0);
        float combinedNoise = mix(noiseValue, localNoise, 0.3);
        
        result = (combinedNoise < density) ? 1.0 : 0.0;
        
    } else if (pattern == 3) {
        // Padrão 4: Hotspots com densidade variável
        float finalDensity = density * 0.3; // Densidade base baixa
        
        // Cria vários hotspots
        for (int i = 0; i < 8; i++) {
            vec2 hotspotCenter = vec2(
                hash21(vec2(float(i) * 123.456, seed)) * resolution.x,
                hash21(vec2(float(i) * 789.123, seed + 1.0)) * resolution.y
            );
            
            float distance = length(pixel - hotspotCenter);
            float hotspotRadius = 30.0 + hash21(vec2(float(i), seed + 2.0)) * 50.0;
            
            if (distance < hotspotRadius) {
                float influence = 1.0 - (distance / hotspotRadius);
                influence = influence * influence; // Curva suave
                finalDensity += density * 1.5 * influence;
            }
        }
        
        // Aplica densidade final
        float r = random(pixel, 4.0);
        result = (r < finalDensity) ? 1.0 : 0.0;
    }
    
    finalColor = vec4(result, result, result, 1.0);
}

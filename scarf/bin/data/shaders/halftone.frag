#version 120

varying vec2 vTexCoord;

uniform vec3 topColor;
uniform vec3 bottomColor;
uniform float noiseScale;
uniform float randomSeed;

// Simple random function - returns value between 0 and 1
float random(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

// Random with seed offset
float randomWithSeed(vec2 p, float seed) {
    return fract(sin(dot(p + seed, vec2(12.9898, 78.233))) * 43758.5453);
}

// Random offset in range [-1, 1]
vec2 randomOffset(vec2 p, float seed) {
    float x = randomWithSeed(p, seed) * 2.0 - 1.0;
    float y = randomWithSeed(p + vec2(127.1, 311.7), seed) * 2.0 - 1.0;
    return vec2(x, y);
}

void main() {
    vec2 uv = vTexCoord;
    
    // Calculate gradient factor (0 = top, 1 = bottom)
    float gradientFactor = uv.y;
    
    // Create grid coordinates
    float pixelSize = 1.0 / noiseScale;
    vec2 currentGrid = floor(uv * noiseScale);
    
    // Initialize with background color
    vec3 backgroundColor = mix(topColor, bottomColor, gradientFactor);
    vec3 finalColor = backgroundColor;
    float highestCenterY = 2.0; // Start with value higher than any possible y coordinate
    bool foundCircle = false;
    
    // Check a 3x3 grid around current position to find overlapping circles
    for(int dx = -1; dx <= 1; dx++) {
        for(int dy = -1; dy <= 1; dy++) {
            vec2 gridCoord = currentGrid + vec2(float(dx), float(dy));
            
            // Get random offset for this grid cell (larger offset range)
            vec2 offset = randomOffset(gridCoord, randomSeed) * pixelSize * 0.8;
            
            // Calculate center of this grid cell with offset
            vec2 cellCenter = (gridCoord + 0.5) / noiseScale + offset;
            
            // Distance from current pixel to this cell center
            float distToCenter = distance(uv, cellCenter);
            
            // Create circular pixel
            float circleRadius = pixelSize * 0.3;
            float circleAlpha = 1.0 - smoothstep(circleRadius * 0.7, circleRadius, distToCenter);
            
            // If this circle affects this pixel AND is higher up than previous circles
            if(circleAlpha > 0.0 && cellCenter.y < highestCenterY) {
                // Get random value for this grid cell for dithering decision
                float randomValue = randomWithSeed(gridCoord, randomSeed);
                
                // Dithering: if random value is less than gradient factor, use bottom color
                vec3 ditherColor;
                if (randomValue < gradientFactor) {
                    ditherColor = bottomColor;
                } else {
                    ditherColor = topColor;
                }
                
                // Update final color based on topmost circle
                finalColor = mix(backgroundColor, ditherColor, circleAlpha);
                highestCenterY = cellCenter.y;
                foundCircle = true;
            }
        }
    }
    
    gl_FragColor = vec4(finalColor, 1.0);
}

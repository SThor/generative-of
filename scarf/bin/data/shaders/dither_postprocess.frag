#version 120

varying vec2 vTexCoord;

uniform sampler2D gradientTexture;
uniform vec3 topColor;
uniform vec3 bottomColor;
uniform float dotRadius;
uniform float grainSize;
uniform float randomSeed;
uniform int samplesPerPixel;
uniform vec2 resolution;

// Simple hash function
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    vec2 uv = vTexCoord;
    
    // Sample the color gradient and convert to gradient factor
    vec3 currentColor = texture2D(gradientTexture, uv).rgb;
    
    // Calculate gradient factor using UV position as fallback if color detection fails
    float gradientFactor = uv.y; // Simple fallback: 0 at top, 1 at bottom
    
    // Try to use color-based detection for better accuracy
    float distToTop = distance(currentColor, topColor);
    float distToBottom = distance(currentColor, bottomColor);
    float totalDist = distToTop + distToBottom;
    
    if(totalDist > 0.01) { // Only use color detection if colors are different enough
        gradientFactor = distToBottom / totalDist;
    }
    
    gradientFactor = clamp(gradientFactor, 0.0, 1.0);
    
    // Grid position for dithering
    vec2 pixelPos = uv * resolution;
    vec2 gridPos = floor(pixelPos / grainSize);
    
    // Position within the cell (0 to 1)
    vec2 cellPos = mod(pixelPos, grainSize) / grainSize;
    
    // Random offset for each cell to break up regularity  
    vec2 randomOffset = vec2(
        hash(gridPos + randomSeed) - 0.5,
        hash(gridPos + randomSeed + 13.7) - 0.5
    ) * 0.3; // Smaller random offset
    
    // Cell center with random offset
    vec2 cellCenter = vec2(0.5, 0.5) + randomOffset;
    float distFromCenter = distance(cellPos, cellCenter);
    
    // Convert dot radius to cell-relative size - allow larger dots but handle overflow
    float maxDotSize = dotRadius * 0.01; // Don't cap the size artificially
    
    bool isDot = false;
    
    if(samplesPerPixel >= 5) {
        // High quality: Random threshold with strong gradient influence
        float randomThreshold = hash(gridPos + randomSeed + 27.3);
        
        // Use gradient factor to influence probability - this creates the density variation
        isDot = randomThreshold < gradientFactor;
        
        // For high quality, dots can be any size but still circular
        if(isDot && distFromCenter > maxDotSize) {
            isDot = false; // Keep dots circular
        }
    } else {
        // Low quality: Fixed dots with gradient-based sizing that varies across image
        float currentDotSize = maxDotSize * sqrt(gradientFactor);
        isDot = distFromCenter < currentDotSize;
    }
    
    // Handle large dots that exceed cell boundaries
    // Instead of clipping, we check neighboring cells for continuity
    if(maxDotSize > 0.5) {
        // For very large dots, sample multiple neighboring cells
        vec2 neighborGridPos = gridPos;
        
        // Check if we should extend this dot from a neighboring cell
        for(int dx = -1; dx <= 1; dx++) {
            for(int dy = -1; dy <= 1; dy++) {
                if(dx == 0 && dy == 0) continue; // Skip current cell
                
                vec2 neighborGrid = gridPos + vec2(float(dx), float(dy));
                vec2 neighborCenter = vec2(0.5, 0.5) + vec2(
                    hash(neighborGrid + randomSeed) - 0.5,
                    hash(neighborGrid + randomSeed + 13.7) - 0.5
                ) * 0.3;
                
                // Calculate distance from current pixel to neighbor's center
                vec2 neighborCenterWorld = (neighborGrid + neighborCenter) * grainSize;
                vec2 currentPixelWorld = pixelPos;
                float distToNeighborCenter = distance(currentPixelWorld, neighborCenterWorld) / grainSize;
                
                // Sample gradient at neighbor position for consistency
                vec2 neighborUV = neighborGrid * grainSize / resolution;
                neighborUV = clamp(neighborUV, vec2(0.0), vec2(1.0));
                float neighborGradientFactor = neighborUV.y; // Use UV fallback
                
                // Check if neighbor should have a dot
                bool neighborHasDot = false;
                if(samplesPerPixel >= 5) {
                    float neighborRandom = hash(neighborGrid + randomSeed + 27.3);
                    neighborHasDot = neighborRandom < neighborGradientFactor;
                } else {
                    float neighborDotSize = maxDotSize * sqrt(neighborGradientFactor);
                    neighborHasDot = true; // Assume it has a dot for size calculation
                }
                
                if(neighborHasDot && distToNeighborCenter < maxDotSize) {
                    isDot = true;
                    break;
                }
            }
            if(isDot) break;
        }
    }
    
    // Output result
    if(isDot) {
        gl_FragColor = vec4(bottomColor, 1.0);
    } else {
        gl_FragColor = vec4(topColor, 1.0);
    }
}

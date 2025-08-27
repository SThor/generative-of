# Dithering Effect Redesign

## Goal
Create a pure two-color dithering effect where gradient illusion comes from varying dot density, not color mixing. Dots should appear randomly distributed with configurable circular size.

## Current Issues
1. **Two-color dots instead of single color**: Currently mixes both palette colors in dots
2. **Background gradient visible**: Should only show through dot density variation
3. **Grid-aligned artifacts**: Even with offsets, still shows grid patterns
4. **Complex shader logic**: Multiple overlapping circle checks make code hard to understand
5. **Squashed circles**: Need perfect circular dots

## Proposed Solution: Post-Processing Filter Approach

### Architecture Overview
```
1. Render gradient to FBO (existing logic)
2. Apply dithering shader as post-process filter
3. Display final result
```

### Benefits
- **Separation of concerns**: Gradient generation separate from dithering
- **Easier debugging**: Can toggle dithering on/off, see intermediate steps
- **Reusable**: Dither shader can work on any input image
- **Selective application**: Easy to add masking for partial effects
- **Simpler logic**: Each pass does one thing well

## Implementation Details

### C++ Changes Required

#### New Members in ofApp.h
```cpp
// Additional FBO for post-processing
ofFbo originalGradientFbo;  // Store original gradient
ofShader ditherShader;      // New post-processing shader
bool ditherShaderLoaded = false;

// New parameters
ofParameter<float> dotRadius;    // Radius in pixels (1-5)
ofParameter<int> samplesPerPixel; // Quality vs performance (4-16)
ofParameter<bool> enableDithering; // Toggle for comparison
```

#### Setup Changes
```cpp
// In setup():
// Load separate dither shader
ditherShaderLoaded = ditherShader.load("shaders/dither_postprocess");

// Create two FBOs
originalGradientFbo.allocate(ofGetWidth(), ofGetHeight());
gradientFbo.allocate(ofGetWidth(), ofGetHeight());

// Add new parameters
ditherGroup.add(dotRadius.set("Dot Radius", 2.0, 0.5, 5.0));
ditherGroup.add(samplesPerPixel.set("Quality", 8, 4, 16));
ditherGroup.add(enableDithering.set("Enable Dithering", true));
```

#### Draw Method Restructure
```cpp
void ofApp::draw() {
    // Step 1: Render original gradient to FBO
    originalGradientFbo.begin();
    ofClear(0, 255);
    ofBackgroundGradient(gradientTop, gradientBottom, OF_GRADIENT_LINEAR);
    originalGradientFbo.end();
    
    if(enableDithering && ditherShaderLoaded) {
        // Step 2: Apply dithering post-process
        gradientFbo.begin();
        ofClear(0, 255);
        
        ditherShader.begin();
        ditherShader.setUniformTexture("gradientTexture", originalGradientFbo.getTexture(), 0);
        ditherShader.setUniform3f("topColor", gradientTop.r/255.0f, gradientTop.g/255.0f, gradientTop.b/255.0f);
        ditherShader.setUniform3f("bottomColor", gradientBottom.r/255.0f, gradientBottom.g/255.0f, gradientBottom.b/255.0f);
        ditherShader.setUniform1f("dotRadius", dotRadius.get() / ofGetWidth()); // Normalize to 0-1
        ditherShader.setUniform1f("randomSeed", randomSeed.get());
        ditherShader.setUniform1i("samplesPerPixel", samplesPerPixel.get());
        ditherShader.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
        
        ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
        ditherShader.end();
        gradientFbo.end();
        
        // Draw processed result
        gradientFbo.draw(0, 0);
    } else {
        // Draw original gradient
        originalGradientFbo.draw(0, 0);
    }
    
    // ... rest of draw method (GUI, info, etc)
}
```

### Shader Implementation

#### New Vertex Shader: `dither_postprocess.vert`
```glsl
#version 120

attribute vec4 position;
varying vec2 vTexCoord;
uniform vec2 resolution;

void main() {
    vTexCoord = position.xy / resolution;
    gl_Position = gl_ModelViewProjectionMatrix * position;
}
```

#### New Fragment Shader: `dither_postprocess.frag`
```glsl
#version 120

varying vec2 vTexCoord;

uniform sampler2D gradientTexture;
uniform vec3 topColor;
uniform vec3 bottomColor;
uniform float dotRadius;
uniform float randomSeed;
uniform int samplesPerPixel;
uniform vec2 resolution;

// Random function with seed
float randomWithSeed(vec2 p, float seed) {
    return fract(sin(dot(p + seed, vec2(12.9898, 78.233))) * 43758.5453);
}

// Generate random offset in range [-1, 1]
vec2 randomOffset(vec2 p, float seed) {
    float x = randomWithSeed(p, seed) * 2.0 - 1.0;
    float y = randomWithSeed(p + vec2(127.1, 311.7), seed) * 2.0 - 1.0;
    return vec2(x, y);
}

void main() {
    vec2 uv = vTexCoord;
    
    // Sample original gradient color at this position
    vec3 originalColor = texture2D(gradientTexture, uv).rgb;
    
    // Calculate gradient factor by comparing to palette colors
    float distToTop = distance(originalColor, topColor);
    float distToBottom = distance(originalColor, bottomColor);
    float gradientFactor = distToBottom / (distToTop + distToBottom + 0.001); // Avoid division by zero
    
    // Density-based random sampling for dots
    bool foundDot = false;
    float maxSearchRadius = dotRadius * 2.0; // Search area around current pixel
    
    for(int i = 0; i < samplesPerPixel && !foundDot; i++) {
        // Generate random potential dot center near current pixel
        vec2 randomPos = uv + randomOffset(uv * 1000.0 + float(i), randomSeed + float(i)) * maxSearchRadius;
        
        // Sample gradient factor at potential dot center
        vec3 centerColor = texture2D(gradientTexture, randomPos).rgb;
        float centerDistToTop = distance(centerColor, topColor);
        float centerDistToBottom = distance(centerColor, bottomColor);
        float centerGradientFactor = centerDistToBottom / (centerDistToTop + centerDistToBottom + 0.001);
        
        // Decide if we should place a dot at this position based on density
        float randomValue = randomWithSeed(floor(randomPos * 1000.0), randomSeed + float(i) * 17.0);
        
        if(randomValue < centerGradientFactor) {
            // Place dot here, check if current pixel is inside it
            float distanceToCenter = distance(uv, randomPos);
            if(distanceToCenter <= dotRadius) {
                foundDot = true;
            }
        }
    }
    
    // Output pure two-color result
    if(foundDot) {
        gl_FragColor = vec4(bottomColor, 1.0);
    } else {
        gl_FragColor = vec4(topColor, 1.0);
    }
}
```

## Parameters to Expose

### Essential Parameters
- **Dot Radius**: 0.5 - 5.0 pixels
- **Random Seed**: 0 - 1000 (for reproducible randomness)
- **Quality (Samples Per Pixel)**: 4 - 16 (performance vs quality trade-off)
- **Enable Dithering**: Boolean toggle for comparison

### Optional Future Parameters
- **Dot Softness**: Hard edges vs soft anti-aliased edges
- **Density Curve**: Linear vs custom gradient mapping
- **Mask Texture**: For selective application

## Key Algorithm Insights

### Density-Based Sampling
- For each pixel, sample multiple random positions nearby
- At each position, check if gradient value warrants placing a dot
- Use probability = gradient factor (0 at top, 1 at bottom)
- If dot should be placed AND current pixel is inside dot radius → color it

### Avoiding Grid Artifacts
- Use continuous random sampling instead of grid-based placement
- Multiple samples per pixel ensure good coverage
- Random offsets break any remaining patterns

### Performance Considerations
- `samplesPerPixel` parameter balances quality vs speed
- Lower values (4-6) for real-time interaction
- Higher values (12-16) for final rendering

## Testing Strategy

1. **A/B Comparison**: Toggle dithering on/off to compare with original gradient
2. **Parameter Sweeps**: Test different dot radii and sample counts
3. **Seed Variation**: Verify different seeds produce different but similar-quality results
4. **Edge Cases**: Test at 0% and 100% gradient positions
5. **Performance**: Monitor frame rate with different quality settings

## Future Enhancements

### Masking Support
```glsl
uniform sampler2D maskTexture;
// In main(): if(texture2D(maskTexture, uv).r < 0.5) return original;
```

### Multiple Dot Sizes
- Sample from distribution of dot sizes for more organic look
- Larger dots in high-density areas, smaller in sparse areas

### Color Temperature Variation
- Slight hue shifts in dots for more natural appearance
- Simulate ink/paper texture effects

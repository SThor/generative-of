#version 150

uniform sampler2D contentTex;
uniform sampler2D ageTex;
uniform float currentFrame;
uniform float maxAge;
uniform float ageBlurMultiplier;
uniform float ageFadeRate;
uniform float blurAmount;
uniform float texWidth;
uniform float texHeight;
uniform float ageRangeStart;
uniform float ageRangeEnd;
uniform int useRelativeAge;

in vec2 texCoordVarying;
out vec4 outputColor;

float mapAgeToInfluence(float age) {
    if(useRelativeAge == 1) {
        // Relative age mapping: 0 = current frame, 1 = oldest frame
        float relativeAge = age / currentFrame;
        return clamp(relativeAge, 0.0, 1.0);
    } else {
        // Absolute age range mapping
        if(age < ageRangeStart) {
            return 0.0; // No influence if too young
        } else if(age > ageRangeEnd) {
            return 1.0; // Maximum influence if very old
        } else {
            // Linear interpolation between start and end
            return (age - ageRangeStart) / (ageRangeEnd - ageRangeStart);
        }
    }
}

void main() {
    vec2 texCoord = texCoordVarying;
    
    // This pixel will be influenced by all surrounding pixels based on THEIR age
    vec4 finalColor = vec4(0.0);
    float totalWeight = 0.0;
    
    // Define maximum search radius
    float maxRadius = blurAmount * (1.0 + ageBlurMultiplier) * 8.0;
    
    // Sample surrounding pixels in a square pattern
    for(float x = -maxRadius; x <= maxRadius; x += 1.0) {
        for(float y = -maxRadius; y <= maxRadius; y += 1.0) {
            vec2 offset = vec2(x / texWidth, y / texHeight);
            vec2 sampleCoord = texCoord + offset;
            
            // Only sample within texture bounds
            if(sampleCoord.x >= 0.0 && sampleCoord.x <= 1.0 && 
               sampleCoord.y >= 0.0 && sampleCoord.y <= 1.0) {
                
                // Sample the content and age at this offset position
                vec4 sampleContent = texture(contentTex, sampleCoord);
                vec4 sampleAge = texture(ageTex, sampleCoord);
                
                // Calculate the age of the sampled pixel
                float samplePixelFrame = (sampleAge.r * 255.0 * 65536.0 + sampleAge.g * 255.0 * 256.0 + sampleAge.b * 255.0);
                float sampleAge_value = currentFrame - samplePixelFrame;
                
                // If no age data, treat as background (age = -1)
                if(sampleAge.r == 0.0 && sampleAge.g == 0.0 && sampleAge.b == 0.0) {
                    sampleAge_value = -1.0;
                }
                
                // Calculate influence weight based on distance and age
                float distance = sqrt(x*x + y*y);
                float baseWeight = 1.0;
                
                if(sampleAge_value >= 0.0) {
                    // This sample has actual content, calculate age-based influence
                    float normalizedAge = mapAgeToInfluence(sampleAge_value);
                    
                    // Older pixels have more influence (larger blur radius)
                    float ageInfluence = 1.0 + normalizedAge * ageBlurMultiplier;
                    float effectiveRadius = blurAmount * ageInfluence * 8.0;
                    
                    // Weight decreases with distance, but aged pixels influence further
                    if(distance <= effectiveRadius) {
                        baseWeight = (effectiveRadius - distance) / effectiveRadius;
                        
                        // Apply age-based opacity fade
                        float opacity = 1.0 - (normalizedAge * ageFadeRate);
                        opacity = clamp(opacity, 0.0, 1.0);
                        baseWeight *= opacity;
                    } else {
                        baseWeight = 0.0;
                    }
                } else {
                    // Background pixel - only has influence at distance 0 (itself)
                    if(distance < 1.0) {
                        baseWeight = 1.0;
                    } else {
                        baseWeight = 0.0;
                    }
                }
                
                finalColor += sampleContent * baseWeight;
                totalWeight += baseWeight;
            }
        }
    }
    
    // Normalize the result
    if(totalWeight > 0.0) {
        finalColor /= totalWeight;
    } else {
        // Fallback to original content
        finalColor = texture(contentTex, texCoord);
    }
    
    outputColor = finalColor;
}

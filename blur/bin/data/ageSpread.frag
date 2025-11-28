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

void main() {
    vec2 texCoord = texCoordVarying;
    
    // Start with original content
    vec4 originalColor = texture(contentTex, texCoord);
    vec4 ageData = texture(ageTex, texCoord);
    vec4 finalColor = originalColor;
    
    // Gradual protection instead of hard cutoff
    float protectionFactor = 1.0;
    if(ageData.r > 0.0 || ageData.g > 0.0 || ageData.b > 0.0) {
        float pixelFrame = (ageData.r * 255.0 * 65536.0 + ageData.g * 255.0 * 256.0 + ageData.b * 255.0);
        float age = currentFrame - pixelFrame;
        
        // Gradual transition from protected to spreadable
        if(age < ageRangeStart) {
            protectionFactor = 1.0; // Fully protected
        } else if(age < ageRangeStart + 50.0) {
            // Gradual transition over 50 frames
            protectionFactor = 1.0 - ((age - ageRangeStart) / 50.0);
        } else {
            protectionFactor = 0.0; // Fully spreadable
        }
    }
    
    if(protectionFactor < 0.8) { // Allow some spreading even on semi-recent pixels
        vec2 onePixel = vec2(1.0/texWidth, 1.0/texHeight);
        
        // Use blurAmount parameter to control spread radius
        float maxSearchRadius = blurAmount * ageBlurMultiplier * 15.0; // Connect to UI parameters!
        maxSearchRadius = clamp(maxSearchRadius, 10.0, 200.0); // Reasonable limits
        
        for(float x = -maxSearchRadius; x <= maxSearchRadius; x += 10.0) {
            for(float y = -maxSearchRadius; y <= maxSearchRadius; y += 10.0) {
                vec2 offset = vec2(x, y) * onePixel;
                vec2 sampleCoord = texCoord + offset;
                
                if(sampleCoord.x >= 0.0 && sampleCoord.x <= 1.0 && 
                   sampleCoord.y >= 0.0 && sampleCoord.y <= 1.0) {
                    
                    vec4 sampleAge = texture(ageTex, sampleCoord);
                    
                    if(sampleAge.r > 0.0 || sampleAge.g > 0.0 || sampleAge.b > 0.0) {
                        float pixelFrame = (sampleAge.r * 255.0 * 65536.0 + sampleAge.g * 255.0 * 256.0 + sampleAge.b * 255.0);
                        float age = currentFrame - pixelFrame;
                        
                        if(age > ageRangeStart) { // Use parameter for age threshold
                            float distance = sqrt(x*x + y*y);
                            
                            // Map age to influence using the existing function logic
                            float ageNormalized;
                            if(useRelativeAge == 1) {
                                ageNormalized = age / currentFrame;
                            } else {
                                if(age < ageRangeStart) {
                                    ageNormalized = 0.0;
                                } else if(age > ageRangeEnd) {
                                    ageNormalized = 1.0;
                                } else {
                                    ageNormalized = (age - ageRangeStart) / (ageRangeEnd - ageRangeStart);
                                }
                            }
                            ageNormalized = clamp(ageNormalized, 0.0, 1.0);
                            
                            // Variable spread radius based on age and parameters
                            float spreadRadius = 20.0 + (ageNormalized * maxSearchRadius * 0.8);
                            
                            if(distance <= spreadRadius && ageNormalized > 0.0) {
                                // Non-linear falloff for more interesting variation
                                float distanceFalloff = 1.0 - (distance / spreadRadius);
                                distanceFalloff = pow(distanceFalloff, 2.5); // More dramatic falloff
                                
                                // Get the original color and apply age-based fading
                                vec4 sampleContent = texture(contentTex, sampleCoord);
                                
                                // Use ageFadeRate parameter for fading
                                float fadeAmount = 1.0 - (ageNormalized * ageFadeRate * 50.0);
                                fadeAmount = clamp(fadeAmount, 0.2, 1.0); // Don't fade completely
                                
                                vec4 backgroundColor = vec4(0.96, 0.96, 0.96, 1.0);
                                vec4 fadedColor = mix(sampleContent, backgroundColor, 1.0 - fadeAmount);
                                
                                // Strength respects protection and uses parameters
                                float spreadStrength = ageNormalized * (1.0 - protectionFactor) * 0.4 * distanceFalloff;
                                
                                finalColor = mix(finalColor, fadedColor, spreadStrength);
                            }
                        }
                    }
                }
            }
        }
    }
    
    outputColor = finalColor;
}

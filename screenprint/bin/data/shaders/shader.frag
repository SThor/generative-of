OF_GLSL_SHADER_HEADER

uniform vec2 resolution;
uniform float intensity;
out vec4 outputColor;

// 0: Addition, 1: Screen, 2: Overlay, 3: Soft Light, 4: Lighten-Only
uniform int blendMode;

#define INTENSITY 0.075
// What gray level noise should tend to.
#define MEAN 0.0
// Controls the contrast/variance of noise.
#define VARIANCE 0.5

// Gold Noise ©2015 dcerisano@standard3d.com
// - based on the Golden Ratio
// - uniform normalized distribution
// - fastest static noise generator function (also runs at low precision)
// - use with indicated fractional seeding method.
// Source - https://stackoverflow.com/a/28095165
// Posted by Birkensox, modified by community. See post 'Timeline' for change history
// Retrieved 2025-11-30, License - CC BY-SA 4.0

float PHI = 1.61803398874989484820459;  // Φ = Golden Ratio   

float gold_noise(in vec2 xy, in float seed){
       return fract(tan(distance(xy*PHI, xy)*seed)*xy.x);
}

vec3 channel_mix(vec3 a, vec3 b, vec3 w) {
    return vec3(mix(a.r, b.r, w.r), mix(a.g, b.g, w.g), mix(a.b, b.b, w.b));
}

float gaussian(float z, float u, float o) {
    return (1.0 / (o * sqrt(2.0 * 3.1415))) * exp(-(((z - u) * (z - u)) / (2.0 * (o * o))));
}

vec3 madd(vec3 a, vec3 b, float w) {
    return a + a * b * w;
}

vec3 screen(vec3 a, vec3 b, float w) {
    return mix(a, vec3(1.0) - (vec3(1.0) - a) * (vec3(1.0) - b), w);
}

vec3 overlay(vec3 a, vec3 b, float w) {
    return mix(a, channel_mix(
        2.0 * a * b,
        vec3(1.0) - 2.0 * (vec3(1.0) - a) * (vec3(1.0) - b),
        step(vec3(0.5), a)
    ), w);
}

vec3 soft_light(vec3 a, vec3 b, float w) {
    return mix(a, pow(a, pow(vec3(2.0), 2.0 * (vec3(0.5) - b))), w);
}

void main()
{
    // Get the original color using GL2 built-in variable
    vec4 originalColor = gl_Color;
    
    // Use fragment coordinates for noise generation
    vec2 st = gl_FragCoord.xy;
    vec2 uv = st / resolution; // Use uniform resolution instead of hardcoded values
    
    // Generate noise values
    float r_noise = gold_noise(st, 12.9898);
    float g_noise = gold_noise(st, 78.233);
    float b_noise = gold_noise(st, 37.719);
    vec3 noise = vec3(r_noise, g_noise, b_noise);
    
    vec3 newColor;
    
    if(blendMode == 1)
    {
        // Multiply blend (darkens, safe)
        newColor = originalColor.rgb * noise;  
        newColor = mix(originalColor.rgb, newColor, intensity);
    }
    else
    {
        // Screen blend (brightens, but clamped safe)
        // Screen formula: 1 - (1-original) * (1-noise)
        newColor = vec3(1.0) - (vec3(1.0) - originalColor.rgb) * (vec3(1.0) - noise);
        newColor = clamp(newColor, 0.0, 1.0);
    }
    vec3 blendedColor = mix(originalColor.rgb, newColor, intensity);
    
    // // Debug: Use a simple test pattern instead of complex noise
    // float testPattern = sin(st.x * 0.1) * sin(st.y * 0.1) * 0.5 + 0.5;
    // vec3 grain = vec3(testPattern);
    
    // // Use the intensity uniform
    // float w = intensity;

    // switch (blendMode) {
    //     case 0: // Addition
    //         blendedColor = clamp(originalColor.rgb + grain * w, 0.0, 1.0);
    //         break;
    //     case 1: // Screen
    //         blendedColor = screen(originalColor.rgb, grain, w);
    //         break;
    //     case 2: // Overlay
    //         blendedColor = overlay(originalColor.rgb, grain, w);
    //         break;
    //     case 3: // Soft Light
    //         blendedColor = soft_light(originalColor.rgb, grain, w);
    //         break;
    //     case 4: // Lighten-Only
    //         blendedColor = max(originalColor.rgb, grain * w);
    //         break;
    //     default: // Fallback: mix original with grain
    //         blendedColor = mix(originalColor.rgb, grain, w);
    //         break;
    // }
    
    outputColor = vec4(blendedColor, originalColor.a);
}
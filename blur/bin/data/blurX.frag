#version 150

uniform sampler2D tex0;
uniform float blurAmnt;
uniform float texwidth;

in vec2 texCoordVarying;
out vec4 outputColor;

// 13-tap Gaussian kernel with proper normalization (sums to 1.0)
// Based on https://observablehq.com/@jobleonard/gaussian-kernel-calculater

void main()
{
	vec4 color = vec4(0.0, 0.0, 0.0, 0.0);

	color += 0.0024055085674964125 * texture(tex0, texCoordVarying + vec2(blurAmnt * -6.0/texwidth, 0.0));
	color += 0.009255297393309877 * texture(tex0, texCoordVarying + vec2(blurAmnt * -5.0/texwidth, 0.0));
	color += 0.02786684424768963 * texture(tex0, texCoordVarying + vec2(blurAmnt * -4.0/texwidth, 0.0));
	color += 0.06566651775036977 * texture(tex0, texCoordVarying + vec2(blurAmnt * -3.0/texwidth, 0.0));
	color += 0.12111723251079276 * texture(tex0, texCoordVarying + vec2(blurAmnt * -2.0/texwidth, 0.0));
	color += 0.17486827308986305 * texture(tex0, texCoordVarying + vec2(blurAmnt * -1.0/texwidth, 0.0));

	color += 0.1976406528809569 * texture(tex0, texCoordVarying + vec2(0.0, 0));

	color += 0.17486827308986305 * texture(tex0, texCoordVarying + vec2(blurAmnt * 1.0/texwidth, 0.0));
	color += 0.12111723251079276 * texture(tex0, texCoordVarying + vec2(blurAmnt * 2.0/texwidth, 0.0));
	color += 0.06566651775036977 * texture(tex0, texCoordVarying + vec2(blurAmnt * 3.0/texwidth, 0.0));
	color += 0.02786684424768963 * texture(tex0, texCoordVarying + vec2(blurAmnt * 4.0/texwidth, 0.0));
	color += 0.009255297393309877 * texture(tex0, texCoordVarying + vec2(blurAmnt * 6.0/texwidth, 0.0));
	color += 0.0024055085674964125 * texture(tex0, texCoordVarying + vec2(blurAmnt * 6.0/texwidth, 0.0));

    outputColor = color;
}

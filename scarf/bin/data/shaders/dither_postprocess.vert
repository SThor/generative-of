#version 120

attribute vec4 position;
varying vec2 vTexCoord;
uniform vec2 resolution;

void main() {
    vTexCoord = position.xy / resolution;
    gl_Position = gl_ModelViewProjectionMatrix * position;
}

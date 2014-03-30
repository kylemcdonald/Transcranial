#version 120

uniform sampler2DRect bg, fg;
uniform vec2 resolution;
varying vec2 texCoord, invTexCoord;

void main() {
    texCoord = gl_MultiTexCoord0.xy;
    invTexCoord = texCoord;
    invTexCoord.x = resolution.x - invTexCoord.x;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
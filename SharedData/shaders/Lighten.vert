#version 120

uniform sampler2DRect bg, fg;
varying vec2 texCoord;

void main() {
    texCoord = gl_MultiTexCoord0.xy;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
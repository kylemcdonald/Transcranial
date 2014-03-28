#version 120

uniform sampler2DRect source;
uniform sampler2DRect offset;
uniform float strength;
varying vec2 texCoord;

void main() {
    vec2 offsetVec = texture2DRect(offset, texCoord).xy;
    gl_FragColor = texture2DRect(source, texCoord + offsetVec * strength);
}
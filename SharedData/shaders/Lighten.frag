#version 120

uniform sampler2DRect bg, fg;
varying vec2 texCoord;

void main() {
    gl_FragColor = max(texture2DRect(bg, texCoord),
                       texture2DRect(fg, texCoord));
}
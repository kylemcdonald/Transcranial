#version 120

uniform sampler2DRect bg, fg;
uniform vec2 offset;
varying vec2 texCoord;

void main() {
    vec3 bgColor = texture2DRect(bg, texCoord).rgb;
    vec3 fgColor = texture2DRect(fg, texCoord - offset).rgb;
    float bgBright = length(bgColor);
    float fgBright = length(fgColor);
    float sum = bgBright + fgBright;
    gl_FragColor = vec4(fgBright > bgBright ? fgColor : bgColor, 1.);
}
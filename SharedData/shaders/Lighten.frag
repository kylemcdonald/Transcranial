#version 120

uniform sampler2DRect a, b;
uniform vec2 offset;
uniform vec2 resolution;
varying vec2 texCoord;

// texture wrap modes aren't working
vec4 clampedSample(sampler2DRect rect, vec2 texCoord) {
    if(texCoord.x < 0 || texCoord.y < 0 ||
       texCoord.x > resolution.x || texCoord.y > resolution.y) {
        return vec4(0, 0, 0, 0);
    } else {
        return texture2DRect(rect, texCoord);
    }
}

void main() {
    gl_FragColor = max(clampedSample(a, texCoord + offset),
                       clampedSample(b, texCoord - offset));
}
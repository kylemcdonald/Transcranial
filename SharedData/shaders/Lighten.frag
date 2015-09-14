#version 120

uniform sampler2DRect a, b;
uniform float opacity;
uniform vec2 offset;
uniform vec2 resolution;
varying vec2 texCoord;
varying vec2 invTexCoord;

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
    gl_FragColor = max(clampedSample(a, invTexCoord - offset),
                       clampedSample(b, texCoord - offset) * opacity);
}
uniform sampler2DRect src, dst, srcBlur, dstBlur;

varying vec2 dstPos;

void main() {
	vec2 srcPos = gl_TexCoord[0].st;	
	vec3 srcColorBlur = texture2DRect(srcBlur, srcPos).rgb;
	vec3 dstColorBlur = texture2DRect(dstBlur, dstPos).rgb;
	vec3 offset = (dstColorBlur - srcColorBlur);	
	vec3 srcColor = texture2DRect(src, srcPos).rgb;
	gl_FragColor = vec4(srcColor + offset, 1.);
}
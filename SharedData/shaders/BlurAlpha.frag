uniform sampler2DRect tex;

const int kernelSize = 8;
const int kernelSide = kernelSize + kernelSize + 1;
const int kernelCount = kernelSide * kernelSide;

void main() {
	vec2 st = gl_TexCoord[0].st;

	float alpha = 0.;
	float count = 0.;
	for(int y = -kernelSize; y <= kernelSize; y++) {
		for(int x = -kernelSize; x <= kernelSize; x++) {
			vec2 cur = vec2(float(x), float(y));
			alpha += texture2DRect(tex, st + cur).a;
			count++;
		}
	}
	
	alpha /= count; // normalize blur
	alpha = (alpha * 2.) - 1.; // scale for inner edges only
	gl_FragColor = vec4(texture2DRect(tex, st).rgb, alpha);
}
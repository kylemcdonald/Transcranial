#version 120

uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iGlobalTime;           // shader playback time (in seconds)

const float PI = 3.1415926536;

float quantize(float x, float amount) {
	return floor(x / amount) * amount;
}

vec2 quantize(vec2 v, float amount) {
	return vec2(
		quantize(v.x, amount),
		quantize(v.y, amount));
}

float rand (vec2 v) {
	return fract(sin(dot(v, vec2 (.129898,.78233))) * 437585.453);
}

float rand(float seed) {
	return fract(sin(seed * 12.9898) * 43758.5453);
}

// triangle wave from 0 to 1
float wrap(float n) {
	return abs(mod(n, 2.)-1.) * -1. + 1.;
}

// creates a cosine wave in the plane at a given angle
float wave(float angle, vec2 point, float phase) {
	float cth = cos(angle);
	float sth = sin(angle);
	return (cos (phase + cth*point.x + sth*point.y) + 1.) / 2.;
}

const float waves = 10.;
// sum cosine waves at various interfering angles
// wrap values when they exceed 1
float quasi(float interferenceAngle, vec2 point, float phase) {
	float sum = 0.;
	for (float i = 0.; i < waves; i++) {
		sum += wave(PI*i*interferenceAngle, point, phase);
	}
	return wrap(sum);
}

void main() {
	float time = iGlobalTime;
	float stage = 0;//mod(time / 2., 5.);
	
	vec2 size = iResolution.xy;
	float positionDivider = 0;//floor(mod(time / 4., 2.)) * 4.;
	float camoStrength = 4;//floor(mod(time / 8., 2.)) * 10.;
	float offsetSpeed = 2;//floor(mod(time / 16., 2.)) * 1.;
	float scrollSpeed = 2;//floor(mod(time / 32., 2.)) * 1.;
	
	float b;
	vec2 position = vec2(gl_FragCoord.x, size.y - gl_FragCoord.y);
	vec2 raw = position;
	position.x += sin(time * 5. + position.y / 25.) * camoStrength;
	position.y += sin(time * 5. + position.x / 25.) * camoStrength;
	position.x += sin(time * 5. + position.y / 50.) * 2. * camoStrength;
	position.y += sin(time * 5. + position.x / 50.) * 2. * camoStrength;
	if(positionDivider > 1.) {
		vec2 quant = quantize(position, positionDivider);
		if(offsetSpeed > 0.) {
			float offset = rand(quant);
			position = quantize(position + offset * time * offsetSpeed, positionDivider);
		} else if (scrollSpeed > 0.) {
			float offset = rand(quant.x);
			position = quantize(position + vec2(0, offset * time * scrollSpeed), positionDivider);
		} else {
			position = quant;
		}
	}
	vec2 center = size / 2.;
	vec2 positionNorm = (position - center) / size.y;
	float positionAngle = atan(positionNorm.y / positionNorm.x);

	if(stage <= 1.) {
		float angleSpeed = .01;
		float phaseSpeed = 5.;
		float scale = 100.;
		b = quasi(time * angleSpeed, (positionNorm) * scale, time * phaseSpeed);
	} else if(stage <= 2.) {
		float chunks = 20.;
		float angle = .01 * time;
		b = wave(angle, positionNorm * chunks, time);
	} else if(stage <= 3.) {
		float base = 100.;
		const int n = 6;
		for(int i = 0; i < n; i++) {
			b = rand(quantize(raw, base));
			b = fract(b + time);
			b *= float(n - i);
			if(b < 1.) {
				break;
			}
			base /= 2.;
		}
		//b = rand(quantize(raw, 4) + time);
	} else if(stage <= 4.) {
		float speed = -10.;
		float scale = 100.;
		float wiggleStrength = .005;
		float petals = 20.;
		float spiral = 20.;
		float power = .1;
		float wiggle = sin(length(positionNorm) * spiral + time + positionAngle * petals);
		float base = pow(length(positionNorm), power) + wiggle * wiggleStrength;
		b = sin(base * scale + speed * time);	
	} else if(stage <= 5.) {
		b = rand(time);
	} else if(stage <= 6.) {
		b = quasi(.0001 * (sin(time) + 1.5) * 400., (positionNorm) * 700., time);
	}

	if(stage < 4.) {
		// post process to black and white
		b = b > .5 ? 1. : 0.;
	}

	gl_FragColor = vec4(vec3(b), 1.);
}

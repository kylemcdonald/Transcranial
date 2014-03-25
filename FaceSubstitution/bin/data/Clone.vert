uniform sampler2DRect src, dst;

varying vec2 dstPos;

void main() {
	gl_Position = ftransform();
	gl_TexCoord[0] = gl_MultiTexCoord0;
	dstPos = gl_Vertex.xy;
}

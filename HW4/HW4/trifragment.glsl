uniform sampler2D screenFramebuffer;
varying vec2 texCoordVar;

void main() {
	
    gl_FragColor = texture2D(screenFramebuffer, texCoordVar);
}
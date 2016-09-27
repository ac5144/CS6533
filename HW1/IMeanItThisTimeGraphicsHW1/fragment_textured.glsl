uniform sampler2D diffuse;
varying vec2 texCoordVar;

uniform float time;

void main() {
	vec2 texCoord = vec2(texCoordVar.x, texCoordVar.y + time);
    gl_FragColor = texture2D(diffuse, texCoord);
}
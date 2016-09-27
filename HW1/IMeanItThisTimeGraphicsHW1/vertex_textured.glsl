attribute vec4 position;
attribute vec2 texCoord;
varying vec2 texCoordVar;

uniform vec2 modelPosition;

void main()
{
    texCoordVar = texCoord;
	gl_Position = vec4(modelPosition.x, modelPosition.y, 0.0, 0.0) + position;
}
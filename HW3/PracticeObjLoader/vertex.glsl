attribute vec4 position;
attribute vec2 texCoord;

attribute vec4 normal;
attribute vec4 binormal;
attribute vec4 tangent;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 normalMatrix;

varying vec3 varyingPosition;
varying vec2 varyingTexCoord;

varying mat3 varyingTBNMatrix;

void main()
{
	varyingTexCoord = texCoord;
	vec4 p = modelViewMatrix * position;
	varyingPosition = p.xyz;
	varyingTBNMatrix = mat3(normalize((normalMatrix * tangent).xyz), normalize((normalMatrix * binormal).xyz), normalize((normalMatrix * normal).xyz));
	gl_Position = projectionMatrix * modelViewMatrix * position;
}
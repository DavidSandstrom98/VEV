#version 120

varying vec4 f_color;
varying vec2 f_texCoord;

uniform sampler2D texture0;

void main() {
	vec4 f_texColor = texture2D(texture0, f_texCoord);
	gl_FragColor = f_color * f_texColor;
}

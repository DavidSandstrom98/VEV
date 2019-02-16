#version 120

uniform mat4 modelToCameraMatrix; // M modelview
uniform mat4 cameraToClipMatrix;  // P proyeccion

attribute vec3 v_position;

varying vec4 f_color;

void main() {

	f_color = vec4(1.0, 1.0, 1.0, 1.0);
	vec4 vpos = vec4(v_position, 1.0);
	gl_Position = cameraToClipMatrix * modelToCameraMatrix * vpos;
}

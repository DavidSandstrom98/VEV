#version 120

attribute vec3 v_position;
attribute vec3 v_normal;
attribute vec2 v_texCoord;

uniform int active_lights_n; // Number of active lights (< MG_MAX_LIGHT)

uniform mat4 modelToCameraMatrix;
uniform mat4 cameraToClipMatrix;
uniform mat4 modelToWorldMatrix;
uniform mat4 modelToClipMatrix;
uniform mat4 worldToShadowCameraClip;

varying vec3 f_position;
varying vec3 f_viewDirection;
varying vec3 f_normal;
varying vec2 f_texCoord;
varying vec3 L_position;

void main() {
	//En este caso no hay que normalizar la normal.
	//Si no al hacer la interpolacion pueden pasar cosas raras.
	f_position = vec3( modelToCameraMatrix * vec4(v_position, 1.0) );
	f_normal = vec3( modelToCameraMatrix * vec4(v_normal, 0.0) );
	f_texCoord = v_texCoord;
	f_viewDirection = vec3( (0.0, 0.0, 0.0, 1.0) - f_position );
    L_position =  (worldToShadowCameraClip * modelToWorldMatrix * vec4(f_position, 1.0)).xyz;

	gl_Position = modelToClipMatrix * vec4(v_position, 1.0);
}
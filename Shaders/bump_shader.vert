#version 120

// Bump mapping with many lights.
//
// All computations are performed in the tangent space; therefore, we need to
// convert all light (and spot) directions and view directions to tangent space
// and pass them the fragment shader.
// ./browser Json/scene_bmap.json
varying vec2 f_texCoord;
varying vec3 f_viewDirection;     // tangent space
varying vec3 f_lightDirection[4]; // tangent space
varying vec3 f_spotDirection[4];  // tangent space

// all attributes in model space
attribute vec3 v_position;
attribute vec3 v_normal;
attribute vec2 v_texCoord;
attribute vec3 v_TBN_t;//Tangente
attribute vec3 v_TBN_b;//Bitangente

uniform mat4 modelToCameraMatrix;
uniform mat4 modelToWorldMatrix;
uniform mat4 cameraToClipMatrix;
uniform mat4 modelToClipMatrix;

uniform int active_lights_n; // Number of active lights (< MG_MAX_LIGHT)

uniform struct light_t {
	vec4 position;    // Camera space
	vec3 diffuse;     // rgb
	vec3 specular;    // rgb
	vec3 attenuation; // (constant, lineal, quadratic)
	vec3 spotDir;     // Camera space
	float cosCutOff;  // cutOff cosine
	float exponent;
} theLights[4];     // MG_MAX_LIGHTS

void main() {

	mat3 MV3x3 = mat3(modelToCameraMatrix); // 3x3 modelview matrix

	//Tangente, bitangente, normal y posicion del vertice en coordenadas de la camara
	vec3 cameraTangent = MV3x3 * v_TBN_t;
	vec3 cameraBiTangent = MV3x3 * v_TBN_b;
	vec3 cameraNormal = MV3x3 * v_normal;
	vec3 cameraPosition = (modelToCameraMatrix * vec4(v_position, 1.0)).xyz;

	//Por defecto se crea por columnas por lo que es necesario transponerla
	//leido por filas normal, tangente y bitangente.
	mat3 TangentMatrix = transpose(mat3(cameraTangent, cameraBiTangent, cameraNormal));
	
	//Vector del punto a la camara
	f_viewDirection = TangentMatrix * (-cameraPosition);

	for (int i = 0; i < 4; i++)
	{
		if(theLights[i].position.w == 1.0)//Luz no direccional
			f_lightDirection[i] = TangentMatrix * (theLights[i].position.xyz - cameraPosition);
		else//luz direccional
			f_lightDirection[i] = TangentMatrix * (-theLights[i].position.xyz);
		
		if(theLights[i].cosCutOff > 0.0)//Solo para las linternas
			f_spotDirection[i] = TangentMatrix * theLights[i].spotDir;
		
	}

	f_texCoord = v_texCoord;

	gl_Position = modelToClipMatrix * vec4(v_position, 1.0);
}
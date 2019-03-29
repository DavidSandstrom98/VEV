#version 120

uniform mat4 modelToCameraMatrix;
uniform mat4 cameraToClipMatrix;
uniform mat4 modelToWorldMatrix;
uniform mat4 modelToClipMatrix;

uniform int active_lights_n; // Number of active lights (< MG_MAX_LIGHT)
uniform vec3 scene_ambient;  // rgb

uniform struct light_t {
	vec4 position;    // Camera space de ser direccional sera la direccion de la misma vector o punto segun convenga
	vec3 diffuse;     // rgb
	vec3 specular;    // rgb
	vec3 attenuation; // (constant, lineal, quadratic)
	vec3 spotDir;     // Camera space
	float cosCutOff;  // cutOff cosine
	float exponent;
} theLights[4];     // MG_MAX_LIGHTS

uniform struct material_t {
	vec3  diffuse;
	vec3  specular;
	float alpha;
	float shininess;
} theMaterial;

attribute vec3 v_position; // Model space
attribute vec3 v_normal;   // Model space
attribute vec2 v_texCoord;

varying vec4 f_color;
varying vec2 f_texCoord;

uniform float u_time;

float lambert_factor(vec3 n, const vec3 l) {//Si es 0 no hay componente especular
	float fac = dot(n,l);//producto escalar
	fac = max(0.0,fac);//El maximo entre 0.0 y el factor para asegurar que no sale negativo
	return fac;
}

float specular_factor(const vec3 n,
					  const vec3 l,
					  const vec3 v,
					  float m) {
	float NoL = dot(n, l);

	if(NoL <= 0.0) return 0.0;

	vec3 r = 2 * NoL * n - l;
	float RoV = dot(r, v);

	if(RoV > 0.0){
		RoV = NoL * pow(RoV, m);
	}else{
		RoV = 0.0;
	}

	return RoV;
}

void direction_light(const in int i,
					 const in vec3 lightDirection,
					 const in vec3 viewDirection,
					 const in vec3 normal,
					 inout vec3 diffuse, inout vec3 specular) {

	//Calcular aportacion difusa de la luz al vertice y sumarlo a diffuse
	float lam = lambert_factor(normal, lightDirection);

	if(lam > 0.0){
		diffuse += lam * theMaterial.diffuse;
	}

	float espectacular = specular_factor(normal, lightDirection, viewDirection,theMaterial.shininess);

	if(espectacular > 0.0){
		specular += espectacular * theMaterial.specular;
	}

}

void point_light(const in int i,
				 const in vec3 position,
				 const in vec3 viewDirection,
				 const in vec3 normal,
				 inout vec3 diffuse, inout vec3 specular) {
}


// Note: no attenuation in spotlights
void spot_light(const in int i,
				const in vec3 position,
				const in vec3 viewDirection,
				const in vec3 normal,
				inout vec3 diffuse, inout vec3 specular) {
}

void main() {

	//Pasar v_position y v_normal al espacio de la camara
	vec3 diffuse = vec3(0.0);
	vec3 specular = vec3(0.0);

	vec4 positionEye = modelToCameraMatrix * vec4(v_position, 1.0);//Posicion del vertice en coordenadas de la camara

	vec3 viewDirection = vec3( (0.0, 0.0, 0.0, 1.0) - positionEye );//Vector desde la camara al vertice
	viewDirection = normalize(viewDirection);

	//normal y posicion del vertice en coordenadas de la camara
	vec3 normal = vec3(modelToCameraMatrix *vec4(v_normal, 0.0)); 
	normal = normalize(normal);

	vec3 lightDirection;
	
	for(int i=0; i < active_lights_n; ++i) {
		if(theLights[i].position.w == 0.0) {
		  	// direction light
			//Con el material se puede sacar factor comun y multiplicar solo al final
			lightDirection = (-1.0)*theLights[i].position.xyz;//cambiar la direccion y coger solo las tres primeras componentes
			lightDirection = normalize(lightDirection);//Hay que asegurarse de que esta normalizado

			direction_light(i, lightDirection, viewDirection, normal, diffuse, specular); 	
		} else {
		  if (theLights[i].cosCutOff == 0.0) {
			// point light
		  } else {
			// spot light
		  }
		}
	}

	//Parte de ¡l color relacionada con la animacion
	//f_color = vec4(1.0);
	float s = sin(u_time);
	float c = cos(u_time);
	float sc = 1-(s+c)/2;
	f_color = vec4(s, c, sc, 1);
	
	f_color.rgb = scene_ambient + diffuse + specular;
	f_color.a = 1.0;
	//ambos hacen lo mismo
	//f_color = vec4(scene_ambient + diffuse + specular);
	gl_Position = modelToClipMatrix * vec4(v_position, 1.0);
	f_texCoord = v_texCoord;
}

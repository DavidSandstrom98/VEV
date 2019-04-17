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
	float fac = dot(n,l);//producto escalar entre la normal del vertice y la direccion de la luz
	fac = max(0.0,fac);//El maximo entre 0.0 y el factor para asegurar que no sale negativo
	return fac;
}

float specular_factor(const vec3 n,
					  const vec3 l,
					  const vec3 v,
					  float m) {
	//Producto escalar entre la normal del vertice y la direccion de la luz
	float NoL = dot(n, l);
	if(NoL <= 0.0) return 0.0;

	//r es el vector con el que sale el reflejo especular de la luz
	//v es el vector que va desde el vertice hasta la camara
	vec3 r = 2 * NoL * n - l;
	float RoV = dot(r, v);

	//Aplicar el shinninesh del objeto. Lo pulido que esta
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
	//Solo si hay aportacion difusa puede haber luz difusa y especular
	if(lam > 0.0){
		diffuse += lam * theMaterial.diffuse * theLights[i].diffuse;

		float especular = specular_factor(normal, lightDirection, viewDirection, theMaterial.shininess);
		if(especular > 0.0){
			specular += especular * theMaterial.specular * theLights[i].specular;
		}

	}

	
}

void point_light(const in int i,
				 const in vec3 position,
				 const in vec3 viewDirection,
				 const in vec3 normal,
				 inout vec3 diffuse, inout vec3 specular) {
	
	//Vector que va de la luz al vertice
	vec3 L = theLights[i].position.xyz - position;

	float dist = length(L);//Obtener la distancia a la que se encuentra la luz del vertice
	L = normalize(L);//NORMALIZAR LA DIRECCION DE LA LUZ AL VERTICE
	float AtenFac = theLights[i].attenuation[0] + theLights[i].attenuation[1]*dist + theLights[i].attenuation[2]*dist*dist;

	if(AtenFac < 0.00001){
		AtenFac = 1.0;
	}else{
		AtenFac = 1 / AtenFac;
	}

	float lam = lambert_factor(normal, L);
	//Igual que el la luz direccional. Si no hay difusa ni hay difusa ni especular
	if(lam > 0.0){
		diffuse += AtenFac * lam * theMaterial.diffuse * theLights[i].diffuse;
		float especular = specular_factor(normal, L, viewDirection, theMaterial.shininess);

		if(especular > 0.0){
			specular += AtenFac * especular * theMaterial.specular * theLights[i].specular;
		}
	}

}


// Note: no attenuation in spotlights
void spot_light(const in int i, //Id de la luz
				const in vec3 position,//Posicion del vertice
				const in vec3 viewDirection, //Direccion del vertice a la camara
				const in vec3 normal, //Normal del vertice
				inout vec3 diffuse, inout vec3 specular) {
	
	vec3 L = theLights[i].position.xyz - position;//vector del punto a la luz
	L = normalize(L);
	//Coseno del angulo entre el vector de direccion de la luz y el que va de la luz al vertice 
	float coseno = dot(-L, theLights[i].spotDir);
	//Comprobar que el vertice esta dentro del cono de vision
	if(coseno < 0.0 || coseno < theLights[i].cosCutOff) return;
	//Factor de atenuacion con el angulo de apertura
	float Cspot = pow( max( coseno, 0.0), theLights[i].exponent);
	float lam = lambert_factor(normal, L);

	if(lam > 0.0){
		diffuse += Cspot * lam * theMaterial.diffuse * theLights[i].diffuse;
		float especular = specular_factor(normal, L, viewDirection, theMaterial.shininess);

		if(especular > 0.0){
			specular += Cspot * especular * theMaterial.specular * theLights[i].specular;
		}
	}

}

void main() {

	//Vectores que haran de acumuladores de iluminacion difusa y especular
	vec3 diffuse = vec3(0.0);
	vec3 specular = vec3(0.0);

	//Posicion del vertice en coordenadas de la camara
	vec4 positionEye = modelToCameraMatrix * vec4(v_position, 1.0);

	//Vector desde el vertice a la camara NORMALIZADO
	vec3 viewDirection = vec3( (0.0, 0.0, 0.0, 1.0) - positionEye );
	viewDirection = normalize(viewDirection);

	//normal del vertice en coordenadas de la camara
	vec3 normal = vec3(modelToCameraMatrix * vec4(v_normal, 0.0)); 
	normal = normalize(normal);

	vec3 lightDirection;
	
	for(int i=0; i < active_lights_n; ++i) {
		if(theLights[i].position.w == 0.0) {
		  	//direction light
			//Vector de la luz invertido. YA ESTA EN COORDENADAS DE LA CAMARA  
			//Hay que asegurarse de que esta normalizado
			lightDirection = (-1.0)*theLights[i].position.xyz;
			lightDirection = normalize(lightDirection);

			direction_light(i, lightDirection, viewDirection, normal, diffuse, specular); 	
		} else {
		  	if (theLights[i].cosCutOff == 0.0) {
				// point light luz posicional
				point_light(i, positionEye.xyz, viewDirection, normal, diffuse, specular);
		  	} else {
				// spot light foco
				spot_light(i, positionEye.xyz, viewDirection, normal, diffuse, specular);
		 	}
		}
	}

	//Parte de el color relacionada con la animacion
	//f_color = vec4(1.0);
	/*float s = sin(u_time);
	float c = cos(u_time);
	float sc = 1-(s+c)/2;
	f_color = vec4(s, c, sc, 1);*/
	
	f_color.rgb = scene_ambient + diffuse + specular;
	f_color.a = 1.0;
	//ambos hacen lo mismo
	//f_color = vec4(scene_ambient + diffuse + specular);
	gl_Position = modelToClipMatrix * vec4(v_position, 1.0);
	f_texCoord = v_texCoord;
}

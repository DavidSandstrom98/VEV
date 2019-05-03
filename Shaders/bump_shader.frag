#version 120

uniform int active_lights_n; // Number of active lights (< MG_MAX_LIGHT)
uniform vec3 scene_ambient; // Scene ambient light

struct material_t {
	vec3  diffuse;
	vec3  specular;
	float alpha;
	float shininess;
};

struct light_t {
	vec4 position;    // Camera space
	vec3 diffuse;     // rgb
	vec3 specular;    // rgb
	vec3 attenuation; // (constant, lineal, quadratic)
	vec3 spotDir;     // Camera space
	float cosCutOff;  // cutOff cosine
	float exponent;
};

uniform light_t theLights[4];
uniform material_t theMaterial;

uniform sampler2D texture0;
uniform sampler2D bumpmap;

varying vec2 f_texCoord;
varying vec3 f_viewDirection;     // tangent space
varying vec3 f_lightDirection[4]; // tangent space
varying vec3 f_spotDirection[4];  // tangent space

//Sin cambios
float lambert_factor(const vec3 n, const vec3 l) {//Si es 0 no hay componente especular
	float fac = dot(n,l);//producto escalar entre la normal del vertice y la direccion de la luz
	fac = max(0.0,fac);//El maximo entre 0.0 y el factor para asegurar que no sale negativo
	return fac;
}

//Sin cambios
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

//Sin cambios
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

// Note: do not calculate the attenuation in point_lights
//
// Note: this function uses 'lightDirection', the direction of the light in
//    tangent space, instead of 'position' (as used in pervertex and
//    perfragment)

//CUIDADO tiene un parametro menos porque no usa la posicion
void point_light(const in int i,
				 const in vec3 viewDirection,
				 const in vec3 normal,
				 inout vec3 diffuse, inout vec3 specular) {

	//Vector que va de la luz al vertice
	//vec3 L = theLights[i].position.xyz - position;
	vec3 L = normalize(f_lightDirection[i]);

	float lam = lambert_factor(normal, L);
	//Igual que el la luz direccional. Si no hay difusa ni hay difusa ni especular
	if(lam > 0.0){
		diffuse += lam * theMaterial.diffuse * theLights[i].diffuse;
		float especular = specular_factor(normal, L, viewDirection, theMaterial.shininess);

		if(especular > 0.0){
			specular += especular * theMaterial.specular * theLights[i].specular;
		}
	}
}

// Note: no attenuation in spotlights

void spot_light(const in int i,
				const in vec3 viewDirection,
				const in vec3 normal,
				inout vec3 diffuse, inout vec3 specular) {

	//vec3 L = theLights[i].position.xyz - position;//vector del punto a la luz
	vec3 L = f_lightDirection[i];
	L = normalize(L);
	//Coseno del angulo entre el vector de direccion de la luz y el que va de la luz al vertice 
	float coseno = dot(-L, normalize(f_spotDirection[i]));
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

	//Vector desde el vertice a la camara NORMALIZADO
	vec3 viewDirection = normalize(f_viewDirection);

	//normal del vertice en coordenadas de la camara
	vec4 mapaBump = texture2D(bumpmap, f_texCoord);
	vec3 normal = normalize(mapaBump.rgb * 2.0 - 1.0);

	vec3 lightDirection;

	for(int i = 0; i < active_lights_n; ++i) {
		if(theLights[i].position.w == 0.0) {
		  	//direction light
			//Vector de la luz invertido.
			lightDirection = normalize(-f_lightDirection[i].xyz);

			direction_light(i, lightDirection, viewDirection, normal, diffuse, specular); 	
		} else {
		  	if (theLights[i].cosCutOff == 0.0) {
				// point light luz posicional
				point_light(i, viewDirection, normal, diffuse, specular);
		  	} else {
				// spot light foco
				spot_light(i, viewDirection, normal, diffuse, specular);
		 	}
		}
	}

	vec4 f_color;
	f_color.rgb = scene_ambient + diffuse + specular;
	f_color.a = 1.0;

	vec4 f_texColor = texture2D(texture0, f_texCoord);
	gl_FragColor = f_color * f_texColor;
}
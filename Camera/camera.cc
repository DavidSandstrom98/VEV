#include <cstdio>
#include <cmath>
#include "camera.h"
#include "constants.h"
#include "tools.h"
#include "intersect.h"

Camera::Camera() :
	m_viewTrfm(new Trfm3D),
	m_projTrfm(new Trfm3D),
	m_lastE(Vector3::ZERO),
	m_lastAt(0.0f, 0.0f, -1.0f),
	m_lastUp(0.0f, 0.0f, -1.0f),
	m_E(Vector3::ZERO),
	m_At(0.0f, 0.0f, -1.0f),
	m_Up(0.0f, 1.0f, 0.0f),
	m_R(Vector3::UNIT_X),
	m_U(Vector3::UNIT_Y),
	m_D(Vector3::UNIT_Z) {
	for(int i = 0; i < MAX_CLIP_PLANES; ++i)
		m_fPlanes[i] = new Plane();
}

Camera::~Camera() {
	delete m_viewTrfm;
	delete m_projTrfm;
	for (int i=0; i<MAX_CLIP_PLANES; ++i)
		delete m_fPlanes[i];
}

void Camera::goLast() {
	lookAt(m_lastE, m_lastAt, m_lastUp);
}

PerspectiveCamera::PerspectiveCamera() :
	m_fovy(60.0f * Constants::degree_to_rad) {
	m_type = perspective;
	m_near = 0.1f;
	m_far = 1500.0f;
	m_aspectRatio = 1.0f;
	m_left = -0.034451;
	m_bottom = -0.026795;
	m_right = 0.034451;
	m_top = 0.026795;
	m_projTrfm->setFrustum(m_left, m_right,
						   m_bottom, m_top,
						   m_near, m_far);
}

PerspectiveCamera::~PerspectiveCamera() {}

void PerspectiveCamera::init(float fovy, float aspect,
							 float near, float far) {
	m_fovy = fovy;
	m_aspectRatio = aspect;
	m_near = near;
	m_far = far;
	updateProjection();
}

OrthographicCamera::OrthographicCamera() {
	m_type = orthographic;
	m_near = 0.1f;
	m_far = 1500.0f;
	m_left = -0.034451;
	m_bottom = -0.026795;
	m_right = 0.034451;
	m_top = 0.026795;
	m_projTrfm->setOrtho(m_left, m_right,
						 m_bottom, m_top,
						 m_near, m_far);
}

OrthographicCamera::~OrthographicCamera() {}

void OrthographicCamera::init(float left, float right,
							  float bottom, float top,
							  float near, float far) {
	m_left = left;
	m_right = right;
	m_bottom = bottom;
	m_top = top;
	m_near = near;
	m_far = far;
	updateProjection();
}

////////////////////////////////////////////////////////////7
// Projection stuff

void OrthographicCamera::onResize(int w, int h) {}
void PerspectiveCamera::onResize(int w, int h ) {
	if( h > 0 )
		m_aspectRatio = (float) w / (float) h;    // width/height
	else
		m_aspectRatio = 1.0f;
	updateProjection();
}

// zoom camera

void Camera::zoom(float angle) {}
void PerspectiveCamera::zoom(float angle) {
	m_fovy = angle;
	updateProjection();
}

float Camera::getZoom() const { return 0.0f; }
float PerspectiveCamera::getZoom() const { return m_fovy; }

void OrthographicCamera::updateProjection() {
	m_projTrfm->setOrtho(m_left, m_right,
						 m_bottom, m_top,
						 m_near, m_far);
	updateFrustumPlanes();
}

// @@ TODO:
// * Given (near, far, aspectRatio, fovy), calculate the values of
//   frustum (top, bottom, right, left).
// * Also, update projection matrix (projTrfm)

//Calcula los valores relacionados con la proyeccion de la camara
void PerspectiveCamera::updateProjection() {
	this->m_top = this->m_near * tanf(this->m_fovy/2);
	this->m_bottom = -this->m_top;
	this->m_right = this->m_aspectRatio * this->m_top;
	this->m_left = -this->m_right;

	//Una vez calculados los valores hay que actualizar el frustrum para realizar la proyeccion
	this->m_projTrfm->setFrustum(m_left, m_right, m_bottom, m_top, m_near, m_far);
	
	// Leave next line as-is
	updateFrustumPlanes();
}



////////////////////////////////////////////////////////////7
// View transformation stuff

void Camera::setViewTrfm() {
	m_viewTrfm->setWorld2Local(m_E, m_R, m_U, m_D);
	updateFrustumPlanes();
}

// @@ TODO:
/**
 * Calculate (R, U, D) vectors of the camera frame given (E, At, Up)
 *
 */

void Camera::updateFrame () {
	Vector3 F = this->m_E - this->m_At;//Vector hacia donde mira la camara
	F.normalize();
	//Calculo del vector que va hacia la derecha en coordenadas del mundo.
	//Producto vectorial entre 
	this->m_R = this->m_Up;//La verticalidad
	this->m_R.normalize();//Y el vector de mira
	this->m_R.cross(F);

	//Calculo del vector y de la camara en coordenadas del mundo
	//Producto vectorial entre
	this->m_U = F;//Vector de mira
	this->m_U.cross(this->m_R);//Vector derecha de la camara

	this->m_D = F;//Vector de mira

	// leave next line as-is
	setViewTrfm();
}

void  Camera::lookAt(const Vector3 & E,
					 const Vector3 & at,
					 const Vector3 & up) {
	m_E = E;
	m_At= at;
	m_Up = up;
	m_Up.normalize();
	Vector3 VA(at - E);
	if (VA.isZero()) {
		fprintf(stderr, "[W] while setting camera (%s): V & A are too near\n", m_name.size() ? m_name.c_str() : "noname" );
		m_At += Vector3(1.0f, 1.0f, 1.0f);
		VA += Vector3(1.0f, 1.0f, 1.0f);
	}
	VA.normalize();
	// Check if VA & up are colinear
	if( fabs( VA.dot(m_Up) ) > 1.0f - Constants::angle_epsilon ) {
		fprintf(stderr, "[W] while setting camera (%s): up is nearly parallel to VA\n", m_name.size() ? m_name.c_str() : "noname" );
		m_Up[0] += 1.0f;
		if(fabs( VA.dot(m_Up) ) > 1.0f - Constants::angle_epsilon ) // last check
			m_Up[1] += 1.0;
	}
	m_lastE = m_E;
	m_lastAt = m_At;
	m_lastUp = m_Up;
	updateFrame();
}

////////////////////////////////////////////////////////////7
// get/set

const std::string & Camera::getName() const { return m_name; };
void Camera::setName(const std::string & n) { m_name = n; };

void Camera::setFar(float far) { m_far = far; updateProjection(); }
float Camera::getFar() const { return m_far; }
void Camera::setNear(float near) { m_near = near; updateProjection(); }
float Camera::getNear() const { return m_near; }
Vector3 Camera::getPosition() const { return m_E; }
Vector3 Camera::getDirection() const { return -1.0f * m_D; }

////////////////////////////////////////////////
// trfm transformations

const Trfm3D *Camera::viewTrfm() const { return m_viewTrfm;; }
const Trfm3D *Camera::projectionTrfm() const {return m_projTrfm; }
void Camera::viewTrfmGL(float *gmatrix) const { m_viewTrfm->getGLMatrix(gmatrix); }
void Camera::projectionTrfmGL(float *gmatrix) const  { m_projTrfm->getGLMatrix(gmatrix); }

////////////////////////////////////////////////
// Movement

// @@ TODO:
// Move the camera "step" units ahead. Fly mode.
//
// thisCamera         -> the camera
// step               -> number of units to fly (can be negative)

/**
 * Hay que actualizar la posicion de la camara y la posicion de mira.
 * Tener cuidado porque m_D va hacia atras.
 * Esta funcion te mueve en la posicion de mira.
 * Actualiza la posicion en los ejes que sea necesario (x, y, z)
 **/
void Camera::fly(float step) {
	//Actualizar la posicion de la camara y la posicion a la que miro.
	//Cuidado que m_D va hacia atras. Actualizo la (x y z)
	this->m_E -= step * m_D;
	this->m_At -= step * m_D;

	setViewTrfm();
}

// @@ TODO:
// Move the camera "step" units ahead. Walk mode.
//
// thisCamera         -> the camera
// step               -> number of units to walk (can be negative)

/**
 * Hay que actualizar la posicion de la camara y la posicion de mira.
 * Tener cuidado porque m_D va hacia atras.
 * Esta funcion aunque mires para arriba y quieras ir hacia arriba ni subes ni bajas.
 * Solo actualiza la posicion en los ejes (x, z)
 **/ 
void Camera::walk(float step) {

	Vector3 mover(this->m_D.x(), 0, this->m_D.z());
	mover *=step;
	this->m_E -= mover;//Punto de mira
	this->m_At -= mover;//Punto en el que me encuentro
	setViewTrfm();
}

void  Camera::panX(float step) {
	m_E += step * m_R;
	m_At += step * m_R;
	setViewTrfm();

}

void  Camera::panY(float step) {
	m_E += step * m_U;
	m_At += step * m_U;
	setViewTrfm();

}

void  Camera::pitch(float angle) {
	static Trfm3D T;
	T.setRotAxis(m_R, m_E, angle);
	m_At = T.transformPoint(m_At);
	m_Up = T.transformVector(m_Up);
	updateFrame();
}

void  Camera::yaw(float angle) {
	static Trfm3D T;
	T.setRotAxis(m_U, m_E, angle);
	m_At = T.transformPoint(m_At);
	m_Up = T.transformVector(m_Up);
	updateFrame();
}

void  Camera::roll(float angle) {
	static Trfm3D T;
	T.setRotAxis(m_D, m_E, -angle);
	m_At = T.transformPoint(m_At);
	m_Up = T.transformVector(m_Up);
	updateFrame();
}

// Rotate camera around world's Y axis

void Camera::viewYWorld(float angle) {
	static Trfm3D T;
	T.setRotAxis(Vector3::UNIT_Y, m_E, angle);
	m_At = T.transformPoint(m_At);
	m_Up = T.transformVector(m_Up);
	updateFrame();
}


void  Camera::arcOverUnder(float angle) {
	static Trfm3D T;
	T.setRotAxis(m_R, m_At, angle);
	m_E = T.transformPoint(m_E);
	m_Up = T.transformVector(m_Up);
	updateFrame();
}

void  Camera::arcLeftRight(float angle) {
	static Trfm3D T;
	T.setRotAxis(Vector3::UNIT_Y, m_At, angle);
	m_E = T.transformPoint(m_E);
	m_Up = T.transformVector(m_Up);
	updateFrame();
}
/**
 * Retorna: -1 en caso de que el objeto este completamente dentro del frustum
 * 			 0 en caso de que se encuentre intersectando alguno de los 6 planos
 * 			 1 en caso de que se encuentre fuera del frustum
 **/
int Camera::checkFrustum(const BBox *theBBox,
						 unsigned int *planesBitM) {
	
	int resul;
	int intersecta = -1;//"Suponemos" que el objeto esta completamente dentro del frustum. De no estarlo este valor se modificara
	
	for(int i = 0; i < 6; i++)
	{
		resul = BBoxPlaneIntersect(theBBox, this->m_fPlanes[i]);
		if(resul == IINTERSECT)
		{
			intersecta = 0; //Caso en que el BBox se encuentra a medias
		}
		else if(resul == +IREJECT)
		{
			return 1; //Totalmente fuera del frustum			
		}
	}

	//*planesBitM = 2;//Cantidad de planos con los que intersecta. No es necesario
	return intersecta; 
}

/////////////////////////////////////////////////////////////////////////////////////
// No tocar a partir de aqui

void Camera::updateFrustumPlanes() {

	static GLfloat M[16];
	Plane *p;

	// V: view transformation
	// P: perspective transformation
	// T = P*V

	Trfm3D T(*m_projTrfm); // T = P;
	T.add(m_viewTrfm); // T = P*V
	T.getGLMatrix(M);

	// Extract the planes parameters

	// left plane
	p = m_fPlanes[0];

	p->m_n[0] = -M[3]  - M[0];  // -(m_41 + m_11)
	p->m_n[1] = -M[7]  - M[4];  // -(m_42 + m_12)
	p->m_n[2] = -M[11] - M[8];  // -(m_43 + m_13)
	p->m_d =     M[15] + M[12]; //  (m_44 + m_14), because d in plane is really (-d)
	p->m_isNorm = false;

	// right plane
	p = m_fPlanes[1];
	p->m_n[0] = -M[3]  + M[0];  // -(m_41 - m_11)
	p->m_n[1] = -M[7]  + M[4];  // -(m_42 - m_12)
	p->m_n[2] = -M[11] + M[8];  // -(m_43 - m_13)
	p->m_d =     M[15] - M[12]; //  (m_44 - m_14) because d in plane is really (-d)
	p->m_isNorm = false;

	// bottom plane
	p = m_fPlanes[2];
	p->m_n[0] = -M[3]  - M[1];  // -(m_41 + m_21)
	p->m_n[1] = -M[7]  - M[5];  // -(m_42 + m_22)
	p->m_n[2] = -M[11] - M[9];  // -(m_43 + m_23)
	p->m_d =     M[15] + M[13]; //  (m_44 + m_24) because d in plane is really (-d)
	p->m_isNorm = false;

	// top plane
	p = m_fPlanes[3];
	p->m_n[0] = -M[3]  + M[1];   // -(m_41 - m_21)
	p->m_n[1] = -M[7]  + M[5];   // -(m_42 - m_22)
	p->m_n[2] = -M[11] + M[9];   // -(m_43 - m_23)
	p->m_d =     M[15] - M[13];  //  (m_44 - m_24) because d in plane is really (-d)
	p->m_isNorm = false;

	// near plane
	p = m_fPlanes[4];
	p->m_n[0] = -M[3]  - M[2];  // -(m_41 + m_31)
	p->m_n[1] = -M[7]  - M[6];  // -(m_42 + m_32)
	p->m_n[2] = -M[11] - M[10]; // -(m_43 + m_33)
	p->m_d =     M[15] + M[14]; //  (m_44 + m_34) because d in plane is really (-d)
	p->m_isNorm = false;

	// far plane
	p = m_fPlanes[5];
	p->m_n[0] = -M[3]  + M[2];  // -(m_41 - m_31)
	p->m_n[1] = -M[7]  + M[6];  // -(m_42 - m_32)
	p->m_n[2] = -M[11] + M[10]; // -(m_43 - m_33)
	p->m_d =     M[15] - M[14]; //  (m_44 - m_34) because d in plane is really (-d)
	p->m_isNorm = false;
	// It is not neccesary to normailze the planes for frustum calculation
}

void Camera::print( ) {

	printf("View Point    : %.4f %.4f %.4f\n", m_E[0], m_E[1], m_E[2]);
	printf("Look At Point : %.4f %.4f %.4f\n", m_At[0], m_At[1], m_At[1]);
	printf("Up Vector     : %.4f %.4f %.4f\n", m_Up[0], m_Up[1], m_Up[2] );
	printf("Frame\n");
	printf("R : %.4f %.4f %.4f\n", m_R[0], m_R[1], m_R[2] );
	printf("U : %.4f %.4f %.4f\n", m_U[0], m_U[1], m_U[2] );
	printf("D : %.4f %.4f %.4f\n", m_D[0], m_D[1], m_D[2] );
	printf("View trfm:\n");
	m_viewTrfm->print();
}

void PerspectiveCamera::print( ) {
	printf("*** Perspective camera: %s\n", m_name.size() ? m_name.c_str() : "(noname)");
	Camera::print();
	printf("Fovy %.4f Near %.4f Far %.4f\n",  m_fovy * Constants::rad_to_degree,
		   m_near, m_far );
	printf("Ortho l,b,n min  %.4f %.4f %.4f\n", m_left, m_bottom, m_near);
	printf("      r,t,f max  %.4f %.4f %.4f\n", m_right, m_top, m_far);
	printf("Projection trfm:\n");
	m_projTrfm->print();
}

void OrthographicCamera::print( ) {
	printf("*** Orthographic camera: %s\n", m_name.size() ? m_name.c_str() : "(noname)");
	Camera::print();
	printf("Ortho l,b,n min  %.4f %.4f %.4f\n", m_left, m_bottom, m_near);
	printf("      r,t,f max  %.4f %.4f %.4f\n", m_right, m_top, m_far);
	printf("Projection trfm:\n");
	m_projTrfm->print();

}

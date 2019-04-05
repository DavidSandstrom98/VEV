#include "tools.h"
#include "avatar.h"
#include "scene.h"

Avatar::Avatar(const std::string &name, Camera * cam, float radius) :
	m_name(name), m_cam(cam), m_walk(false) {
	Vector3 P = cam->getPosition();
	m_bsph = new BSphere(P, radius);
}

Avatar::~Avatar() {
	delete m_bsph;
}

bool Avatar::walkOrFly(bool walkOrFly) {
	bool walk = m_walk;
	m_walk = walkOrFly;
	return walk;
}

// TODO:
//
// AdvanceAvatar: see if avatar can advance 'step' units.

bool Avatar::advance(float step)
{

	Node *rootNode = Scene::instance()->rootNode(); // root node of scene
	//Comprobar si estoy en modo walk o fly y actuar
	if (m_walk)
		m_cam->walk(step);
	else
		m_cam->fly(step);
	//Actualizar la posicion del avatar de la camara
	this->m_bsph->setPosition(this->m_cam->getPosition());
	//En caso de que haya colision, tengo que retornar la camara a donde estaba
	if (rootNode->checkCollision(this->m_bsph)!= 0)
	{
		if (m_walk)
			m_cam->walk(-step);
		else
			m_cam->fly(-step);
		return false;
	}
	return true;
}


void Avatar::leftRight(float angle) {
	if (m_walk)
		m_cam->viewYWorld(angle);
	else
		m_cam->yaw(angle);
}

void Avatar::upDown(float angle) {
	m_cam->pitch(angle);
}

void Avatar::print() const { }

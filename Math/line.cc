#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "line.h"
#include "constants.h"
#include "tools.h"

Line::Line() : m_O(Vector3::ZERO), m_d(Vector3::UNIT_Y) {}
Line::Line(const Vector3 & o, const Vector3 & d) : m_O(o), m_d(d) {}
Line::Line(const Line & line) : m_O(line.m_O), m_d(line.m_d) {}

Line & Line::operator=(const Line & line) {
	if (&line != this) {
		m_O = line.m_O;
		m_d = line.m_d;
	}
	return *this;
}

// @@ TODO: Set line to pass through two points A and B
//
// Note: Check than A and B are not too close!
void Line::setFromAtoB(const Vector3 & A, const Vector3 & B) {
	Vector3 res =  Vector3(B - A);
	if(res.isZero()){
		std::cout<<"Puntos demasiado proximos";
	}else{
		this->m_O = A;
		res.normalize();
		this->m_d = res;
	}
	
}

// @@ TODO: Give the point corresponding to parameter u
Vector3 Line::at(float u) const {
	Vector3 res = this->m_O + u * this->m_d;
	return res;
}

// @@ TODO: Calculate the parameter 'u0' of the line point nearest to P
//
// u0 = D*(P-O) / D*D , where * == dot product
//(Punto proyectado sobre la linea - origen)/vector de direccion
float Line::paramDistance(const Vector3 & P) const {
	float num = this->m_d.dot(P - this->m_O);
	float den = this->m_d.dot(this->m_d);

	if(den > Vector3::epsilon){
		return num/den;
	}else{
		std::cout<<"CUIDADO: El denominador ha resultado ser 0";
		return 0;
	}

}

// @@ TODO: Calculate the minimum distance 'dist' from line to P
//
// dist = ||P - (O + uD)||
// Where u = paramDistance(P)
float Line::distance(const Vector3 & P) const {
	float res = (P - (this->m_O + this->paramDistance(P) * this->m_d)).length();
	return res;
}

void Line::print() const {
	printf("O:");
	m_O.print();
	printf(" d:");
	m_d.print();
	printf("\n");
}

#include <cmath>
#include "intersect.h"
#include "constants.h"
#include "tools.h"

/* | algo           | difficulty | */
/* |----------------+------------| */
/* | BSPhereBSPhere |          1 | */
/* | BSPherePlane   |          2 | */
/* | BBoxBBox       |          3 | */
/* | BSphereBBox    |          5 | */
/* | BBoxPlane      |          5 | */


// @@ TODO: test if two BBoxes intersect.
//! Returns :
//    IINTERSECT intersect
//    IREJECT don't intersect
//Si se cumple alguno de los intervalos del teorema intersectan
int  BBoxBBoxIntersect(const BBox *bba, const BBox *bbb ) {
	if( (bba->m_min.x() > bbb->m_max.x() || bbb->m_min.x() > bba->m_max.y()) || 
		(bba->m_min.y() > bbb->m_max.y() || bbb->m_min.y() > bba->m_max.y()) || 
		(bba->m_min.z() > bbb->m_max.z() || bbb->m_min.z() > bba->m_max.z())){
		return IREJECT;
	}else{
		return IINTERSECT;
	}
}

// @@ TODO: test if a BBox and a plane intersect.
//! Returns :
//   +IREJECT outside
//   -IREJECT inside
//    IINTERSECT intersect

int  BBoxPlaneIntersect (const BBox *theBBox, Plane *thePlane) {
	return IINTERSECT;
}


// @@ TODO: test if two BSpheres intersect.
//! Returns :
//    IREJECT don't intersect
//    IINTERSECT intersect
//Comprobar si la distancia entre centros es menor que la suma de los radios.
int BSphereBSphereIntersect(const BSphere *bsa, const BSphere *bsb)
{
	Vector3 dist(bsa->m_centre - bsb->m_centre);
	if(dist.lengthSquare() <= pow(bsa->m_radius + bsb->m_radius, 2.0)){
		return IINTERSECT;
	}else{
		return IREJECT;
	}
}

// @@ TODO: test if a BSpheres intersects a plane.
//! Returns :
//   +IREJECT outside
//   -IREJECT inside
//    IINTERSECT intersect
//Comprobar si la distancia del centro al plano es menor que el radio
int BSpherePlaneIntersect(const BSphere *bs, Plane *pl) {
	float dist = pl->distance(bs->m_centre);
	if(dist > bs->m_radius){
		return IREJECT;
	}else{
		return IINTERSECT;
	}
}

// @@ TODO: test if a BSpheres intersect a BBox.
//! Returns :
//    IREJECT don't intersect
//    IINTERSECT intersect

int BSphereBBoxIntersect(const BSphere *sphere, const BBox *box) {

	if(box->m_min.x() >= sphere->m_centre.x() &&  box->m_max.x() <= sphere->m_centre.x() && 
	   box->m_min.y() >= sphere->m_centre.y() &&  box->m_max.y() <= sphere->m_centre.y() && 
	   box->m_min.z() >= sphere->m_centre.z() &&  box->m_max.z() <= sphere->m_centre.z()){
		   return IINTERSECT;
	}else{
		Vector3 *vertices[8];
		vertices[0] = new Vector3(box->min.x(), box->min.y(), box->min.z()); 
		vertices[1] = new Vector3(box->min.x(), box->min.y(), box->max.z()); 
		vertices[2] = new Vector3(box->min.x(), box->max.y(), box->min.z()); 
		vertices[3] = new Vector3(box->min.x(), box->max.y(), box->max.z()); 
		vertices[4] = new Vector3(box->max.x(), box->min.y(), box->min.z()); 
		vertices[5] = new Vector3(box->min.x(), box->min.y(), box->max.z()); 
		vertices[6] = new Vector3(box->min.x(), box->max.y(), box->min.z()); 
		vertices[7] = new Vector3(box->min.x(), box->max.y(), box->max.z()); 

		
	}
	return IREJECT;
	
}


int IntersectTriangleRay(const Vector3 & P0,
						 const Vector3 & P1,
						 const Vector3 & P2,
						 const Line *l,
						 Vector3 & uvw) {
	Vector3 e1(P1 - P0);
	Vector3 e2(P2 - P0);
	Vector3 p(crossVectors(l->m_d, e2));
	float a = e1.dot(p);
	if (fabs(a) < Constants::distance_epsilon) return IREJECT;
	float f = 1.0f / a;
	// s = l->o - P0
	Vector3 s(l->m_O - P0);
	float lu = f * s.dot(p);
	if (lu < 0.0 || lu > 1.0) return IREJECT;
	Vector3 q(crossVectors(s, e1));
	float lv = f * q.dot(l->m_d);
	if (lv < 0.0 || lv > 1.0) return IREJECT;
	uvw[0] = lu;
	uvw[1] = lv;
	uvw[2] = f * e2.dot(q);
	return IINTERSECT;
}

/* IREJECT 1 */
/* IINTERSECT 0 */

const char *intersect_string(int intersect) {

	static const char *iint = "IINTERSECT";
	static const char *prej = "IREJECT";
	static const char *mrej = "-IREJECT";
	static const char *error = "IERROR";

	const char *result = error;

	switch (intersect) {
	case IINTERSECT:
		result = iint;
		break;
	case +IREJECT:
		result = prej;
		break;
	case -IREJECT:
		result = mrej;
		break;
	}
	return result;
}

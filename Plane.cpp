/*
 * Plane.cpp
 *
 *  Created on: Nov 14, 2018
 *      Author: smithcg
 */

#include "Plane.h"

#include <iostream>
#include "Triangle.h"
#include <glm/glm.hpp>

Plane::Plane(glm::vec3 point0, glm::vec3 orientation) {
	p0 = point0;
	normal = glm::normalize(orientation);
}


Plane::~Plane() {
	// TODO Auto-generated destructor stub
}

float Plane::intersect(Ray currentRay){
	glm::vec3 o = currentRay.origin;	//origin
	glm::vec3 d = currentRay.direction;		//direction
	glm::vec3 q = p0;						//point on plain
	glm::vec3 n = normal;					//normal

	float t = glm::dot((q-o),n)/glm::dot(d,n);

	return (t);
}

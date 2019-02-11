/*
 * Ray.cpp
 *
 *  Created on: Nov 8, 2018
 *      Author: smithcg
 */

#include "Ray.h"
#include <glm/vec3.hpp>

Ray::Ray(glm::vec3 o, glm::vec3 d) {
	origin = o;
	direction = d;

	intersect = -1;
	reflectionCounter = 0;
}

Ray::~Ray() {
}



glm::vec3 Ray::evaluate(float t){
	return origin + t*direction;
}

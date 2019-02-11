/*
 * Sphere.h
 *
 *  Created on: Nov 8, 2018
 *      Author: smithcg
 */

#ifndef SPHERE_H_
#define SPHERE_H_

#include <glm/glm.hpp>
#include "Ray.h"
#include "Object.h"

class Sphere: public Object {
public:
	Sphere(glm::vec3 newCenter, float newRadius);
	virtual ~Sphere();

	glm::vec3 center;
	float radius;

	float intersect(Ray currentRay);
};

#endif /* SPHERE_H_ */

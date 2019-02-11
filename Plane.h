/*
 * Plane.h
 *
 *  Created on: Nov 14, 2018
 *      Author: smithcg
 */

#ifndef PLANE_H_
#define PLANE_H_

#include <glm/glm.hpp>
#include "Ray.h"
#include "Object.h"

class Plane: public Object {
public:
	Plane(glm::vec3 point0, glm::vec3 normal);
	virtual ~Plane();

	glm::vec3 p0;

	float intersect(Ray currentRay);

	glm::vec3 normal;
};

#endif /* PLANE_H_ */

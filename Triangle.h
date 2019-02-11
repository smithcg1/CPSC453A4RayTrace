/*
 * Triangle.h
 *
 *  Created on: Nov 14, 2018
 *      Author: smithcg
 */

#ifndef TRIANGLE_H_
#define TRIANGLE_H_

#include <glm/glm.hpp>
#include "Ray.h"
#include "Object.h"

class Triangle: public Object {
public:
	Triangle(glm::vec3 point0, glm::vec3 point1, glm::vec3 point2);
	virtual ~Triangle();

	glm::vec3 p0;
	glm::vec3 p1;
	glm::vec3 p2;

	float intersect(Ray currentRay);

	glm::vec3 normal;
};

#endif /* TRIANGLE_H_ */

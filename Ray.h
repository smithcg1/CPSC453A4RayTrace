
/*
 * Ray.h
 *
 *  Created on: Nov 8, 2018
 *      Author: smithcg
 */

#ifndef RAY_H_
#define RAY_H_

#include <glm/vec3.hpp>
#include "Object.h"

class Ray {
public:
	Ray(glm::vec3 o, glm::vec3 d);
	virtual ~Ray();

	glm::vec3 evaluate(float t);

	glm::vec3 origin;
	glm::vec3 direction;

	glm::vec3 colour;

	Object closestObject;
	float intersect;

	glm::vec3 normal;
	glm::vec3 collisionPoint;

	int reflectionCounter;
private:


};

#endif /* RAY_H_ */

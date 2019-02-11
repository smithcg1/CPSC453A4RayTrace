/*
 * Light.h
 *
 *  Created on: Nov 14, 2018
 *      Author: smithcg
 */

#ifndef LIGHT_H_
#define LIGHT_H_

#include <glm/glm.hpp>
#include "Object.h"

class Light {
public:
	Light(glm::vec3 source);
	virtual ~Light();

	glm::vec3 position;

	Object test;
};

#endif /* LIGHT_H_ */

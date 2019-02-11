/*
 * Light.cpp
 *
 *  Created on: Nov 14, 2018
 *      Author: smithcg
 */

#include "Light.h"

Light::Light(glm::vec3 source) {
	position = source;
}

Light::~Light() {
}


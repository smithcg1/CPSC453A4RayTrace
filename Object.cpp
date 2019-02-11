/*
 * Object.cpp
 *
 *  Created on: Nov 8, 2018
 *      Author: smithcg
 */

#include "Object.h"
#include <glm/glm.hpp>
#include <iostream>


Object::Object() {
}

Object::~Object() {
}

//ka ia + sum( kd (L.N) id + ks (R.V)^P is)
//kd: diffuse %		L:light							N:normal	id: diffuse colour
//ks: specular %	R:reflected vector(normalized)	V:view		P:				is: specular colour

void Object::setProperties(glm::vec3 newColour){
	colour = newColour;
}

void Object::setProperties(float newka, glm::vec3 newia, float newkd, glm::vec3 newid, float newks, glm::vec3 newis, float newphi, float newalpha, float newbeta){
	ka = newka;
	ia = newia;
	kd = newkd;
	id = newid;
	ks = newks;
	is = newis;
	phi = newphi;

	alpha = newalpha;
	beta = newbeta;
}




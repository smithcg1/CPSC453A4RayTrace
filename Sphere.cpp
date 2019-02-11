/*
 * Sphere.cpp
 *
 *  Created on: Nov 8, 2018
 *      Author: smithcg
 */

#include "Sphere.h"
#include <glm/glm.hpp>

#include <iostream>
#include <string>


Sphere::Sphere(glm::vec3 newCenter, float newRadius) {
	center = newCenter;
	radius = newRadius;
}

Sphere::~Sphere() {
}



float Sphere::intersect(Ray currentRay){
	float t;
	float collision1;
	float collision2;

	glm::vec3 ro = currentRay.origin;
	glm::vec3 d = currentRay.direction;
	glm::vec3 rs = center;
	float R = radius;

	/*
	std::cout << ro.x << " " << ro.y << " " << ro.z <<std::endl;
	std::cout << d.x << " " << d.y << " " << d.z <<std::endl;
	std::cout << rs.x << " " << rs.y << " " << rs.z <<std::endl;
	std::cout << R <<std::endl<<std::endl;
*/

	float A = glm::dot(d,d);
	float B = 2*glm::dot(d,(ro-rs));
	float C = glm::dot((ro-rs),(ro-rs))-pow(R,2);

	/*
	std::cout << A <<std::endl;
	std::cout << B <<std::endl;
	std::cout << C <<std::endl;
*/

	collision1 = ((-B + sqrt(pow(B,2)-(4*A*C)))/(2*A));
	collision2 = ((-B - sqrt(pow(B,2)-(4*A*C)))/(2*A));


	float collisionError = 0.001;

	//If both collisions are valid, check which is smaller
	if (collisionError<collision1 && collisionError<collision2){
		if(collision1 < collision2){
			t = collision1;
		}
		else{
			t = collision2;
		}
	}
	//If only collision1 is positive, return collision1
	else if (collisionError < collision1){
		t = collision1;
	}
	//If only collision2 is positive, return collision2
	else if (collisionError < collision2){
		t = collision2;
	}
	//Else both are negative
	else{
		t = -1;
	}

	return t;
}

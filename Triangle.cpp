/*
 * Triangle.cpp
 *
 *  Created on: Nov 14, 2018
 *      Author: smithcg
 */

#include <iostream>
#include "Triangle.h"
#include <glm/glm.hpp>

Triangle::Triangle(glm::vec3 point0, glm::vec3 point1, glm::vec3 point2) {
	p0 = point0;
	p1 = point1;
	p2 = point2;

	//normal = glm::normalize(glm::cross(p1-p0, p2-p1));
	normal = glm::normalize(glm::cross(p0-p1, p2-p1));
	//std::cout << "x:" << normal.x << " y:" <<  normal.y << " z:" << normal.z << std::endl;
}

Triangle::~Triangle() {
}


float Triangle::intersect(Ray currentRay){
	glm::vec3 d = currentRay.direction;
	glm::vec3 e1 = p1 - p0;
	glm::vec3 e2 = p2 - p0;
	glm::vec3 s = currentRay.origin - p0;

	float detdee = glm::determinant(glm::mat3(-d,e1,e2));
	float detsee = glm::determinant(glm::mat3(s,e1,e2));
	float detdse = glm::determinant(glm::mat3(-d,s,e2));
	float detdes = glm::determinant(glm::mat3(-d,e1,s));

	glm::vec3 resultVector = (1/detdee)*glm::vec3(detsee,detdse,detdes);

	float t = resultVector[0];
	float u = resultVector[1];
	float v = resultVector[2];

	//std::cout << "t: " << t << "u: " << u << "v: " << v <<std::endl;

	//If point is behind camera or there is no intersection
	if ((t < 0) || !(0<=u && u<=1) || !(0<=v && v<=1) || !(0<=(u+v) && (u+v)<=1)){
		t = -1;
	}

	return (t);
}



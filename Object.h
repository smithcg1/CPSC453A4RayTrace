/*
 * Object.h
 *
 *  Created on: Nov 8, 2018
 *      Author: smithcg
 */

#ifndef OBJECT_H_
#define OBJECT_H_

#include <glm/glm.hpp>

class Object {
public:
	Object();
	virtual ~Object();
	void setProperties(glm::vec3 newColour);
	void setProperties(float ka, glm::vec3 ia, float kd, glm::vec3 id, float ks, glm::vec3 is, float phi, float newalpha, float newbeta);


	glm::vec3 colour;
	float alpha;
	float beta;

	float ka;
	glm::vec3 ia;

	float kd;
	glm::vec3 id;
	float ks;
	glm::vec3 is;
	float phi;
private:


};



#endif /* OBJECT_H_ */

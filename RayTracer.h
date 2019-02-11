/*
 * RayTracer.h
 *
 *  Created on: Nov 16, 2018
 *      Author: smithcg
 */

#ifndef RAYTRACER_H_
#define RAYTRACER_H_

#include "imagebuffer.h"
#include "Sphere.h"
#include "Triangle.h"
#include "Plane.h"
#include <glm/glm.hpp>

class RayTracer {
public:
	RayTracer();
	virtual ~RayTracer();


	void createImage();
	void changeFOV(float change);
	float fov;

	void generateObjects(int scene);

    ImageBuffer image;

    Ray collisionCheck(Ray ray);
    Ray collisionCheckLight(Ray ray, float maxIntersect);

	glm::vec3 collide(Ray ray, glm::vec3 lightSource);
	glm::vec3 colourCalculation(Object hitObject, glm::vec3 L, glm::vec3 N, glm::vec3 R, glm::vec3 V);

private:
    std::vector<Sphere> sphereObjects;
    std::vector<Triangle> triangleObjects;
    std::vector<Plane> planeObjects;

    glm::vec3 lightSource;
};

#endif /* RAYTRACER_H_ */


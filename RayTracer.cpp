/*
 * RayTracer.cpp
 *
 *  Created on: Nov 8, 2018
 *      Author: smithcg
 */


#include "RayTracer.h"
#include <glm/glm.hpp>
#include <iostream>
#include "Ray.h"
#include "Object.h"
#include <math.h>

RayTracer::RayTracer() {
	fov = 90;
}

RayTracer::~RayTracer() {
}

void RayTracer::createImage(){

    image.Initialize();

	float n = image.Width();
	float h = glm::tan(glm::radians(fov/2));
	float f = 1;

	for (unsigned int i = 0; i < image.Width(); i++) {
		for (unsigned int j = 0; j < image.Height(); j++) {
			float xCoordinate = (-h/2) + (h/(2*n)) + ((i*h)/n);
			float yCoordinate = (-h/2) + (h/(2*n)) + ((j*h)/n);

			Ray pixelRay = Ray(glm::vec3(0.0,0.0,0.0), glm::normalize(glm::vec3(xCoordinate, yCoordinate, f)));
			pixelRay = collisionCheck(pixelRay);

			if (0<pixelRay.intersect){
				pixelRay.colour = collide(pixelRay, lightSource);
				image.SetPixel(i, j, pixelRay.colour);
			}
		}
	}
}


Ray RayTracer::collisionCheck(Ray ray){
	float newIntersect = -1;
	float minimum = 0.0001;

	for( int k = 0 ; k < sphereObjects.size() ; k++){
		newIntersect = sphereObjects[k].intersect(ray);
		if(minimum<newIntersect && (newIntersect<ray.intersect || (ray.intersect == -1))){
			ray.intersect = newIntersect;
			ray.closestObject = sphereObjects[k];
			ray.collisionPoint = ray.origin + (ray.intersect*ray.direction);
			ray.normal = glm::normalize(ray.collisionPoint - sphereObjects[k].center);
		}
	}

	for( int k = 0 ; k < triangleObjects.size() ; k++){
		newIntersect = triangleObjects[k].intersect(ray);
		if(minimum<newIntersect && (newIntersect<ray.intersect || (ray.intersect == -1))){
			ray.intersect = newIntersect;
			ray.closestObject = triangleObjects[k];
			ray.collisionPoint = ray.origin + (ray.intersect*ray.direction);
			ray.normal = triangleObjects[k].normal;
		}
	}

	for( int k = 0 ; k < planeObjects.size() ; k++){
		newIntersect = planeObjects[k].intersect(ray);
		if(minimum<newIntersect && (newIntersect<ray.intersect || (ray.intersect == -1))){
			ray.intersect = newIntersect;
			ray.closestObject = planeObjects[k];
			ray.collisionPoint = ray.origin + (ray.intersect*ray.direction);
			ray.normal = planeObjects[k].normal;
		}
	}

	return ray;
}


Ray RayTracer::collisionCheckLight(Ray ray, float maxIntersect){
	float newIntersect = -1;
	float minimum = 0.0001;

	for( int k = 0 ; k < sphereObjects.size() ; k++){
		newIntersect = sphereObjects[k].intersect(ray);
		if(minimum<newIntersect && (newIntersect<ray.intersect || (ray.intersect == -1)) && newIntersect<maxIntersect){
			ray.intersect = newIntersect;
		}
	}

	for( int k = 0 ; k < triangleObjects.size() ; k++){
		newIntersect = triangleObjects[k].intersect(ray);
		if(newIntersect<minimum){
			float radians = glm::acos(glm::dot(glm::normalize(ray.direction),glm::normalize(triangleObjects[k].normal)));
			if(radians > (M_PI/2)){
				//ray.intersect = 0.0;
			}
		}

		if(minimum<newIntersect && (newIntersect<ray.intersect || (ray.intersect == -1)) && newIntersect<maxIntersect){
			ray.intersect = newIntersect;
			//std::cout << "Triangle hit" <<std::endl;
		}
	}

	for( int k = 0 ; k < planeObjects.size() ; k++){
		newIntersect = planeObjects[k].intersect(ray);
		if(minimum<newIntersect && (newIntersect<ray.intersect || (ray.intersect == -1)) && newIntersect<maxIntersect){
			ray.intersect = newIntersect;
		}
	}
	return ray;
}



void RayTracer::changeFOV(float change){
	if ( 5 < fov+change && fov+change < 180){
		fov += change;
		std::cout << "FOV changed to: " << fov <<std::endl;
	}
}







glm::vec3 RayTracer::collide(Ray ray, glm::vec3 lightSource){
	if(ray.reflectionCounter >= 11){
		return glm::vec3(0.0,0.0,0.0);
	}


	Object hitObject = ray.closestObject;
	glm::vec3 colour = glm::vec3(0.0,0.0,0.0);

	glm::vec3 L = glm::normalize(lightSource - ray.collisionPoint);
	glm::vec3 N = ray.normal;
	glm::vec3 R = -L+(2*glm::dot(L,N))*N;
	glm::vec3 V = glm::normalize(glm::vec3(0.0,0.0,0.0) - ray.collisionPoint);

	float alpha = hitObject.alpha;
	float beta = hitObject.beta;

	glm::vec3 shaderColour = colourCalculation(hitObject, L, N, R, V);

	//Shadow checker
	float maxIntersect = glm::length(lightSource - ray.collisionPoint);
	Ray bouncedRay = Ray(ray.collisionPoint, L);
	//std::cout << "x:" << ray.collisionPoint.x << " y:" <<  ray.collisionPoint.y << " z:" << ray.collisionPoint.z << std::endl;
	bouncedRay = collisionCheckLight(bouncedRay, maxIntersect);
	if(bouncedRay.intersect != -1){
		shaderColour = glm::vec3(0.0,0.0,0.0);
	}

	//Ambient Light
	float ka = hitObject.ka;
	glm::vec3 id = hitObject.id;
	shaderColour = ka*id + shaderColour;


	//Reflection Calculator
	if(hitObject.alpha == 0 && hitObject.beta == 0){
		colour = shaderColour;
	}
	else{
		glm::vec3 d = ray.direction;
		glm::vec3 r = d - (2*(glm::dot(d,N))*N);			//r = d - 2(d.n)n   //r is the reflected direction vector
		Ray bounceRay = Ray(ray.collisionPoint, r);
		bounceRay.reflectionCounter = ray.reflectionCounter + 1;

		bounceRay = collisionCheck(bounceRay);

		if (bounceRay.intersect != -1){
			bounceRay.colour = collide(bounceRay, lightSource);
		}
		else{
			bounceRay.colour = glm::vec3(0.0,0.0,0.0);
		}

		glm::vec3 reflectionColour = bounceRay.colour;

		// alpha and beta should not both be present so either
		// alpha > 1 & beta == 0: (1-alpha)*shaderColour + (alpha*reflectionColour) + 0
		// beta > 1 & alpha == 0: 1*shaderColour + 0 + (beta*reflectionColour)
		colour = (1-hitObject.alpha)*shaderColour + (hitObject.alpha*reflectionColour) + (hitObject.beta*reflectionColour);
	}

	return colour;
}


glm::vec3 RayTracer::colourCalculation(Object hitObject, glm::vec3 L, glm::vec3 N, glm::vec3 R, glm::vec3 V){
	float ka = hitObject.ka;
	glm::vec3 ia = hitObject.ia;
	float kd = hitObject.kd;
	glm::vec3 id = hitObject.id;
	float ks = hitObject.ks;
	glm::vec3 is = hitObject.is;
	float phi = hitObject.phi;

	float LN = glm::dot(L,N);
	float RV = glm::dot(R,V);
	if (RV < 0){RV = 0;}

	return (kd*LN*id)+(ks*glm::pow(RV,phi)*is);
}





/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////Scenes//////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


//(float newka, glm::vec3 newia, float newkd, glm::vec3 newid, float newks, glm::vec3 newis, float newphi)
void RayTracer::generateObjects(int scene){
	sphereObjects.clear();
	triangleObjects.clear();
	planeObjects.clear();

	if (scene == 1){
		float ka = 0.5;
		glm::vec3 ia = glm::vec3(1.0, 1.0, 1.0);

		lightSource = glm::vec3(0.0,2.5,7.75);
//////////////////////////////////////////////////////
		Sphere gSphere = Sphere(glm::vec3(0.9,-1.925,6.69),0.825);
		gSphere.setProperties(ka, ia, 0.8, glm::vec3(0.3,0.3,0.3), 0.2, glm::vec3(0.3,0.3,0.3), 8, 0.6, 0);
//////////////////////////////////////////////////////
		Triangle bTriangle1 = Triangle(glm::vec3(-0.4,-2.75,9.55),glm::vec3(-0.93,0.55,8.51),glm::vec3(0.11,-2.75,7.98));
		bTriangle1.setProperties(ka, ia, 1, glm::vec3(0.1,0.5,0.6), 0, glm::vec3(1.0,0.0,0.0), 1, 0, 0.2);

		Triangle bTriangle2 = Triangle(glm::vec3(0.11,-2.75,7.98),glm::vec3(-0.93,0.55,8.51),glm::vec3(-1.46,-2.75,7.47));
		bTriangle2.setProperties(ka, ia, 1, glm::vec3(0.1,0.5,0.6), 0, glm::vec3(1.0,0.0,0.0), 1, 0, 0.2);

		Triangle bTriangle3 = Triangle(glm::vec3(-1.46,-2.75,7.47),glm::vec3(-0.93,0.55,8.51),glm::vec3(-1.97,-2.75,9.04));
		bTriangle3.setProperties(ka, ia, 1, glm::vec3(0.1,0.5,0.6), 0, glm::vec3(1.0,0.0,0.0), 1, 0, 0.2);

		Triangle bTriangle4 = Triangle(glm::vec3(-1.97,-2.75,9.04),glm::vec3(-0.93,0.55,8.51),glm::vec3(-0.4,-2.75,9.55));
		bTriangle4.setProperties(ka, ia, 1, glm::vec3(0.1,0.5,0.6), 0, glm::vec3(1.0,0.0,0.0), 1, 0, 0.2);
///////////////////////////////////////////////////////
		Triangle ceilingTriangle1 = Triangle(glm::vec3(2.75,2.75,10.5),glm::vec3(2.75,2.75,5.0),glm::vec3(-2.75,2.75,5.0));
		ceilingTriangle1.setProperties(ka, ia, 1, glm::vec3(0.9,0.9,0.9), 0, glm::vec3(1.0,0.0,0.0), 1, 0, 0);

		Triangle ceilingTriangle2 = Triangle(glm::vec3(-2.75,2.75,10.5),glm::vec3(2.75,2.75,10.5),glm::vec3(-2.75,2.75,5.0));
		ceilingTriangle2.setProperties(ka, ia, 1, glm::vec3(0.9,0.9,0.9), 0, glm::vec3(1.0,0.0,0.0), 1, 0, 0);
///////////////////////////////////////////////////////
		Triangle rWallTriangle1 = Triangle(glm::vec3(2.75,2.75,5.0),glm::vec3(2.75,2.75,10.5),glm::vec3(2.75,-2.75,10.5));
		rWallTriangle1.setProperties(ka, ia, 1, glm::vec3(0.0,0.5,0.0), 0, glm::vec3(1.0,0.0,0.0), 1, 0, 0);

		Triangle rWallTriangle2 = Triangle(glm::vec3(2.75,-2.75,5.0),glm::vec3(2.75,2.75,5.0),glm::vec3(2.75,-2.75,10.5));
		rWallTriangle2.setProperties(ka, ia, 1, glm::vec3(0.0,0.5,0.0), 0, glm::vec3(1.0,0.0,0.0), 1, 0, 0);
//////////////////////////////////////////////////////
		Triangle lWallTriangle1 = Triangle(glm::vec3(-2.75,-2.75,5.0),glm::vec3(-2.75,-2.75,10.5),glm::vec3(-2.75,2.75,10.5));
		lWallTriangle1.setProperties(ka, ia, 1, glm::vec3(0.5,0.0,0.0), 0, glm::vec3(1.0,0.0,0.0), 1, 0, 0);

		Triangle lWallTriangle2 = Triangle(glm::vec3(-2.75,2.75,5.0),glm::vec3(-2.75,-2.75,5.0),glm::vec3(-2.75,2.75,10.5));
		lWallTriangle2.setProperties(ka, ia, 1, glm::vec3(0.5,0.0,0.0), 0, glm::vec3(1.0,0.0,0.0), 1, 0, 0);
///////////////////////////////////////////////////////
		Triangle floorTriangle1 = Triangle(glm::vec3(2.75,-2.75,5.0),glm::vec3(2.75,-2.75,10.5),glm::vec3(-2.75,-2.75,10.5));
		floorTriangle1.setProperties(ka, ia, 1, glm::vec3(0.5,0.5,0.5), 0, glm::vec3(1.0,0.0,0.0), 1, 0, 0);

		Triangle floorTriangle2 = Triangle(glm::vec3(-2.75,-2.75,5.0),glm::vec3(2.75,-2.75,5.0),glm::vec3(-2.75,-2.75,10.5));
		floorTriangle2.setProperties(ka, ia, 1, glm::vec3(0.5,0.5,0.5), 0, glm::vec3(1.0,0.0,0.0), 1, 0, 0);
///////////////////////////////////////////////////////
		Plane backPlane = Plane(glm::vec3(0,0,10.5),glm::vec3(0.0,0.0,-1.0));
		backPlane.setProperties(ka, ia, 1.0, glm::vec3(0.6,0.6,0.6), 0.0, glm::vec3(0.0,0.0,0.0), 1, 0, 0);


		sphereObjects.push_back(gSphere);

		triangleObjects.push_back(bTriangle1);
		triangleObjects.push_back(bTriangle2);
		triangleObjects.push_back(bTriangle3);
		triangleObjects.push_back(bTriangle4);

		triangleObjects.push_back(ceilingTriangle1);
		triangleObjects.push_back(ceilingTriangle2);

		triangleObjects.push_back(rWallTriangle1);
		triangleObjects.push_back(rWallTriangle2);

		triangleObjects.push_back(lWallTriangle1);
		triangleObjects.push_back(lWallTriangle2);

		triangleObjects.push_back(floorTriangle1);
		triangleObjects.push_back(floorTriangle2);

		planeObjects.push_back(backPlane);

		std::cout << "Scene 1 generated" <<std::endl;
	}
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
	if (scene == 2){
		float ka = 0.5;
		glm::vec3 ia = glm::vec3(1.0, 1.0, 1.0);

		lightSource = glm::vec3(4.0,6.0,1.0);
//////////////////////////////////////////////
		Plane floorPlane = Plane(glm::vec3(0.0,-1.0,0.0),glm::vec3(0.0,1.0,0.0));
		floorPlane.setProperties(ka, ia, 1.0, glm::vec3(0.5,0.5,0.5), 0.0, glm::vec3(0.0,0.0,0.0), 1, 0, 0);

		Plane backPlane = Plane(glm::vec3(0.0,0.0,12.0),glm::vec3(0.0,0.0,-1.0));
		backPlane.setProperties(ka, ia, 0.3, glm::vec3(0.0,0.3,0.4), 0.5, glm::vec3(1.0,1.0,1.0), 1, 0, 0);
///////////////////////////////////////////////
		Sphere ySphere = Sphere(glm::vec3(1.0,-0.5,3.5),0.5);
		ySphere.setProperties(ka, ia, 0.85, glm::vec3(0.7,0.6,0.0), 0.4, glm::vec3(1.0,1.0,1.0), 12, 0, 0);

		Sphere gSphere = Sphere(glm::vec3(0.0,1.0,5.0),0.4);
		gSphere.setProperties(ka, ia, 1.0, glm::vec3(0.6,0.6,0.6), 0.55, glm::vec3(1.0,1.0,1.0), 128, 0.4, 0);

		Sphere pSphere = Sphere(glm::vec3(-0.8,-0.75,4.0),0.25);
		pSphere.setProperties(ka, ia, 1.0, glm::vec3(0.27,0.062,0.24), 0.4, glm::vec3(0.27,0.062,0.24), 32, 0, 0.2);
////////////////////////////////////////////////
		float gcdc = 0.4;
		float gcsc = 0.4;
		float gcPhi = 16;

		Triangle gcTriangle1 = Triangle(glm::vec3(0.0,-1.0,5.8),glm::vec3(0.0,0.6,5.0),glm::vec3(0.4,-1.0,5.693));
		gcTriangle1.setProperties(ka, ia, gcdc, glm::vec3(0.0,0.6,0.0), gcsc, glm::vec3(1.0,1.0,1.0), gcPhi, 0, 0);

		Triangle gcTriangle2 = Triangle(glm::vec3(0.4,-1.0,5.693),glm::vec3(0.0,0.6,5.0),glm::vec3(0.6928,-1.0,5.4));
		gcTriangle2.setProperties(ka, ia, gcdc, glm::vec3(0.0,0.6,0.0), gcsc, glm::vec3(1.0,1.0,1.0), gcPhi, 0, 0);

		Triangle gcTriangle3 = Triangle(glm::vec3(0.6928,-1.0,5.4),glm::vec3(0.0,0.6,5.0),glm::vec3(0.8,-1.0,5.0));
		gcTriangle3.setProperties(ka, ia, gcdc, glm::vec3(0.0,0.6,0.0), gcsc, glm::vec3(1.0,1.0,1.0), gcPhi, 0, 0);

		Triangle gcTriangle4 = Triangle(glm::vec3(0.8,-1.0,5.0),glm::vec3(0.0,0.6,5.0),glm::vec3(0.6928,-1.0,4.6));
		gcTriangle4.setProperties(ka, ia, gcdc, glm::vec3(0.0,0.6,0.0), gcsc, glm::vec3(1.0,1.0,1.0), gcPhi, 0, 0);

		Triangle gcTriangle5 = Triangle(glm::vec3(0.6928,-1.0,4.6),glm::vec3(0.0,0.6,5.0),glm::vec3(0.4,-1.0,4.307));
		gcTriangle5.setProperties(ka, ia, gcdc, glm::vec3(0.0,0.6,0.0), gcsc, glm::vec3(1.0,1.0,1.0), gcPhi, 0, 0);

		Triangle gcTriangle6 = Triangle(glm::vec3(0.4,-1.0,4.307),glm::vec3(0.0,0.6,5.0),glm::vec3(0.0,-1.0,4.2));
		gcTriangle6.setProperties(ka, ia, gcdc, glm::vec3(0.0,0.6,0.0), gcsc, glm::vec3(1.0,1.0,1.0), gcPhi, 0, 0);

		Triangle gcTriangle7 = Triangle(glm::vec3(0.0,-1.0,4.2),glm::vec3(0.0,0.6,5.0),glm::vec3(-0.4,-1.0,4.307));
		gcTriangle7.setProperties(ka, ia, gcdc, glm::vec3(0.0,0.6,0.0), gcsc, glm::vec3(1.0,1.0,1.0), gcPhi, 0, 0);

		Triangle gcTriangle8 = Triangle(glm::vec3(-0.4,-1.0,4.307),glm::vec3(0.0,0.6,5.0),glm::vec3(-0.6928,-1.0,4.6));
		gcTriangle8.setProperties(ka, ia, gcdc, glm::vec3(0.0,0.6,0.0), gcsc, glm::vec3(1.0,1.0,1.0), gcPhi, 0, 0);

		Triangle gcTriangle9 = Triangle(glm::vec3(-0.6928,-1.0,4.6),glm::vec3(0.0,0.6,5.0),glm::vec3(-0.8,-1.0,5.0));
		gcTriangle9.setProperties(ka, ia, gcdc, glm::vec3(0.0,0.6,0.0), gcsc, glm::vec3(1.0,1.0,1.0), gcPhi, 0, 0);

		Triangle gcTriangle10 = Triangle(glm::vec3(-0.8,-1.0,5.0),glm::vec3(0.0,0.6,5.0),glm::vec3(-0.6928,-1.0,5.4));
		gcTriangle10.setProperties(ka, ia, gcdc, glm::vec3(0.0,0.6,0.0), gcsc, glm::vec3(1.0,1.0,1.0), gcPhi, 0, 0);

		Triangle gcTriangle11 = Triangle(glm::vec3(-0.6928,-1.0,5.4),glm::vec3(0.0,0.6,5.0),glm::vec3(-0.4,-1.0,5.693));
		gcTriangle11.setProperties(ka, ia, gcdc, glm::vec3(0.0,0.6,0.0), gcsc, glm::vec3(1.0,1.0,1.0), gcPhi, 0, 0);

		Triangle gcTriangle12 = Triangle(glm::vec3(-0.4,-1.0,5.693),glm::vec3(0.0,0.6,5.0),glm::vec3(0.0,-1.0,5.8));
		gcTriangle12.setProperties(ka, ia, gcdc, glm::vec3(0.0,0.6,0.0), gcsc, glm::vec3(1.0,1.0,1.0), gcPhi, 0, 0);
////////////////////////////////////////////////
		float m = 0.3;
		float dc = 0.9;

		Triangle riTriangle1 = Triangle(glm::vec3(-2.0,-1.0,7.0),glm::vec3(-1.276,-0.4472,6.474),glm::vec3(-2.276,-0.4472,6.149));
		riTriangle1.setProperties(ka, ia, dc, glm::vec3(0.6,0.0,0.0), 0, glm::vec3(1.0,0.0,0.0), 1, 0, m);

		Triangle riTriangle2 = Triangle(glm::vec3(-1.276,-0.4472,6.474),glm::vec3(-2.0,-1.0,7.0),glm::vec3(-1.276,-0.4472,7.526));
		riTriangle2.setProperties(ka, ia, dc, glm::vec3(0.6,0.0,0.0), 0, glm::vec3(1.0,0.0,0.0), 1, 0, m);

		Triangle riTriangle3 = Triangle(glm::vec3(-2.0,-1.0,7.0),glm::vec3(-2.276,-0.4472,6.149),glm::vec3(-2.894,-0.4472,7.0));
		riTriangle3.setProperties(ka, ia, dc, glm::vec3(0.6,0.0,0.0), 0, glm::vec3(1.0,0.0,0.0), 1, 0, m);

		Triangle riTriangle4 = Triangle(glm::vec3(-2.0,-1.0,7.0),glm::vec3(-2.894,-0.4472,7.0),glm::vec3(-2.276,-0.4472,7.851));
		riTriangle4.setProperties(ka, ia, dc, glm::vec3(0.6,0.0,0.0), 0, glm::vec3(1.0,0.0,0.0), 1, 0, m);

		Triangle riTriangle5 = Triangle(glm::vec3(-2.0,-1.0,7.0),glm::vec3(-2.276,-0.4472,7.851),glm::vec3(-1.276,-0.4472,7.526));
		riTriangle5.setProperties(ka, ia, dc, glm::vec3(0.6,0.0,0.0), 0, glm::vec3(1.0,0.0,0.0), 1, 0, m);

		Triangle riTriangle6 = Triangle(glm::vec3(-1.276,-0.4472,6.474),glm::vec3(-1.276,-0.4472,7.526),glm::vec3(-1.106,0.4472,7.0));
		riTriangle6.setProperties(ka, ia, dc, glm::vec3(0.6,0.0,0.0), 0, glm::vec3(1.0,0.0,0.0), 1, 0, m);

		Triangle riTriangle7 = Triangle(glm::vec3(-2.276,-0.4472,6.149),glm::vec3(-1.276,-0.4472,6.474),glm::vec3(-1.724,0.4472,6.149));
		riTriangle7.setProperties(ka, ia, dc, glm::vec3(0.6,0.0,0.0), 0, glm::vec3(1.0,0.0,0.0), 1, 0, m);

		Triangle riTriangle8 = Triangle(glm::vec3(-2.894,-0.4472,7.0),glm::vec3(-2.276,-0.4472,6.149),glm::vec3(-2.724,0.4472,6.474));
		riTriangle8.setProperties(ka, ia, dc, glm::vec3(0.6,0.0,0.0), 0, glm::vec3(1.0,0.0,0.0), 1, 0, m);

		Triangle riTriangle9 = Triangle(glm::vec3(-2.276,-0.4472,7.851),glm::vec3(-2.894,-0.4472,7.0),glm::vec3(-2.724,0.4472,7.526));
		riTriangle9.setProperties(ka, ia, dc, glm::vec3(0.6,0.0,0.0), 0, glm::vec3(1.0,0.0,0.0), 1, 0, m);

		Triangle riTriangle10 = Triangle(glm::vec3(-1.276,-0.4472,7.526),glm::vec3(-2.276,-0.4472,7.851),glm::vec3(-1.724,0.4472,7.851));
		riTriangle10.setProperties(ka, ia, dc, glm::vec3(0.6,0.0,0.0), 0, glm::vec3(1.0,0.0,0.0), 1, 0, m);

		Triangle riTriangle11 = Triangle(glm::vec3(-1.276,-0.4472,6.474),glm::vec3(-1.106,0.4472,7.0),glm::vec3(-1.724,0.4472,6.149));
		riTriangle11.setProperties(ka, ia, dc, glm::vec3(0.6,0.0,0.0), 0, glm::vec3(1.0,0.0,0.0), 1, 0, m);

		Triangle riTriangle12 = Triangle(glm::vec3(-2.276,-0.4472,6.149),glm::vec3(-1.724,0.4472,6.149),glm::vec3(-2.724,0.4472,6.474));
		riTriangle12.setProperties(ka, ia, dc, glm::vec3(0.6,0.0,0.0), 0, glm::vec3(1.0,0.0,0.0), 1, 0, m);

		Triangle riTriangle13 = Triangle(glm::vec3(-2.894,-0.4472,7.0),glm::vec3(-2.724,0.4472,6.474),glm::vec3(-2.724,0.4472,7.526));
		riTriangle13.setProperties(ka, ia, dc, glm::vec3(0.6,0.0,0.0), 0, glm::vec3(1.0,0.0,0.0), 1, 0, m);

		Triangle riTriangle14 = Triangle(glm::vec3(-2.276,-0.4472,7.851),glm::vec3(-2.724,0.4472,7.526),glm::vec3(-1.724,0.4472,7.851));
		riTriangle14.setProperties(ka, ia, dc, glm::vec3(0.6,0.0,0.0), 0, glm::vec3(1.0,0.0,0.0), 1, 0, m);

		Triangle riTriangle15 = Triangle(glm::vec3(-1.276,-0.4472,7.526),glm::vec3(-1.724,0.4472,7.851),glm::vec3(-1.106,0.4472,7.0));
		riTriangle15.setProperties(ka, ia, dc, glm::vec3(0.6,0.0,0.0), 0, glm::vec3(1.0,0.0,0.0), 1, 0, m);

		Triangle riTriangle16 = Triangle(glm::vec3(-1.724,0.4472,6.149),glm::vec3(-1.106,0.4472,7.0),glm::vec3(-2.0,1.0,7.0));
		riTriangle16.setProperties(ka, ia, dc, glm::vec3(0.6,0.0,0.0), 0, glm::vec3(1.0,0.0,0.0), 1, 0, m);

		Triangle riTriangle17 = Triangle(glm::vec3(-2.724,0.4472,6.474),glm::vec3(-1.724,0.4472,6.149),glm::vec3(-2.0,1.0,7.0));
		riTriangle17.setProperties(ka, ia, dc, glm::vec3(0.6,0.0,0.0), 0, glm::vec3(1.0,0.0,0.0), 1, 0, m);

		Triangle riTriangle18 = Triangle(glm::vec3(-2.724,0.4472,7.526),glm::vec3(-2.724,0.4472,6.474),glm::vec3(-2.0,1.0,7.0));
		riTriangle18.setProperties(ka, ia, dc, glm::vec3(0.6,0.0,0.0), 0, glm::vec3(1.0,0.0,0.0), 1, 0, m);

		Triangle riTriangle19 = Triangle(glm::vec3(-1.724,0.4472,7.851),glm::vec3(-2.724,0.4472,7.526),glm::vec3(-2.0,1.0,7.0));
		riTriangle19.setProperties(ka, ia, dc, glm::vec3(0.6,0.0,0.0), 0, glm::vec3(1.0,0.0,0.0), 1, 0, m);

		Triangle riTriangle20 = Triangle(glm::vec3(-1.106,0.4472,7.0),glm::vec3(-1.724,0.4472,7.851),glm::vec3(-2.0,1.0,7.0));
		riTriangle20.setProperties(ka, ia, dc, glm::vec3(0.6,0.0,0.0), 0, glm::vec3(1.0,0.0,0.0), 1, 0, m);

		planeObjects.push_back(floorPlane);
		planeObjects.push_back(backPlane);

		sphereObjects.push_back(ySphere);
		sphereObjects.push_back(gSphere);
		sphereObjects.push_back(pSphere);

		triangleObjects.push_back(gcTriangle1);
		triangleObjects.push_back(gcTriangle2);
		triangleObjects.push_back(gcTriangle3);
		triangleObjects.push_back(gcTriangle4);
		triangleObjects.push_back(gcTriangle5);
		triangleObjects.push_back(gcTriangle6);
		triangleObjects.push_back(gcTriangle7);
		triangleObjects.push_back(gcTriangle8);
		triangleObjects.push_back(gcTriangle9);
		triangleObjects.push_back(gcTriangle10);
		triangleObjects.push_back(gcTriangle11);
		triangleObjects.push_back(gcTriangle12);

		triangleObjects.push_back(riTriangle1);
		triangleObjects.push_back(riTriangle2);
		triangleObjects.push_back(riTriangle3);
		triangleObjects.push_back(riTriangle4);
		triangleObjects.push_back(riTriangle5);
		triangleObjects.push_back(riTriangle6);
		triangleObjects.push_back(riTriangle7);
		triangleObjects.push_back(riTriangle8);
		triangleObjects.push_back(riTriangle9);
		triangleObjects.push_back(riTriangle10);
		triangleObjects.push_back(riTriangle11);
		triangleObjects.push_back(riTriangle12);
		triangleObjects.push_back(riTriangle13);
		triangleObjects.push_back(riTriangle14);
		triangleObjects.push_back(riTriangle15);
		triangleObjects.push_back(riTriangle16);
		triangleObjects.push_back(riTriangle17);
		triangleObjects.push_back(riTriangle18);
		triangleObjects.push_back(riTriangle19);
		triangleObjects.push_back(riTriangle20);

		std::cout << "Scene 2 generated" <<std::endl;
	}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

	if (scene == 3){
		float ka = 0.3;
		glm::vec3 ia = glm::vec3(1.0, 1.0, 1.0);

		lightSource = glm::vec3(-100.0,500.0,-100.0);

		Plane floorPlane = Plane(glm::vec3(0.0,-50.0,0.0),glm::vec3(0.0,1.0,0.0));
		floorPlane.setProperties(ka, ia, 1.0, glm::vec3(0.0,0.5,0.0), 0.0, glm::vec3(0.0,0.0,0.0), 1, 0, 0);

		Plane SkyPlane = Plane(glm::vec3(0.0,0.0,1000.0),glm::vec3(0.0,0.0,-1.0));
		SkyPlane.setProperties(ka, ia, 1.0, glm::vec3(0.0,0.0,0.6), 0.0, glm::vec3(0.0,0.0,0.0), 1, 0, 0);


		Sphere cSphere1 = Sphere(glm::vec3(-15.0,20.0,80.0),4);
		cSphere1.setProperties(ka, ia, 1.0, glm::vec3(3.0,3.0,3.0), 0.0, glm::vec3(1.0,1.0,1.0), 1, 0, 0.05);
		Sphere cSphere2 = Sphere(glm::vec3(-18.0,20.0,78.0),3);
		cSphere2.setProperties(ka, ia, 1.0, glm::vec3(3.0,3.0,3.0), 0.0, glm::vec3(1.0,1.0,1.0), 1, 0, 0.05);
		Sphere cSphere3 = Sphere(glm::vec3(-12.0,20.0,78.0),3);
		cSphere3.setProperties(ka, ia, 1.0, glm::vec3(3.0,3.0,3.0), 0.0, glm::vec3(1.0,1.0,1.0), 1, 0, 0.05);

		sphereObjects.push_back(cSphere1);
		sphereObjects.push_back(cSphere2);
		sphereObjects.push_back(cSphere3);


		Sphere cSphere4 = Sphere(glm::vec3(35.0,20.0,100.0),4);
		cSphere4.setProperties(ka, ia, 1.0, glm::vec3(3.0,3.0,3.0), 0.0, glm::vec3(1.0,1.0,1.0), 1, 0, 0);
		Sphere cSphere5 = Sphere(glm::vec3(32.0,20.0,98.0),3);
		cSphere5.setProperties(ka, ia, 1.0, glm::vec3(3.0,3.0,3.0), 0.0, glm::vec3(1.0,1.0,1.0), 1, 0, 0);
		Sphere cSphere6 = Sphere(glm::vec3(38.0,20.0,98.0),3);
		cSphere6.setProperties(ka, ia, 1.0, glm::vec3(3.0,3.0,3.0), 0.0, glm::vec3(1.0,1.0,1.0), 1, 0, 0);

		sphereObjects.push_back(cSphere4);
		sphereObjects.push_back(cSphere5);
		sphereObjects.push_back(cSphere6);


		Sphere cSphere7 = Sphere(glm::vec3(10.0,20.0,120.0),4);
		cSphere7.setProperties(ka, ia, 1.0, glm::vec3(3.0,3.0,3.0), 0.0, glm::vec3(1.0,1.0,1.0), 1, 0, 0);
		Sphere cSphere8 = Sphere(glm::vec3(8.0,20.0,118.0),3);
		cSphere8.setProperties(ka, ia, 1.0, glm::vec3(3.0,3.0,3.0), 0.0, glm::vec3(1.0,1.0,1.0), 1, 0, 0);
		Sphere cSphere9 = Sphere(glm::vec3(12.0,20.0,122.0),3);
		cSphere9.setProperties(ka, ia, 1.0, glm::vec3(3.0,3.0,3.0), 0.0, glm::vec3(1.0,1.0,1.0), 1, 0, 0);

		sphereObjects.push_back(cSphere7);
		sphereObjects.push_back(cSphere8);
		sphereObjects.push_back(cSphere9);

		//glm::vec3 shipOrigin = glm::vec3(0.0,0.0,5.0);

		//float xDegrees = 90.0;
		//glm::mat3 rotation = glm::mat3(glm::vec3(1.0,0.0,0.0), glm::vec3(0.0, glm::cos(xDegrees),glm::sin(xDegrees)), glm::vec3(0.0,-glm::sin(xDegrees),glm::cos(xDegrees)));

		float bDif = 0.9;
		float bSpec = 0.1;
		float bRef = 0.13;

		glm::vec3 xReflect = glm::vec3(-1.0,1.0,1.0);

		//Ship length = 12

		glm::vec3 brp1 = glm::vec3(1.0,0.0,30.0);
		glm::vec3 brp2 = glm::vec3(0.0,-2.0,18.0);
		glm::vec3 brp3 = glm::vec3(1.5,-1.8,24.0);
		glm::vec3 brp4 = glm::vec3(2.5,-2.0,31.0);
		glm::vec3 brp5 = glm::vec3(1.25,-1.0,35.0);

		//brp1 = (rotation*(brp1-shipOrigin))+shipOrigin;

		glm::vec3 blp1 = brp1 * xReflect;
		glm::vec3 blp2 = brp2 * xReflect;
		glm::vec3 blp3 = brp3 * xReflect;
		glm::vec3 blp4 = brp4 * xReflect;
		glm::vec3 blp5 = brp5 * xReflect;

		//Arwing Body
		Triangle bTriangle1 = Triangle(brp1,blp1,brp2);
		Triangle bTriangle2 = Triangle(brp1,brp2,brp3);
		Triangle bTriangle3 = Triangle(brp1,brp3,brp4);
		Triangle bTriangle4 = Triangle(brp1,brp4,brp5);

		Triangle bTriangle5 = Triangle(blp1,blp3,blp2);
		Triangle bTriangle6 = Triangle(blp1,blp4,blp3);
		Triangle bTriangle7 = Triangle(blp1,blp5,brp4);

		bTriangle1.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		bTriangle2.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		bTriangle3.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		bTriangle4.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);

		bTriangle5.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		bTriangle6.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		bTriangle7.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);

		//Arwing Cockpit
		glm::vec3 crp1 = glm::vec3(1.0,2.0,34.0);
		glm::vec3 clp1 = crp1 * xReflect;

		Triangle cTriangle1 = Triangle(brp1,crp1,blp1);
		Triangle cTriangle2 = Triangle(blp1,crp1,clp1);

		Triangle cTriangle3 = Triangle(brp1,brp5,crp1);
		Triangle cTriangle4 = Triangle(blp1,clp1,blp5);

		cTriangle1.setProperties(ka, ia, 1.0, glm::vec3(0.05,0.05,0.05), 1.0, glm::vec3(1.0,1.0,1.0), 32, 0.1, 0);
		cTriangle2.setProperties(ka, ia, 1.0, glm::vec3(0.05,0.05,0.05), 1.0, glm::vec3(1.0,1.0,1.0), 32, 0.1, 0);

		cTriangle3.setProperties(ka, ia, 1.0, glm::vec3(0.05,0.05,0.05), 1.0, glm::vec3(1.0,1.0,1.0), 32, 0.1, 0);
		cTriangle4.setProperties(ka, ia, 1.0, glm::vec3(0.05,0.05,0.05), 1.0, glm::vec3(1.0,1.0,1.0), 32, 0.1, 0);

		//Arwing sholders
		glm::vec3 srp1 = glm::vec3(3.0,2.5,36.0);
		glm::vec3 srp2 = glm::vec3(1.5,-3.5,31.0);
		glm::vec3 srp3 = glm::vec3(2.7,-3.5,24.0);
		glm::vec3 srp4 = glm::vec3(3.7,-3.5,31.0);

		glm::vec3 slp1 = srp1 * xReflect;
		glm::vec3 slp2 = srp2 * xReflect;
		glm::vec3 slp3 = srp3 * xReflect;
		glm::vec3 slp4 = srp4 * xReflect;

		Triangle sTriangle1 = Triangle(srp1,srp2,srp3);
		Triangle sTriangle2 = Triangle(srp1,srp4,srp3);
		Triangle sTriangle3 = Triangle(slp1,slp3,slp2);
		Triangle sTriangle4 = Triangle(slp1,slp3,slp4);

		sTriangle1.setProperties(ka, ia, 1.0, glm::vec3(0.3,0.3,0.8), 0.0, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		sTriangle2.setProperties(ka, ia, 1.0, glm::vec3(0.3,0.3,0.8), 0.0, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		sTriangle3.setProperties(ka, ia, 1.0, glm::vec3(0.3,0.3,0.8), 0.0, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		sTriangle4.setProperties(ka, ia, 1.0, glm::vec3(0.3,0.3,0.8), 0.0, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);


		//Arwing Wings
		glm::vec3 wrp1 = glm::vec3(3.0,-2.0,33.0);
		glm::vec3 wrp2 = glm::vec3(3.0,-2.2,30.0);
		glm::vec3 wrp3 = glm::vec3(8.0,-2.2,30.0);
		glm::vec3 wrp4 = glm::vec3(8.0,-2.2,33.0);
		glm::vec3 wrp5 = glm::vec3(10.0,-2.0,31.5);
		glm::vec3 wrp6 = glm::vec3(12.0,-1.8,35.0);

		glm::vec3 wlp1 = wrp1 * xReflect;
		glm::vec3 wlp2 = wrp2 * xReflect;
		glm::vec3 wlp3 = wrp3 * xReflect;
		glm::vec3 wlp4 = wrp4 * xReflect;
		glm::vec3 wlp5 = wrp5 * xReflect;
		glm::vec3 wlp6 = wrp6 * xReflect;


		Triangle wTriangle1 = Triangle(wrp1,wrp2,wrp3);
		Triangle wTriangle2 = Triangle(wrp1,wrp3,wrp4);
		Triangle wTriangle3 = Triangle(wrp4,wrp3,wrp5);
		Triangle wTriangle4 = Triangle(wrp4,wrp5,wrp6);

		Triangle wTriangle5 = Triangle(wlp1,wlp3,wlp2);
		Triangle wTriangle6 = Triangle(wlp1,wlp4,wlp3);
		Triangle wTriangle7 = Triangle(wlp4,wlp5,wlp3);
		Triangle wTriangle8 = Triangle(wlp4,wlp6,wlp5);


		wTriangle1.setProperties(ka, ia, 1.0, glm::vec3(0.6,0.6,0.6), 0.0, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		wTriangle2.setProperties(ka, ia, 1.0, glm::vec3(0.6,0.6,0.6), 0.0, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		wTriangle3.setProperties(ka, ia, 1.0, glm::vec3(0.6,0.6,0.6), 0.0, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		wTriangle4.setProperties(ka, ia, 1.0, glm::vec3(0.6,0.6,0.6), 0.0, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		wTriangle5.setProperties(ka, ia, 1.0, glm::vec3(0.6,0.6,0.6), 0.0, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		wTriangle6.setProperties(ka, ia, 1.0, glm::vec3(0.6,0.6,0.6), 0.0, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		wTriangle7.setProperties(ka, ia, 1.0, glm::vec3(0.6,0.6,0.6), 0.0, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		wTriangle8.setProperties(ka, ia, 1.0, glm::vec3(0.6,0.6,0.6), 0.0, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);


		planeObjects.push_back(floorPlane);
		planeObjects.push_back(SkyPlane);



		triangleObjects.push_back(bTriangle1);
		triangleObjects.push_back(bTriangle2);
		triangleObjects.push_back(bTriangle3);
		triangleObjects.push_back(bTriangle4);
		triangleObjects.push_back(bTriangle5);
		triangleObjects.push_back(bTriangle6);
		triangleObjects.push_back(bTriangle7);

		triangleObjects.push_back(cTriangle1);
		triangleObjects.push_back(cTriangle2);
		triangleObjects.push_back(cTriangle3);
		triangleObjects.push_back(cTriangle4);

		triangleObjects.push_back(sTriangle1);
		triangleObjects.push_back(sTriangle2);
		triangleObjects.push_back(sTriangle3);
		triangleObjects.push_back(sTriangle4);

		triangleObjects.push_back(wTriangle1);
		triangleObjects.push_back(wTriangle2);
		triangleObjects.push_back(wTriangle3);
		triangleObjects.push_back(wTriangle4);
		triangleObjects.push_back(wTriangle5);
		triangleObjects.push_back(wTriangle6);
		triangleObjects.push_back(wTriangle7);
		triangleObjects.push_back(wTriangle8);
	}



	if (scene == 4){
		float ka = 0.3;
		glm::vec3 ia = glm::vec3(1.0, 1.0, 1.0);

		lightSource = glm::vec3(-10.0,15.0,0.0);

		Sphere Base = Sphere(glm::vec3(0.0,-3.8,20.0),2.0);
		Base.setProperties(ka, ia, 0.8, glm::vec3(1.0,1.0,1.0), 0.01, glm::vec3(1.0,1.0,1.0), 4, 0, 0);

		Sphere Body = Sphere(glm::vec3(0.0,-1.0,20.0),1.5);
		Body.setProperties(ka, ia, 0.8, glm::vec3(1.0,1.0,1.0), 0.01, glm::vec3(1.0,1.0,1.0), 4, 0, 0);

		Sphere Head = Sphere(glm::vec3(0.0,1.0,20.0),1.0);
		Head.setProperties(ka, ia, 0.8, glm::vec3(1.0,1.0,1.0), 0.01, glm::vec3(1.0,1.0,1.0), 4, 0, 0);


		Sphere Button1 = Sphere(glm::vec3(0.0,-0.2,18.75),0.2);
		Button1.setProperties(ka, ia, 0.8, glm::vec3(0.0,0.0,0.0), 0.1, glm::vec3(1.0,1.0,1.0), 4, 0.5, 0.0);

		Sphere Button2 = Sphere(glm::vec3(0.0,-0.8,18.6),0.2);
		Button2.setProperties(ka, ia, 0.8, glm::vec3(0.0,0.0,0.0), 0.1, glm::vec3(1.0,1.0,1.0), 4, 0.0, 0.5);

		Sphere Button3 = Sphere(glm::vec3(0.0,-1.4,18.6),0.2);
		Button3.setProperties(ka, ia, 0.8, glm::vec3(0.0,0.0,0.0), 0.1, glm::vec3(1.0,1.0,1.0), 4, 0.2, 0);


		Sphere lEye = Sphere(glm::vec3(-0.5,1.0,19.0),0.1);
		Button3.setProperties(ka, ia, 0.8, glm::vec3(0.0,0.0,0.0), 0, glm::vec3(1.0,1.0,1.0), 4, 0, 0);

		Sphere rEye = Sphere(glm::vec3(0.5,1.0,19.0),0.1);
		Button3.setProperties(ka, ia, 0.8, glm::vec3(0.0,0.0,0.0), 0, glm::vec3(1.0,1.0,1.0), 4, 0, 0);


		Plane plane1 = Plane(glm::vec3(0.0,-5.0,20.0),glm::vec3(0.0,1.0,0.0));
		plane1.setProperties(ka, ia, 1.0, glm::vec3(1.0,1.0,1.0), 0.0, glm::vec3(0.0,0.0,0.0), 1, 0, 0);

		Plane plane2 = Plane(glm::vec3(0.0,0.0,40.0),glm::vec3(0.0,0.0,-1.0));
		plane2.setProperties(ka, ia, 1.0, glm::vec3(0.5,0.5,0.9), 0.0, glm::vec3(0.0,0.0,0.0), 1, 0, 0);

		Plane plane3 = Plane(glm::vec3(0.0,40.0,0.0),glm::vec3(0.0,-1.0,0.0));
		plane3.setProperties(ka, ia, 1.0, glm::vec3(0.5,0.5,0.9), 0.0, glm::vec3(0.0,0.0,0.0), 1, 0, 0);

		sphereObjects.push_back(Base);
		sphereObjects.push_back(Body);
		sphereObjects.push_back(Head);
		sphereObjects.push_back(Button1);
		sphereObjects.push_back(Button2);
		sphereObjects.push_back(Button3);
		sphereObjects.push_back(lEye);
		sphereObjects.push_back(rEye);
		planeObjects.push_back(plane1);
		planeObjects.push_back(plane2);
		planeObjects.push_back(plane3);
		std::cout << "Scene 4 generated" <<std::endl;

	}


	if (scene == 5){
		float ka = 0.5;
		glm::vec3 ia = glm::vec3(1.0, 1.0, 1.0);

		lightSource = glm::vec3(0.0,500.0,0.0);

		Plane floorPlane = Plane(glm::vec3(0.0,-50.0,0.0),glm::vec3(0.0,1.0,0.0));
		floorPlane.setProperties(ka, ia, 1.0, glm::vec3(0.0,0.5,0.0), 0.0, glm::vec3(0.0,0.0,0.0), 1, 0, 0);

		Plane SkyPlane = Plane(glm::vec3(0.0,0.0,1000.0),glm::vec3(0.0,0.0,-1.0));
		SkyPlane.setProperties(ka, ia, 1.0, glm::vec3(0.0,0.0,0.6), 0.0, glm::vec3(0.0,0.0,0.0), 1, 0, 0);

		planeObjects.push_back(floorPlane);
		planeObjects.push_back(SkyPlane);



		float bDif = 1.0;
		float bSpec = 0.0;
		float bRef = 0.0;



		Triangle triangle1 = Triangle(glm::vec3(-1.16467,1.65129,13.8032),glm::vec3(-0.298779,1.99383,14.8105),glm::vec3(-0.501897,1.62755,13.7508));
		Triangle triangle2 = Triangle(glm::vec3(0.106169,1.01988,13.577),glm::vec3(0.757034,0.938699,14.5089),glm::vec3(0.114802,0.367013,13.4318));
		Triangle triangle3 = Triangle(glm::vec3(1.43053,0.613305,15.9496),glm::vec3(0.874114,0.977979,14.6895),glm::vec3(1.43916,1.09876,15.874));
		Triangle triangle4 = Triangle(glm::vec3(1.43053,0.613305,15.9496),glm::vec3(-0.646551,-0.375646,12.1392),glm::vec3(0.874114,0.977979,14.6895));
		Triangle triangle5 = Triangle(glm::vec3(2.77846,2.85278,18.7325),glm::vec3(1.43053,0.613305,15.9496),glm::vec3(1.43916,1.09876,15.874));
		Triangle triangle6 = Triangle(glm::vec3(0.0904007,1.07572,13.5675),glm::vec3(0.114802,0.367013,13.4318),glm::vec3(-0.646551,-0.375646,12.1392));
		Triangle triangle7 = Triangle(glm::vec3(0.114802,0.367013,13.4318),glm::vec3(0.0904007,1.07572,13.5675),glm::vec3(0.106169,1.01988,13.577));
		Triangle triangle8 = Triangle(glm::vec3(0.114802,0.367013,13.4318),glm::vec3(0.874114,0.977979,14.6895),glm::vec3(-0.646551,-0.375646,12.1392));
		Triangle triangle9 = Triangle(glm::vec3(0.874114,0.977979,14.6895),glm::vec3(0.114802,0.367013,13.4318),glm::vec3(0.757034,0.938699,14.5089));
		Triangle triangle10 = Triangle(glm::vec3(0.874114,0.977979,14.6895),glm::vec3(0.757034,0.938699,14.5089),glm::vec3(0.0904007,1.07572,13.5675));
		Triangle triangle11 = Triangle(glm::vec3(0.0904007,1.07572,13.5675),glm::vec3(0.757034,0.938699,14.5089),glm::vec3(0.106169,1.01988,13.577));
		Triangle triangle12 = Triangle(glm::vec3(0.832374,0.323611,16.4615),glm::vec3(-0.646551,-0.375646,12.1392),glm::vec3(1.43053,0.613305,15.9496));
		Triangle triangle13 = Triangle(glm::vec3(2.77846,2.85278,18.7325),glm::vec3(0.832374,0.323611,16.4615),glm::vec3(1.43053,0.613305,15.9496));
		Triangle triangle14 = Triangle(glm::vec3(2.77846,2.85278,18.7325),glm::vec3(1.43916,1.09876,15.874),glm::vec3(1.72252,2.12906,15.5955));
		Triangle triangle15 = Triangle(glm::vec3(-3.42154,-0.198283,7.71441),glm::vec3(0.0904007,1.07572,13.5675),glm::vec3(-0.646551,-0.375646,12.1392));
		Triangle triangle16 = Triangle(glm::vec3(-1.61934,0.242816,12.4695),glm::vec3(-0.646551,-0.375646,12.1392),glm::vec3(0.832374,0.323611,16.4615));
		Triangle triangle17 = Triangle(glm::vec3(2.77846,2.85278,18.7325),glm::vec3(0.791333,0.668777,16.6654),glm::vec3(0.832374,0.323611,16.4615));
		Triangle triangle18 = Triangle(glm::vec3(0.791333,0.668777,16.6654),glm::vec3(2.77846,2.85278,18.7325),glm::vec3(1.2236,1.63809,17.4273));
		Triangle triangle19 = Triangle(glm::vec3(1.2236,1.63809,17.4273),glm::vec3(2.77846,2.85278,18.7325),glm::vec3(0.0822828,1.37737,16.868));
		Triangle triangle20 = Triangle(glm::vec3(0.0822828,1.37737,16.868),glm::vec3(2.77846,2.85278,18.7325),glm::vec3(-0.299536,1.45479,16.7849));
		Triangle triangle21 = Triangle(glm::vec3(0.791333,0.668777,16.6654),glm::vec3(-0.299536,1.45479,16.7849),glm::vec3(0.832374,0.323611,16.4615));
		Triangle triangle22 = Triangle(glm::vec3(-0.299536,1.45479,16.7849),glm::vec3(0.791333,0.668777,16.6654),glm::vec3(0.0822828,1.37737,16.868));
		Triangle triangle23 = Triangle(glm::vec3(1.18065,2.67058,15.7503),glm::vec3(2.77846,2.85278,18.7325),glm::vec3(1.72252,2.12906,15.5955));
		Triangle triangle24 = Triangle(glm::vec3(-3.42154,-0.198283,7.71441),glm::vec3(-0.0894818,1.25382,13.3014),glm::vec3(0.0904007,1.07572,13.5675));
		Triangle triangle25 = Triangle(glm::vec3(-3.42154,-0.198283,7.71441),glm::vec3(-0.646551,-0.375646,12.1392),glm::vec3(-1.61934,0.242816,12.4695));
		Triangle triangle26 = Triangle(glm::vec3(-0.299536,1.45479,16.7849),glm::vec3(-1.61934,0.242816,12.4695),glm::vec3(0.832374,0.323611,16.4615));
		Triangle triangle27 = Triangle(glm::vec3(2.77846,2.85278,18.7325),glm::vec3(-0.189617,2.2324,16.4126),glm::vec3(-0.299536,1.45479,16.7849));
		Triangle triangle28 = Triangle(glm::vec3(2.77846,2.85278,18.7325),glm::vec3(1.18065,2.67058,15.7503),glm::vec3(0.30725,2.22993,16.1974));
		Triangle triangle29 = Triangle(glm::vec3(-0.360417,1.52458,13.3788),glm::vec3(-0.0894818,1.25382,13.3014),glm::vec3(-3.42154,-0.198283,7.71441));
		Triangle triangle30 = Triangle(glm::vec3(-2.2667,1.24345,12.6021),glm::vec3(-3.42154,-0.198283,7.71441),glm::vec3(-1.61934,0.242816,12.4695));
		Triangle triangle31 = Triangle(glm::vec3(-2.2667,1.24345,12.6021),glm::vec3(-1.61934,0.242816,12.4695),glm::vec3(-0.299536,1.45479,16.7849));
		Triangle triangle32 = Triangle(glm::vec3(-0.299536,1.45479,16.7849),glm::vec3(-0.189617,2.2324,16.4126),glm::vec3(-2.2667,1.24345,12.6021));
		Triangle triangle33 = Triangle(glm::vec3(-0.189617,2.2324,16.4126),glm::vec3(2.77846,2.85278,18.7325),glm::vec3(0.30725,2.22993,16.1974));
		Triangle triangle34 = Triangle(glm::vec3(-0.45147,1.61724,13.7223),glm::vec3(-0.360417,1.52458,13.3788),glm::vec3(-3.42154,-0.198283,7.71441));
		Triangle triangle35 = Triangle(glm::vec3(-2.2667,1.24345,12.6021),glm::vec3(-0.45147,1.61724,13.7223),glm::vec3(-3.42154,-0.198283,7.71441));
		Triangle triangle36 = Triangle(glm::vec3(-0.189617,2.2324,16.4126),glm::vec3(-0.206894,2.05828,14.9984),glm::vec3(-2.2667,1.24345,12.6021));
		Triangle triangle37 = Triangle(glm::vec3(-0.189617,2.2324,16.4126),glm::vec3(0.30725,2.22993,16.1974),glm::vec3(-0.206894,2.05828,14.9984));
		Triangle triangle38 = Triangle(glm::vec3(-2.2667,1.24345,12.6021),glm::vec3(-0.501897,1.62755,13.7508),glm::vec3(-0.45147,1.61724,13.7223));
		Triangle triangle39 = Triangle(glm::vec3(-0.501897,1.62755,13.7508),glm::vec3(-2.2667,1.24345,12.6021),glm::vec3(-1.16467,1.65129,13.8032));
		Triangle triangle40 = Triangle(glm::vec3(-0.501897,1.62755,13.7508),glm::vec3(-0.206894,2.05828,14.9984),glm::vec3(-0.45147,1.61724,13.7223));
		Triangle triangle41 = Triangle(glm::vec3(-0.206894,2.05828,14.9984),glm::vec3(-0.501897,1.62755,13.7508),glm::vec3(-0.298779,1.99383,14.8105));
		Triangle triangle42 = Triangle(glm::vec3(-0.206894,2.05828,14.9984),glm::vec3(-0.298779,1.99383,14.8105),glm::vec3(-1.16467,1.65129,13.8032));
		Triangle triangle43 = Triangle(glm::vec3(-0.206894,2.05828,14.9984),glm::vec3(-1.16467,1.65129,13.8032),glm::vec3(-2.2667,1.24345,12.6021));
		Triangle triangle44 = Triangle(glm::vec3(1.33315,1.92205,14.9568),glm::vec3(0.0904007,1.07572,13.5675),glm::vec3(-0.0894818,1.25382,13.3014));
		Triangle triangle45 = Triangle(glm::vec3(-0.0894818,1.25382,13.3014),glm::vec3(0.0904007,1.07572,13.5675),glm::vec3(1.33315,1.92205,14.9568));
		Triangle triangle46 = Triangle(glm::vec3(-0.0894818,1.25382,13.3014),glm::vec3(0.791284,2.46357,15.1116),glm::vec3(1.33315,1.92205,14.9568));
		Triangle triangle47 = Triangle(glm::vec3(0.791284,2.46357,15.1116),glm::vec3(-0.0894818,1.25382,13.3014),glm::vec3(-0.360417,1.52458,13.3788));
		Triangle triangle48 = Triangle(glm::vec3(-0.360417,1.52458,13.3788),glm::vec3(-0.0894818,1.25382,13.3014),glm::vec3(0.791284,2.46357,15.1116));
		Triangle triangle49 = Triangle(glm::vec3(1.33315,1.92205,14.9568),glm::vec3(0.791284,2.46357,15.1116),glm::vec3(-0.0894818,1.25382,13.3014));
		Triangle triangle50 = Triangle(glm::vec3(1.33315,1.92205,14.9568),glm::vec3(0.874114,0.977979,14.6895),glm::vec3(0.0904007,1.07572,13.5675));
		Triangle triangle51 = Triangle(glm::vec3(0.0904007,1.07572,13.5675),glm::vec3(0.874114,0.977979,14.6895),glm::vec3(1.33315,1.92205,14.9568));
		Triangle triangle52 = Triangle(glm::vec3(0.791284,2.46357,15.1116),glm::vec3(-0.360417,1.52458,13.3788),glm::vec3(-0.45147,1.61724,13.7223));
		Triangle triangle53 = Triangle(glm::vec3(-0.45147,1.61724,13.7223),glm::vec3(-0.360417,1.52458,13.3788),glm::vec3(0.791284,2.46357,15.1116));
		Triangle triangle54 = Triangle(glm::vec3(0.791284,2.46357,15.1116),glm::vec3(1.72252,2.12906,15.5955),glm::vec3(1.33315,1.92205,14.9568));
		Triangle triangle55 = Triangle(glm::vec3(1.72252,2.12906,15.5955),glm::vec3(0.791284,2.46357,15.1116),glm::vec3(1.18065,2.67058,15.7503));
		Triangle triangle56 = Triangle(glm::vec3(1.18065,2.67058,15.7503),glm::vec3(0.791284,2.46357,15.1116),glm::vec3(1.72252,2.12906,15.5955));
		Triangle triangle57 = Triangle(glm::vec3(1.33315,1.92205,14.9568),glm::vec3(1.72252,2.12906,15.5955),glm::vec3(0.791284,2.46357,15.1116));
		Triangle triangle58 = Triangle(glm::vec3(1.72252,2.12906,15.5955),glm::vec3(0.874114,0.977979,14.6895),glm::vec3(1.33315,1.92205,14.9568));
		Triangle triangle59 = Triangle(glm::vec3(1.33315,1.92205,14.9568),glm::vec3(0.874114,0.977979,14.6895),glm::vec3(1.72252,2.12906,15.5955));
		Triangle triangle60 = Triangle(glm::vec3(0.791284,2.46357,15.1116),glm::vec3(-0.45147,1.61724,13.7223),glm::vec3(-0.206894,2.05828,14.9984));
		Triangle triangle61 = Triangle(glm::vec3(-0.206894,2.05828,14.9984),glm::vec3(-0.45147,1.61724,13.7223),glm::vec3(0.791284,2.46357,15.1116));
		Triangle triangle62 = Triangle(glm::vec3(1.18065,2.67058,15.7503),glm::vec3(0.791284,2.46357,15.1116),glm::vec3(-0.206894,2.05828,14.9984));
		Triangle triangle63 = Triangle(glm::vec3(-0.206894,2.05828,14.9984),glm::vec3(0.791284,2.46357,15.1116),glm::vec3(1.18065,2.67058,15.7503));
		Triangle triangle64 = Triangle(glm::vec3(1.43916,1.09876,15.874),glm::vec3(0.874114,0.977979,14.6895),glm::vec3(1.72252,2.12906,15.5955));
		Triangle triangle65 = Triangle(glm::vec3(1.72252,2.12906,15.5955),glm::vec3(0.874114,0.977979,14.6895),glm::vec3(1.43916,1.09876,15.874));
		Triangle triangle66 = Triangle(glm::vec3(0.30725,2.22993,16.1974),glm::vec3(1.18065,2.67058,15.7503),glm::vec3(-0.206894,2.05828,14.9984));
		Triangle triangle67 = Triangle(glm::vec3(-0.206894,2.05828,14.9984),glm::vec3(1.18065,2.67058,15.7503),glm::vec3(0.30725,2.22993,16.1974));
		Triangle triangle68 = Triangle(glm::vec3(-2.12726,3.36308,16.2521),glm::vec3(-2.02332,3.48515,16.189),glm::vec3(-1.86044,2.42214,15.0789));
		Triangle triangle69 = Triangle(glm::vec3(-2.12726,3.36308,16.2521),glm::vec3(-1.86044,2.42214,15.0789),glm::vec3(-3.4967,3.41257,14.7704));
		Triangle triangle70 = Triangle(glm::vec3(-2.12726,3.36308,16.2521),glm::vec3(-0.982012,7.45441,21.0011),glm::vec3(-2.02332,3.48515,16.189));
		Triangle triangle71 = Triangle(glm::vec3(-1.86044,2.42214,15.0789),glm::vec3(-2.02332,3.48515,16.189),glm::vec3(-1.81531,2.46938,15.0541));
		Triangle triangle72 = Triangle(glm::vec3(-1.86044,2.42214,15.0789),glm::vec3(-2.2667,1.24345,12.6021),glm::vec3(-3.4967,3.41257,14.7704));
		Triangle triangle73 = Triangle(glm::vec3(-2.63541,5.105,18.1061),glm::vec3(-2.12726,3.36308,16.2521),glm::vec3(-3.4967,3.41257,14.7704));
		Triangle triangle74 = Triangle(glm::vec3(-0.982012,7.45441,21.0011),glm::vec3(-2.12726,3.36308,16.2521),glm::vec3(-2.63541,5.105,18.1061));
		Triangle triangle75 = Triangle(glm::vec3(-2.63541,5.105,18.1061),glm::vec3(-2.02332,3.48515,16.189),glm::vec3(-0.982012,7.45441,21.0011));
		Triangle triangle76 = Triangle(glm::vec3(-0.840455,1.92252,15.2186),glm::vec3(-1.86044,2.42214,15.0789),glm::vec3(-1.81531,2.46938,15.0541));
		Triangle triangle77 = Triangle(glm::vec3(-3.4967,3.41257,14.7704),glm::vec3(-1.81531,2.46938,15.0541),glm::vec3(-2.02332,3.48515,16.189));
		Triangle triangle78 = Triangle(glm::vec3(-2.2667,1.24345,12.6021),glm::vec3(-1.29261,1.71303,14.3688),glm::vec3(-3.4967,3.41257,14.7704));
		Triangle triangle79 = Triangle(glm::vec3(-1.16947,1.48258,14.6245),glm::vec3(-2.2667,1.24345,12.6021),glm::vec3(-1.86044,2.42214,15.0789));
		Triangle triangle80 = Triangle(glm::vec3(-3.4967,3.41257,14.7704),glm::vec3(-2.02332,3.48515,16.189),glm::vec3(-2.63541,5.105,18.1061));
		Triangle triangle81 = Triangle(glm::vec3(-1.16947,1.48258,14.6245),glm::vec3(-1.86044,2.42214,15.0789),glm::vec3(-0.840455,1.92252,15.2186));
		Triangle triangle82 = Triangle(glm::vec3(-0.840455,1.92252,15.2186),glm::vec3(-1.81531,2.46938,15.0541),glm::vec3(-1.29261,1.71303,14.3688));
		Triangle triangle83 = Triangle(glm::vec3(-3.4967,3.41257,14.7704),glm::vec3(-1.29261,1.71303,14.3688),glm::vec3(-1.81531,2.46938,15.0541));
		Triangle triangle84 = Triangle(glm::vec3(-2.2667,1.24345,12.6021),glm::vec3(-0.840455,1.92252,15.2186),glm::vec3(-1.29261,1.71303,14.3688));
		Triangle triangle85 = Triangle(glm::vec3(-1.86044,2.42214,15.0789),glm::vec3(-0.840455,1.92252,15.2186),glm::vec3(-1.81531,2.46938,15.0541));
		Triangle triangle86 = Triangle(glm::vec3(-2.2667,1.24345,12.6021),glm::vec3(-1.16947,1.48258,14.6245),glm::vec3(-1.86044,2.42214,15.0789));
		Triangle triangle87 = Triangle(glm::vec3(-1.86044,2.42214,15.0789),glm::vec3(-1.16947,1.48258,14.6245),glm::vec3(-0.840455,1.92252,15.2186));
		Triangle triangle88 = Triangle(glm::vec3(-1.53976,-1.53826,9.64292),glm::vec3(1.59175,-0.800509,14.5858),glm::vec3(3.16801,1.38335,15.9008));
		Triangle triangle89 = Triangle(glm::vec3(3.16801,1.38335,15.9008),glm::vec3(1.59175,-0.800509,14.5858),glm::vec3(1.17123,-0.240646,14.5924));
		Triangle triangle90 = Triangle(glm::vec3(0.647763,0.142861,14.8555),glm::vec3(-1.53976,-1.53826,9.64292),glm::vec3(3.16801,1.38335,15.9008));
		Triangle triangle91 = Triangle(glm::vec3(1.17123,-0.240646,14.5924),glm::vec3(1.59175,-0.800509,14.5858),glm::vec3(-1.53976,-1.53826,9.64292));
		Triangle triangle92 = Triangle(glm::vec3(0.647763,0.142861,14.8555),glm::vec3(3.16801,1.38335,15.9008),glm::vec3(1.17123,-0.240646,14.5924));
		Triangle triangle93 = Triangle(glm::vec3(0.647763,0.142861,14.8555),glm::vec3(1.17123,-0.240646,14.5924),glm::vec3(-1.53976,-1.53826,9.64292));
		Triangle triangle94 = Triangle(glm::vec3(3.16801,1.38335,15.9008),glm::vec3(1.59175,-0.800509,14.5858),glm::vec3(-1.53976,-1.53826,9.64292));
		Triangle triangle95 = Triangle(glm::vec3(1.17123,-0.240646,14.5924),glm::vec3(1.59175,-0.800509,14.5858),glm::vec3(3.16801,1.38335,15.9008));
		Triangle triangle96 = Triangle(glm::vec3(3.16801,1.38335,15.9008),glm::vec3(-1.53976,-1.53826,9.64292),glm::vec3(0.647763,0.142861,14.8555));
		Triangle triangle97 = Triangle(glm::vec3(-1.53976,-1.53826,9.64292),glm::vec3(1.59175,-0.800509,14.5858),glm::vec3(1.17123,-0.240646,14.5924));
		Triangle triangle98 = Triangle(glm::vec3(1.17123,-0.240646,14.5924),glm::vec3(3.16801,1.38335,15.9008),glm::vec3(0.647763,0.142861,14.8555));
		Triangle triangle99 = Triangle(glm::vec3(-1.53976,-1.53826,9.64292),glm::vec3(1.17123,-0.240646,14.5924),glm::vec3(0.647763,0.142861,14.8555));
		Triangle triangle100 = Triangle(glm::vec3(-4.20891,0.946192,10.2808),glm::vec3(-2.02138,2.62732,15.4934),glm::vec3(-1.5037,1.3957,18.1764));
		Triangle triangle101 = Triangle(glm::vec3(-1.5037,1.3957,18.1764),glm::vec3(-2.02138,2.62732,15.4934),glm::vec3(-1.66406,2.03871,15.3662));
		Triangle triangle102 = Triangle(glm::vec3(-1.0774,1.68395,15.2237),glm::vec3(-4.20891,0.946192,10.2808),glm::vec3(-1.5037,1.3957,18.1764));
		Triangle triangle103 = Triangle(glm::vec3(-1.66406,2.03871,15.3662),glm::vec3(-2.02138,2.62732,15.4934),glm::vec3(-4.20891,0.946192,10.2808));
		Triangle triangle104 = Triangle(glm::vec3(-1.0774,1.68395,15.2237),glm::vec3(-1.5037,1.3957,18.1764),glm::vec3(-1.66406,2.03871,15.3662));
		Triangle triangle105 = Triangle(glm::vec3(-1.0774,1.68395,15.2237),glm::vec3(-1.66406,2.03871,15.3662),glm::vec3(-4.20891,0.946192,10.2808));
		Triangle triangle106 = Triangle(glm::vec3(-1.5037,1.3957,18.1764),glm::vec3(-2.02138,2.62732,15.4934),glm::vec3(-4.20891,0.946192,10.2808));
		Triangle triangle107 = Triangle(glm::vec3(-1.66406,2.03871,15.3662),glm::vec3(-2.02138,2.62732,15.4934),glm::vec3(-1.5037,1.3957,18.1764));
		Triangle triangle108 = Triangle(glm::vec3(-1.5037,1.3957,18.1764),glm::vec3(-4.20891,0.946192,10.2808),glm::vec3(-1.0774,1.68395,15.2237));
		Triangle triangle109 = Triangle(glm::vec3(-4.20891,0.946192,10.2808),glm::vec3(-2.02138,2.62732,15.4934),glm::vec3(-1.66406,2.03871,15.3662));
		Triangle triangle110 = Triangle(glm::vec3(-1.66406,2.03871,15.3662),glm::vec3(-1.5037,1.3957,18.1764),glm::vec3(-1.0774,1.68395,15.2237));
		Triangle triangle111 = Triangle(glm::vec3(-4.20891,0.946192,10.2808),glm::vec3(-1.66406,2.03871,15.3662),glm::vec3(-1.0774,1.68395,15.2237));
		Triangle triangle112 = Triangle(glm::vec3(-1.64479,-1.61627,9.54819),glm::vec3(0.542742,0.0648559,14.7608),glm::vec3(1.06043,-1.16676,17.4437));
		Triangle triangle113 = Triangle(glm::vec3(1.06043,-1.16676,17.4437),glm::vec3(0.542742,0.0648559,14.7608),glm::vec3(0.900064,-0.523747,14.6335));
		Triangle triangle114 = Triangle(glm::vec3(1.48673,-0.878513,14.4911),glm::vec3(-1.64479,-1.61627,9.54819),glm::vec3(1.06043,-1.16676,17.4437));
		Triangle triangle115 = Triangle(glm::vec3(0.900064,-0.523747,14.6335),glm::vec3(0.542742,0.0648559,14.7608),glm::vec3(-1.64479,-1.61627,9.54819));
		Triangle triangle116 = Triangle(glm::vec3(1.48673,-0.878513,14.4911),glm::vec3(1.06043,-1.16676,17.4437),glm::vec3(0.900064,-0.523747,14.6335));
		Triangle triangle117 = Triangle(glm::vec3(1.48673,-0.878513,14.4911),glm::vec3(0.900064,-0.523747,14.6335),glm::vec3(-1.64479,-1.61627,9.54819));
		Triangle triangle118 = Triangle(glm::vec3(1.06043,-1.16676,17.4437),glm::vec3(0.542742,0.0648559,14.7608),glm::vec3(-1.64479,-1.61627,9.54819));
		Triangle triangle119 = Triangle(glm::vec3(0.900064,-0.523747,14.6335),glm::vec3(0.542742,0.0648559,14.7608),glm::vec3(1.06043,-1.16676,17.4437));
		Triangle triangle120 = Triangle(glm::vec3(1.06043,-1.16676,17.4437),glm::vec3(-1.64479,-1.61627,9.54819),glm::vec3(1.48673,-0.878513,14.4911));
		Triangle triangle121 = Triangle(glm::vec3(-1.64479,-1.61627,9.54819),glm::vec3(0.542742,0.0648559,14.7608),glm::vec3(0.900064,-0.523747,14.6335));
		Triangle triangle122 = Triangle(glm::vec3(0.900064,-0.523747,14.6335),glm::vec3(1.06043,-1.16676,17.4437),glm::vec3(1.48673,-0.878513,14.4911));
		Triangle triangle123 = Triangle(glm::vec3(-1.64479,-1.61627,9.54819),glm::vec3(0.900064,-0.523747,14.6335),glm::vec3(1.48673,-0.878513,14.4911));
		Triangle triangle124 = Triangle(glm::vec3(2.50187,-1.0371,14.896),glm::vec3(1.18195,-0.618272,14.2096),glm::vec3(2.39793,-1.15917,14.9591));
		Triangle triangle125 = Triangle(glm::vec3(1.18195,-0.618272,14.2096),glm::vec3(1.98556,-2.06612,13.2039),glm::vec3(2.39793,-1.15917,14.9591));
		Triangle triangle126 = Triangle(glm::vec3(7.67746,-1.19943,18.5268),glm::vec3(2.50187,-1.0371,14.896),glm::vec3(2.39793,-1.15917,14.9591));
		Triangle triangle127 = Triangle(glm::vec3(2.50187,-1.0371,14.896),glm::vec3(1.22225,-0.566212,14.1862),glm::vec3(1.18195,-0.618272,14.2096));
		Triangle triangle128 = Triangle(glm::vec3(-0.64655,-0.375646,12.1392),glm::vec3(1.98556,-2.06612,13.2039),glm::vec3(1.18195,-0.618272,14.2096));
		Triangle triangle129 = Triangle(glm::vec3(2.39793,-1.15917,14.9591),glm::vec3(1.98556,-2.06612,13.2039),glm::vec3(4.56115,-2.08689,16.0498));
		Triangle triangle130 = Triangle(glm::vec3(2.39793,-1.15917,14.9591),glm::vec3(4.56115,-2.08689,16.0498),glm::vec3(7.67746,-1.19943,18.5268));
		Triangle triangle131 = Triangle(glm::vec3(2.50187,-1.0371,14.896),glm::vec3(7.67746,-1.19943,18.5268),glm::vec3(4.56115,-2.08689,16.0498));
		Triangle triangle132 = Triangle(glm::vec3(1.18195,-0.618272,14.2096),glm::vec3(1.22225,-0.566212,14.1862),glm::vec3(0.779692,0.303424,14.7557));
		Triangle triangle133 = Triangle(glm::vec3(1.22225,-0.566212,14.1862),glm::vec3(2.50187,-1.0371,14.896),glm::vec3(1.98556,-2.06612,13.2039));
		Triangle triangle134 = Triangle(glm::vec3(0.327535,0.093941,13.9059),glm::vec3(1.98556,-2.06612,13.2039),glm::vec3(-0.64655,-0.375646,12.1392));
		Triangle triangle135 = Triangle(glm::vec3(-0.64655,-0.375646,12.1392),glm::vec3(1.18195,-0.618272,14.2096),glm::vec3(0.18111,0.132881,14.2386));
		Triangle triangle136 = Triangle(glm::vec3(2.50187,-1.0371,14.896),glm::vec3(4.56115,-2.08689,16.0498),glm::vec3(1.98556,-2.06612,13.2039));
		Triangle triangle137 = Triangle(glm::vec3(1.18195,-0.618272,14.2096),glm::vec3(0.779692,0.303424,14.7557),glm::vec3(0.18111,0.132881,14.2386));
		Triangle triangle138 = Triangle(glm::vec3(0.327535,0.093941,13.9059),glm::vec3(1.22225,-0.566212,14.1862),glm::vec3(1.98556,-2.06612,13.2039));
		Triangle triangle139 = Triangle(glm::vec3(1.22225,-0.566212,14.1862),glm::vec3(0.327535,0.093941,13.9059),glm::vec3(0.779692,0.303424,14.7557));
		Triangle triangle140 = Triangle(glm::vec3(0.779692,0.303424,14.7557),glm::vec3(0.327535,0.093941,13.9059),glm::vec3(-0.64655,-0.375646,12.1392));
		Triangle triangle141 = Triangle(glm::vec3(0.779692,0.303424,14.7557),glm::vec3(1.22225,-0.566212,14.1862),glm::vec3(1.18195,-0.618272,14.2096));
		Triangle triangle142 = Triangle(glm::vec3(0.18111,0.132881,14.2386),glm::vec3(1.18195,-0.618272,14.2096),glm::vec3(-0.64655,-0.375646,12.1392));
		Triangle triangle143 = Triangle(glm::vec3(0.18111,0.132881,14.2386),glm::vec3(0.779692,0.303424,14.7557),glm::vec3(1.18195,-0.618272,14.2096));
		Triangle triangle144 = Triangle(glm::vec3(-4.10389,1.0242,10.3756),glm::vec3(-0.972383,1.76195,15.3184),glm::vec3(0.603877,3.94581,16.6334));
		Triangle triangle145 = Triangle(glm::vec3(0.603877,3.94581,16.6334),glm::vec3(-0.972383,1.76195,15.3184),glm::vec3(-1.3929,2.32182,15.325));
		Triangle triangle146 = Triangle(glm::vec3(-1.91637,2.70532,15.5882),glm::vec3(-4.10389,1.0242,10.3756),glm::vec3(0.603877,3.94581,16.6334));
		Triangle triangle147 = Triangle(glm::vec3(-1.3929,2.32182,15.325),glm::vec3(-0.972383,1.76195,15.3184),glm::vec3(-4.10389,1.0242,10.3756));
		Triangle triangle148 = Triangle(glm::vec3(-1.91637,2.70532,15.5882),glm::vec3(0.603877,3.94581,16.6334),glm::vec3(-1.3929,2.32182,15.325));
		Triangle triangle149 = Triangle(glm::vec3(-1.91637,2.70532,15.5882),glm::vec3(-1.3929,2.32182,15.325),glm::vec3(-4.10389,1.0242,10.3756));
		Triangle triangle150 = Triangle(glm::vec3(0.603877,3.94581,16.6334),glm::vec3(-0.972383,1.76195,15.3184),glm::vec3(-4.10389,1.0242,10.3756));
		Triangle triangle151 = Triangle(glm::vec3(-1.3929,2.32182,15.325),glm::vec3(-0.972383,1.76195,15.3184),glm::vec3(0.603877,3.94581,16.6334));
		Triangle triangle152 = Triangle(glm::vec3(0.603877,3.94581,16.6334),glm::vec3(-4.10389,1.0242,10.3756),glm::vec3(-1.91637,2.70532,15.5882));
		Triangle triangle153 = Triangle(glm::vec3(-4.10389,1.0242,10.3756),glm::vec3(-0.972383,1.76195,15.3184),glm::vec3(-1.3929,2.32182,15.325));
		Triangle triangle154 = Triangle(glm::vec3(-1.3929,2.32182,15.325),glm::vec3(0.603877,3.94581,16.6334),glm::vec3(-1.91637,2.70532,15.5882));
		Triangle triangle155 = Triangle(glm::vec3(-4.10389,1.0242,10.3756),glm::vec3(-1.3929,2.32182,15.325),glm::vec3(-1.91637,2.70532,15.5882));



		triangle1.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle2.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle3.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle4.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle5.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle6.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle7.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle8.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle9.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle10.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle11.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle12.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle13.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle14.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle15.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle16.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle17.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle18.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle19.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle20.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle21.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle22.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle23.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle24.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle25.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle26.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle27.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle28.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle29.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle30.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle31.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle32.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle33.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle34.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle35.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle36.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle37.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle38.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle39.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle40.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle41.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle42.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle43.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle44.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle45.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle46.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle47.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle48.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle49.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle50.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle51.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle52.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle53.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle54.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle55.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle56.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle57.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle58.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle59.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle60.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle61.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle62.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle63.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle64.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle65.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle66.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle67.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle68.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle69.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle70.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle71.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle72.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle73.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle74.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle75.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle76.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle77.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle78.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle79.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle80.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle81.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle82.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle83.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle84.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle85.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle86.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle87.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle88.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle89.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle90.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle91.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle92.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle93.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle94.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle95.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle96.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle97.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle98.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle99.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle100.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle101.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle102.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle103.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle104.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle105.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle106.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle107.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle108.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle109.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle110.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle111.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle112.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle113.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle114.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle115.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle116.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle117.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle118.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle119.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle120.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle121.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle122.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle123.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle124.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle125.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle126.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle127.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle128.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle129.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle130.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle131.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle132.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle133.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle134.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle135.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle136.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle137.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle138.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle139.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle140.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle141.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle142.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle143.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle144.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle145.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle146.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle147.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle148.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle149.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle150.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle151.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle152.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle153.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle154.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);
		triangle155.setProperties(ka, ia, bDif, glm::vec3(0.6,0.6,0.6), bSpec, glm::vec3(1.0,0.0,0.0), 1, bRef, 0);


		triangleObjects.push_back(triangle1);
		triangleObjects.push_back(triangle2);
		triangleObjects.push_back(triangle3);
		triangleObjects.push_back(triangle4);
		triangleObjects.push_back(triangle5);
		triangleObjects.push_back(triangle6);
		triangleObjects.push_back(triangle7);
		triangleObjects.push_back(triangle8);
		triangleObjects.push_back(triangle9);
		triangleObjects.push_back(triangle10);
		triangleObjects.push_back(triangle11);
		triangleObjects.push_back(triangle12);
		triangleObjects.push_back(triangle13);
		triangleObjects.push_back(triangle14);
		triangleObjects.push_back(triangle15);
		triangleObjects.push_back(triangle16);
		triangleObjects.push_back(triangle17);
		triangleObjects.push_back(triangle18);
		triangleObjects.push_back(triangle19);
		triangleObjects.push_back(triangle20);
		triangleObjects.push_back(triangle21);
		triangleObjects.push_back(triangle22);
		triangleObjects.push_back(triangle23);
		triangleObjects.push_back(triangle24);
		triangleObjects.push_back(triangle25);
		triangleObjects.push_back(triangle26);
		triangleObjects.push_back(triangle27);
		triangleObjects.push_back(triangle28);
		triangleObjects.push_back(triangle29);
		triangleObjects.push_back(triangle30);
		triangleObjects.push_back(triangle31);
		triangleObjects.push_back(triangle32);
		triangleObjects.push_back(triangle33);
		triangleObjects.push_back(triangle34);
		triangleObjects.push_back(triangle35);
		triangleObjects.push_back(triangle36);
		triangleObjects.push_back(triangle37);
		triangleObjects.push_back(triangle38);
		triangleObjects.push_back(triangle39);
		triangleObjects.push_back(triangle40);
		triangleObjects.push_back(triangle41);
		triangleObjects.push_back(triangle42);
		triangleObjects.push_back(triangle43);
		triangleObjects.push_back(triangle44);
		triangleObjects.push_back(triangle45);
		triangleObjects.push_back(triangle46);
		triangleObjects.push_back(triangle47);
		triangleObjects.push_back(triangle48);
		triangleObjects.push_back(triangle49);
		triangleObjects.push_back(triangle50);
		triangleObjects.push_back(triangle51);
		triangleObjects.push_back(triangle52);
		triangleObjects.push_back(triangle53);
		triangleObjects.push_back(triangle54);
		triangleObjects.push_back(triangle55);
		triangleObjects.push_back(triangle56);
		triangleObjects.push_back(triangle57);
		triangleObjects.push_back(triangle58);
		triangleObjects.push_back(triangle59);
		triangleObjects.push_back(triangle60);
		triangleObjects.push_back(triangle61);
		triangleObjects.push_back(triangle62);
		triangleObjects.push_back(triangle63);
		triangleObjects.push_back(triangle64);
		triangleObjects.push_back(triangle65);
		triangleObjects.push_back(triangle66);
		triangleObjects.push_back(triangle67);
		triangleObjects.push_back(triangle68);
		triangleObjects.push_back(triangle69);
		triangleObjects.push_back(triangle70);
		triangleObjects.push_back(triangle71);
		triangleObjects.push_back(triangle72);
		triangleObjects.push_back(triangle73);
		triangleObjects.push_back(triangle74);
		triangleObjects.push_back(triangle75);
		triangleObjects.push_back(triangle76);
		triangleObjects.push_back(triangle77);
		triangleObjects.push_back(triangle78);
		triangleObjects.push_back(triangle79);
		triangleObjects.push_back(triangle80);
		triangleObjects.push_back(triangle81);
		triangleObjects.push_back(triangle82);
		triangleObjects.push_back(triangle83);
		triangleObjects.push_back(triangle84);
		triangleObjects.push_back(triangle85);
		triangleObjects.push_back(triangle86);
		triangleObjects.push_back(triangle87);
		triangleObjects.push_back(triangle88);
		triangleObjects.push_back(triangle89);
		triangleObjects.push_back(triangle90);
		triangleObjects.push_back(triangle91);
		triangleObjects.push_back(triangle92);
		triangleObjects.push_back(triangle93);
		triangleObjects.push_back(triangle94);
		triangleObjects.push_back(triangle95);
		triangleObjects.push_back(triangle96);
		triangleObjects.push_back(triangle97);
		triangleObjects.push_back(triangle98);
		triangleObjects.push_back(triangle99);
		triangleObjects.push_back(triangle100);
		triangleObjects.push_back(triangle101);
		triangleObjects.push_back(triangle102);
		triangleObjects.push_back(triangle103);
		triangleObjects.push_back(triangle104);
		triangleObjects.push_back(triangle105);
		triangleObjects.push_back(triangle106);
		triangleObjects.push_back(triangle107);
		triangleObjects.push_back(triangle108);
		triangleObjects.push_back(triangle109);
		triangleObjects.push_back(triangle110);
		triangleObjects.push_back(triangle111);
		triangleObjects.push_back(triangle112);
		triangleObjects.push_back(triangle113);
		triangleObjects.push_back(triangle114);
		triangleObjects.push_back(triangle115);
		triangleObjects.push_back(triangle116);
		triangleObjects.push_back(triangle117);
		triangleObjects.push_back(triangle118);
		triangleObjects.push_back(triangle119);
		triangleObjects.push_back(triangle120);
		triangleObjects.push_back(triangle121);
		triangleObjects.push_back(triangle122);
		triangleObjects.push_back(triangle123);
		triangleObjects.push_back(triangle124);
		triangleObjects.push_back(triangle125);
		triangleObjects.push_back(triangle126);
		triangleObjects.push_back(triangle127);
		triangleObjects.push_back(triangle128);
		triangleObjects.push_back(triangle129);
		triangleObjects.push_back(triangle130);
		triangleObjects.push_back(triangle131);
		triangleObjects.push_back(triangle132);
		triangleObjects.push_back(triangle133);
		triangleObjects.push_back(triangle134);
		triangleObjects.push_back(triangle135);
		triangleObjects.push_back(triangle136);
		triangleObjects.push_back(triangle137);
		triangleObjects.push_back(triangle138);
		triangleObjects.push_back(triangle139);
		triangleObjects.push_back(triangle140);
		triangleObjects.push_back(triangle141);
		triangleObjects.push_back(triangle142);
		triangleObjects.push_back(triangle143);
		triangleObjects.push_back(triangle144);
		triangleObjects.push_back(triangle145);
		triangleObjects.push_back(triangle146);
		triangleObjects.push_back(triangle147);
		triangleObjects.push_back(triangle148);
		triangleObjects.push_back(triangle149);
		triangleObjects.push_back(triangle150);
		triangleObjects.push_back(triangle151);
		triangleObjects.push_back(triangle152);
		triangleObjects.push_back(triangle153);
		triangleObjects.push_back(triangle154);
		triangleObjects.push_back(triangle155);

	}
}



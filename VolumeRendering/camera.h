/*
*
* FileName: camera.cs
* Written by Arun Babu Madhavan for CS6366, assignment 2, starting Feb 23, 2019.
* NetID: axm170039
*
*
* Author:  Arun Babu Madhavan
*
*/

#ifndef CAMERA_H
#define CAMERA_H


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>


//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


//Initial camera position in degrees
const GLfloat YAW = 0.0f;
const GLfloat PITCH = 0.0f;
const GLfloat ROLL = 0.0f;


//Camera class to process the  inputs and translates the camera
class Camera
{
public:
	glm::vec4 _position;
	glm::vec3 _front;
	glm::vec3 move;
	glm::vec3 m_position;
	//Constructor
	Camera(glm::vec3 position = glm::vec3(0.0f),
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
		float yaw = YAW,
		float pitch = PITCH,
		float roll = ROLL)
	{
		resetCamera(position, up, yaw, pitch, roll);
	}

	void resetCamera(glm::vec3 position = glm::vec3(0.0f),
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW,
		float pitch = PITCH,
		float roll = ROLL)
	{
		_yaw = yaw;
		_pitch = pitch;
		_roll = roll;
		m_position = position;
		_position = glm::vec4(position, 1.0f);
		_up = up;

		move = glm::vec3(0.0f);
		_front.x = cos(glm::radians(_pitch)) * cos(glm::radians(_yaw));
		_front.y = sin(glm::radians(_pitch));
		_front.z = cos(glm::radians(_pitch)) * sin(glm::radians(_yaw));
		_front = glm::normalize(_front);

		updateCameraVectors();
	}

	//Add pitch in degrees
	void addPitch(GLfloat value) {
		_pitch = _pitch + value;
		updateCameraVectors();
	}

	//Add roll in degrees
	void addRoll(GLfloat value) {
		_roll = _roll + value;
		updateCameraVectors();
	}

	//Add yaw in degrees
	void addYaw(GLfloat value) {
		_yaw = _yaw + value;
		updateCameraVectors();
	}

	void setRotationAngles(GLfloat pitch, GLfloat yaw, GLfloat roll) {
		_pitch = pitch;
		_yaw = yaw;
		_roll = roll;
		updateCameraVectors();
	}

	void setPosition(glm::vec4 pos) {
		_position = pos;
		move = glm::vec3(0.0f);
		updateCameraVectors();
	}
	//Return the view matrix
	glm::mat4 GetViewMatrix()
	{
		glm::mat4 translate = glm::translate(glm::mat4(1.0f), move);
		glm::vec3 camPosition = m_position + move;  // translate the camera position by the movement vector

		glm::mat4 view = glm::lookAt(camPosition, camPosition + _front, _up);
		return view;
	}
	
	void processMouseScroll(float yoffset) {
		_front.z += (yoffset) / 50.0f;
	}

private:
	glm::vec3 _up;

	GLfloat _yaw;
	GLfloat _pitch;
	GLfloat _roll;
	void updateCameraVectors() {
		glm::vec3 right;

		glm::mat4 R = glm::yawPitchRoll(glm::radians(_yaw), glm::radians(_pitch), glm::radians(_roll));

		_front = glm::vec3(R*glm::vec4(0, 0, -1, 0));
		_up = glm::vec3(R*glm::vec4(0, 1, 0, 0));
		_up = glm::normalize(_up);
		right = glm::cross(_front, _up);

	}
};

#endif
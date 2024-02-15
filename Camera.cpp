#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        //TODO
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(cameraUp, cameraFrontDirection));
    }
    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        //TODO

        return glm::lookAt(cameraPosition, cameraTarget, this->cameraUpDirection);
    }
    
    void Camera::setPosition(glm::vec3 cameraPositionPar) {
        this->cameraPosition = cameraPositionPar;
    }
    glm::vec3 Camera::getPosition() {
        return this->cameraPosition;
    }
    glm::vec3 Camera::getFront() {
        return this->cameraFrontDirection;
    }
    void Camera::setFront(glm::vec3 cameraFront) {
        this->cameraFrontDirection = cameraFront;
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        //TODO
        if (direction == gps::MOVE_FORWARD)
        {
            cameraPosition += speed * cameraFrontDirection;
            //cameraTarget = cameraPosition + cameraFrontDirection;
        }
        if (direction == gps::MOVE_BACKWARD)
        {
            cameraPosition -= speed * cameraFrontDirection;
            //cameraTarget = cameraPosition + cameraFrontDirection;
        }
        if (direction == gps::MOVE_LEFT)
        {
            cameraPosition -= speed * cameraRightDirection;
            //cameraTarget = cameraPosition - cameraRightDirection;
        }
        if (direction == gps::MOVE_RIGHT)
        {
            cameraPosition += speed * cameraRightDirection;
            //cameraTarget = cameraPosition + cameraRightDirection;
        }
        cameraTarget = cameraPosition + cameraFrontDirection;
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        //TODO
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        cameraFrontDirection = glm::normalize(direction);

        cameraTarget = cameraPosition + cameraFrontDirection;
        // also re-calculate the Right and Up vector
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        //cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
    }
}

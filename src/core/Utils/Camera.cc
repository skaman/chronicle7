#include "Camera.h"

namespace chronicle {

Camera::Camera()
    : _fov(45.0f)
    , _aspect(1920.0f / 1080.0f)
    , _zNear(0.1f)
    , _zFar(100.0f)
{
    recalculateProjection();
    recalculateView();
}

Camera::Camera(float fov, float aspect, float zNear, float zFar)
    : _fov(fov)
    , _aspect(aspect)
    , _zNear(zNear)
    , _zFar(zFar)
{
    recalculateProjection();
    recalculateView();
}

void Camera::moveForward(float offset)
{
    _cameraPos += glm::vec3(-1.0f, -1.0f, 0.0f) * _cameraSpeed * offset;
    _cameraFront += glm::vec3(-1.0f, -1.0f, 0.0f) * _cameraSpeed * offset;
}

void Camera::panHorizontal(float offset)
{
    _cameraPos += glm::vec3(1.0f, -1.0f, 0.0f) * _cameraSpeed * offset;
    _cameraFront += glm::vec3(1.0f, -1.0f, 0.0f) * _cameraSpeed * offset;
}

void Camera::panVertical(float offset)
{
    _cameraPos += glm::vec3(0.0f, 0.0f, -1.0f) * _cameraSpeed * offset;
    _cameraFront += glm::vec3(0.0f, 0.0f, -1.0f) * _cameraSpeed * offset;
}

void Camera::rotateHorizontal(float offset) { _cameraFront += glm::vec3(-1.0f, 1.0f, 0.0f) * _cameraSpeed * offset; }

void Camera::rotateVertical(float offset) { _cameraFront += glm::vec3(0.0f, 0.0f, 1.0f) * _cameraSpeed * offset; }

void Camera::recalculateView() { _view = glm::lookAt(_cameraPos, _cameraFront, _cameraUp); }

void Camera::recalculateProjection()
{
    _projection = glm::perspective(glm::radians(_fov), _aspect, _zNear, _zFar);
    _projection[1][1] *= -1;
}

} // namespace chronicle
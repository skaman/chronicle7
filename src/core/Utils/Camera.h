// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

namespace chronicle {

class Camera {
public:
    explicit Camera();
    explicit Camera(float fov, float aspect, float zNear, float zFar);

    void moveForward(float offset);
    void panHorizontal(float offset);
    void panVertical(float offset);
    void rotateHorizontal(float offset);
    void rotateVertical(float offset);

    [[nodiscard]] float fov() const { return _fov; }
    void setFov(float fov) { _fov = fov; }

    [[nodiscard]] float aspect() const { return _aspect; }
    void setAspect(float aspect) { _aspect = aspect; }

    [[nodiscard]] float zNear() const { return _zNear; }
    void setZNear(float zNear) { _zNear = zNear; }

    [[nodiscard]] float zFar() const { return _zFar; }
    void setZFar(float zFar) { _zFar = zFar; }

    [[nodiscard]] glm::mat4 view() const { return _view; }
    [[nodiscard]] glm::mat4 projection() const { return _projection; }

    void recalculateView();
    void recalculateProjection();

private:
    float _fov;
    float _aspect;
    float _zNear;
    float _zFar;

    float _cameraSpeed = 2.0f;

    glm::vec3 _cameraPos = glm::vec3(2.0f, 2.0f, 2.0f);
    glm::vec3 _cameraFront = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 _cameraUp = glm::vec3(0.0f, 0.0f, 1.0f);

    glm::mat4 _view;
    glm::mat4 _projection;
};

} // namespace chronicle
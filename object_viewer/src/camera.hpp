#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

class camera {
public:
    static inline const float YAW = -90.0f;
    static inline const float PITCH = 0.0f;
    static inline const float SPEED = 2.5f;
    static inline const float SENSITIVITY = 0.1f;
    static inline const float ZOOM = 100.0f;

    enum class direction {
        forward,
        backward,
        left,
        right
    };

    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right = glm::vec3(0.0f);
    glm::vec3 world_up;
    float yaw;
    float pitch;
    float speed;
    float sensitivity;
    float zoom;

    explicit camera(
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = YAW,
        float pitch = PITCH
    ): position(position), up(up), world_up(up), yaw(yaw), pitch(pitch), front(glm::vec3(0.0f, 0.0f, -1.0f)),
        speed(SPEED), sensitivity(SENSITIVITY), zoom(ZOOM / 2) {
        update();
    }

    [[nodiscard]]
    glm::mat4 get_view_matrix() const {
        return glm::lookAt(position, position + front, up);
    }

    void keyboard(direction direction, float deltaTime) {
        float velocity = speed * deltaTime;
        if (direction == direction::forward)
            position += front * velocity;
        if (direction == direction::backward)
            position -= front * velocity;
        if (direction == direction::left)
            position -= right * velocity;
        if (direction == direction::right)
            position += right * velocity;
    }

    void mouse_move(float xoffset, float yoffset, GLboolean constrainPitch = true) {
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        if (constrainPitch) {
            if (pitch > 89.0f)
                pitch = 89.0f;
            if (pitch < -89.0f)
                pitch = -89.0f;
        }

        update();
    }

    void mouse_scroll(float yoffset) {
        zoom -= yoffset;
        if (zoom < 1.0f) {
            zoom = 1.0f;
        }
        if (zoom > ZOOM) {
            zoom = ZOOM;
        }
    }

private:
    void update() {
        front = glm::normalize(glm::vec3(
            std::cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
            std::sin(glm::radians(pitch)),
            std::sin(glm::radians(yaw)) * cos(glm::radians(pitch))
        ));
        right = glm::normalize(glm::cross(front, world_up));
        up = glm::normalize(glm::cross(right, front));
    }
};

#endif
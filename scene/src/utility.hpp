#ifndef UTILITY_HPP
#define UTILITY_HPP

#pragma once

namespace utility {
    auto get_window_size(const GLFWwindow* window) {
        int width;
        int height;
        glfwGetFramebufferSize((GLFWwindow*)window, &width, &height);
        return std::make_tuple(width, height);
    }
}

#endif

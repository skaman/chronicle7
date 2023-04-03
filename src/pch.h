#pragma once

// renderer defines
#if RENDERER == VULKAN
#define VULKAN_RENDERER
#endif

// vulkan hpp
#ifdef VULKAN_RENDERER
#include <vulkan/vulkan.hpp>
#endif

// glfw
#include <GLFW/glfw3.h>

// spdlog
#include <spdlog/spdlog.h>

// glm
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// entt
#include <entt/entt.hpp>

// tracy
#include <tracy/Tracy.hpp>

// std lib
#include <bit>
#include <chrono>
#include <fstream>
#include <iostream>
#include <optional>
#include <set>
#include <string>
#include <vector>

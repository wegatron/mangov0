#pragma once

#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

#define LOGI(...) spdlog::info(__VA_ARGS__);
#define LOGW(...) spdlog::warn(__VA_ARGS__);
#define LOGE(...) spdlog::error("[{}:{}] {}", __FILE__, __LINE__, __VA_ARGS__);
#define LOGF(...) spdlog::critical("[{}:{}] {}", __FILE__, __LINE__, __VA_ARGS__);
#define LOGD(...) spdlog::debug(__VA_ARGS__);
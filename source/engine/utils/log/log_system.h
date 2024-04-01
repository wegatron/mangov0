#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/ringbuffer_sink.h>
#include <stdexcept>
#include <memory>

namespace mango
{
	enum class ELogLevel
	{
		Debug, Info, Warning, Error, Fatal
	};

    class LogSystem
    {
        public:
            void init();
            void destroy();
			std::vector<std::string> getLastestLogs();

			std::shared_ptr<spdlog::logger> getLogger() { return m_logger; }

        private:
			std::string getLogFilename();

			std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> m_ringbuffer_sink;
            std::shared_ptr<spdlog::logger> m_logger;
    };
}

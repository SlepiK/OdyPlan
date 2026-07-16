#ifndef SIERRA_ODYPLAN_V2_COMMON_IO_IOREGISTRY_H
#define SIERRA_ODYPLAN_V2_COMMON_IO_IOREGISTRY_H

#include <functional>
#include <memory>
#include <nlohmann/json.hpp>

#include "io_format.h"
#include "io_sink.h"
#include "io_source.h"
#include "json/json_sink.h"
#include "json/json_source.h"


namespace sierra::odyplan::v2::common::io {
    class IORegistry {
    public:
        using IOSourceFactory = std::function<std::unique_ptr<IIOSource>(const nlohmann::json&)>;
        using IOSinkFactory = std::function<std::unique_ptr<IIOSink>(const nlohmann::json&)>;

        void registerSourceIO(const IOFormat format, IOSourceFactory factory) {
            if (mIOSourceFactory.contains(format)) {
                throw std::invalid_argument("IORegistry: source is already registered");
            }
            mIOSourceFactory.emplace(format, std::move(factory));
        }

        void registerSinkIO(const IOFormat format, IOSinkFactory factory) {
            if (mIOSinkFactory.contains(format)) {
                throw std::invalid_argument("IORegistry: sink is already registered");
            }
            mIOSinkFactory.emplace(format, std::move(factory));
        }

        [[nodiscard]] std::unique_ptr<IIOSource> createSourceIO(const IOFormat format, const nlohmann::json& config) const {
            const auto iter = mIOSourceFactory.find(format);
            if (iter == mIOSourceFactory.end()) {
                throw std::invalid_argument("IORegistry: source is not registered" + std::to_string(std::to_underlying(format)));
            }
            return iter->second(config);
        }

        [[nodiscard]] std::unique_ptr<IIOSink> createSinkIO(const IOFormat format, const nlohmann::json& config) const {
            const auto iter = mIOSinkFactory.find(format);
            if (iter == mIOSinkFactory.end()) {
                throw std::invalid_argument("IORegistry: sink is not registered" + std::to_string(std::to_underlying(format)));
            }
            return iter->second(config);
        }

        static IORegistry withDefaults() {
            IORegistry registry;
            registry.registerSourceIO(IOFormat::JSON, [](const nlohmann::json& config) {
                return std::make_unique<JsonSource>(config.at("path").get<std::string>());
            });
            registry.registerSinkIO(IOFormat::JSON, [](const nlohmann::json& config) {
                return std::make_unique<JsonSink>(config.at("path").get<std::string>());
            });
            return registry;
        }

        [[nodiscard]] bool hasSourceIO(const IOFormat format) const {
            return mIOSourceFactory.contains(format);
        }

        [[nodiscard]] bool hasSinkIO(const IOFormat format) const {
            return mIOSinkFactory.contains(format);
        }
    private:
        std::unordered_map<IOFormat, IOSourceFactory> mIOSourceFactory;
        std::unordered_map<IOFormat, IOSinkFactory> mIOSinkFactory;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_IO_IOREGISTRY_H
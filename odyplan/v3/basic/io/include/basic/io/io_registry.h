#ifndef SIERRA_ODYPLAN_V3_BASIC_IO_IO_REGISTRY_HPP
#define SIERRA_ODYPLAN_V3_BASIC_IO_IO_REGISTRY_HPP

#include <nlohmann/json.hpp>

#include <basic/io/io_format.h>
#include <basic/io/io_sink.h>
#include <basic/io/io_source.h>
#include <basic/io/json/json_sink.h>
#include <basic/io/json/json_source.h>
#include <basic/io/ndjson/ndjson_sink.h>
#include <basic/io/ndjson/ndjson_source.h>
#include <functional>
#include <memory>
#include <utility>

namespace sierra::odyplan::v3::basic::io {
    class IORegistry {
    public:
        using IOSourceFactory = std::function<std::unique_ptr<IIOSource>(const nlohmann::json&)>;
        using IOSinkFactory = std::function<std::unique_ptr<IIOSink>(const nlohmann::json&)>;

        void registerSourceIO(const IOFormatType format, IOSourceFactory factory) {
            if (mIOSourceFactory.contains(format)) {
                throw std::invalid_argument("IORegistry: source is already registered");
            }
            mIOSourceFactory.emplace(format, std::move(factory));
        }

        void registerSourceIO(const BuiltinIOFormat format, IOSourceFactory factory) {
            registerSourceIO(static_cast<IOFormatType>(format), std::move(factory));
        }

        void registerSinkIO(const IOFormatType format, IOSinkFactory factory) {
            if (mIOSinkFactory.contains(format)) {
                throw std::invalid_argument("IORegistry: sink is already registered");
            }
            mIOSinkFactory.emplace(format, std::move(factory));
        }

        void registerSinkIO(const BuiltinIOFormat format, IOSinkFactory factory) {
            registerSinkIO(static_cast<IOFormatType>(format), std::move(factory));
        }

        [[nodiscard]] std::unique_ptr<IIOSource>
        createSourceIO(const IOFormatType format, const nlohmann::json& config) const {
            const auto iter = mIOSourceFactory.find(format);
            if (iter == mIOSourceFactory.end()) {
                throw std::invalid_argument("IORegistry: source is not registered" +
                                            std::to_string(format));
            }
            return iter->second(config);
        }

        [[nodiscard]] std::unique_ptr<IIOSource>
        createSourceIO(const BuiltinIOFormat format, const nlohmann::json& config) const {
            return createSourceIO(static_cast<IOFormatType>(format), config);
        }

        [[nodiscard]] std::unique_ptr<IIOSink> createSinkIO(const IOFormatType format,
                                                            const nlohmann::json& config) const {
            const auto iter = mIOSinkFactory.find(format);
            if (iter == mIOSinkFactory.end()) {
                throw std::invalid_argument("IORegistry: sink is not registered" +
                                            std::to_string(format));
            }
            return iter->second(config);
        }

        [[nodiscard]] std::unique_ptr<IIOSink> createSinkIO(const BuiltinIOFormat format,
                                                            const nlohmann::json& config) const {
            return createSinkIO(static_cast<IOFormatType>(format), config);
        }

        static IORegistry withBuiltins() {
            IORegistry registry;
            registry.registerSourceIO(BuiltinIOFormat::JSON, [](const nlohmann::json& config) {
                return std::make_unique<json::JSONSource>(config.at("path").get<std::string>());
            });
            registry.registerSinkIO(BuiltinIOFormat::JSON, [](const nlohmann::json& config) {
                return std::make_unique<json::JSONSink>(config.at("path").get<std::string>());
            });
            registry.registerSourceIO(BuiltinIOFormat::NDJSON, [](const nlohmann::json& config) {
                return std::make_unique<ndjson::NDJSONSource>(config.at("path").get<std::string>());
            });
            registry.registerSinkIO(BuiltinIOFormat::NDJSON, [](const nlohmann::json& config) {
                return std::make_unique<ndjson::NDJSONSink>(config.at("path").get<std::string>());
            });
            return registry;
        }

        [[nodiscard]] bool hasSourceIO(const IOFormatType format) const {
            return mIOSourceFactory.contains(format);
        }

        [[nodiscard]] bool hasSinkIO(const IOFormatType format) const {
            return mIOSinkFactory.contains(format);
        }

    private:
        std::unordered_map<IOFormatType, IOSourceFactory> mIOSourceFactory;
        std::unordered_map<IOFormatType, IOSinkFactory> mIOSinkFactory;
    };
}  // namespace sierra::odyplan::v3::basic::io

#endif  // SIERRA_ODYPLAN_V3_BASIC_IO_IO_REGISTRY_HPP

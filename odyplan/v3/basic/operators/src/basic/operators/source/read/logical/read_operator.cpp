#include <basic/operators/source/read/logical/read_operator.h>

#include <utility>

namespace sierra::odyplan::v3::basic::operators::source::read::logical {
    ReadOperator::ReadOperator(core::logical::port::Port outputPort,
                               const io::IOFormatType ioFormat,
                               nlohmann::json ioConfig,
                               details::LReadOperatorOptions options)
        : TypedSourceOperator(std::move(outputPort)),
          mIOFormat(ioFormat),
          mIOConfig(std::move(ioConfig)),
          mOptions(std::move(options)) {}


    std::unique_ptr<ReadOperator> ReadOperator::fromAutoSchema(const io::IOFormatType ioFormat,
                 nlohmann::json ioConfig,
                 details::LReadOperatorOptions options) {
        static const auto ioRegistry = io::IORegistry::withBuiltins();
        if (!ioRegistry.hasSchemaInference(ioFormat)) {
            throw std::runtime_error("ReadOperator: no schema auto-inference available for this format");
        }
        auto inferred = ioRegistry.inferSchema(ioFormat, ioConfig);
        return std::make_unique<ReadOperator>(std::move(inferred), ioFormat, std::move(ioConfig), std::move(options));
    }

    core::shared::types::OperatorTypeId ReadOperator::getOperatorTypeId() const {
        return TYPE_ID;
    }

    io::IOFormatType ReadOperator::getIOFormat() const {
        return mIOFormat;
    }

    const nlohmann::json& ReadOperator::getIOConfig() const {
        return mIOConfig;
    }

    details::PReadOperatorOptions ReadOperator::getOptions() const {
        return details::resolve(mOptions, getOutputPorts().front());
    }

    void ReadOperator::validate() const {
    }
}  // namespace sierra::odyplan::v3::basic::operators::source::read::logical
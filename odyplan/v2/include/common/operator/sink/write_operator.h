#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_WRITEOPERATOR_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_WRITEOPERATOR_H

#include <memory>
#include <utility>

#include "common/io/io_sink.h"
#include "core/common/identifier/operator.h"

namespace sierra::odyplan::v2::common::opr::sink {
    namespace logical {
        class WriteOperator {
        public:
            static constexpr core::common::identifier::OperatorTypeId TYPE_ID = std::to_underlying(core::common::identifier::Operator::WRITE);

            explicit WriteOperator(std::shared_ptr<io::IIOSink> dataSink)
                : mDataSink(std::move(dataSink)) {
                if (!mDataSink) {
                    throw std::invalid_argument("WriteOperator: dataSink must not be null");
                }
            }

            [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
                return TYPE_ID;
            }

            [[nodiscard]] const std::shared_ptr<io::IIOSink> getDataSink() const {
                return mDataSink;
            }
        private:
            std::shared_ptr<io::IIOSink> mDataSink;
        };
    }

    namespace physical {
        class WriteOperator {
        public:
            WriteOperator() {

            }
        protected:
        private:
        };
    }
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_WRITEOPERATOR_H
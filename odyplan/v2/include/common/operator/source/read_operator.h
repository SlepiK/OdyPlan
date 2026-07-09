#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SOURCE_READOPERATOR_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SOURCE_READOPERATOR_H
#include "common/io/io_data.h"
#include "common/io/io_source.h"
#include "core/logical/operator/source/source_operator.h"
#include "core/physical/operator/source/source_operator.h"

namespace sierra::odyplan::v2::common::opr::source {
    namespace logical {
        class ReadOperator : core::logical::opr::TypedSourceOperator {
        public:
            static constexpr core::common::identifier::OperatorTypeId TYPE_ID = std::to_underlying(core::common::identifier::Operator::READ);

            ReadOperator(core::logical::port::Port outputPort, std::shared_ptr<io::IIOSource> dataSource)
                : TypedSourceOperator(std::move(outputPort)),
                    mDataSource(std::move(dataSource)) {
                if (!mDataSource) {
                    throw std::invalid_argument("ReadOperator: dataSource must not be null");
                }
            }

            [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
                return TYPE_ID;
            }

            [[nodiscard]] const std::shared_ptr<io::IIOSource> getDataSource() const {
                return mDataSource;
            }
        protected:
        private:
            std::shared_ptr<io::IIOSource> mDataSource;
        };
    }

    namespace physical {
        class ReadOperator : core::physical::opr::TypedSourceOperator {
        public:
        protected:
        private:
        };
    }
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SOURCE_READOPERATOR_H
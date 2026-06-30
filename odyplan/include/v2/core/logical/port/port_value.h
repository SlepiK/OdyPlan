#ifndef SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORTTYPE_H
#define SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORTTYPE_H

#include <cstdint>
#include <memory>

namespace sierra::odyplan::v2::core::logical::port {
    class PortValueBase {
    public:
        explicit PortValueBase(const std::uint32_t id)
            : mStableId(id) {
        }

        virtual ~PortValueBase() {}

        PortValueBase(const PortValueBase&) = delete;
        PortValueBase& operator=(const PortValueBase&) = delete;

        virtual std::unique_ptr<PortValueBase> clone() = 0;

        std::uint32_t getStableId() const {
            return mStableId;
        }

    private:
        const std::uint32_t mStableId;
    };

    template <typename T>
    bool isa(const PortValueBase* pt) {
        return pt && pt->getStableId() == T::STABLE_ID;
    }

    template <typename T>
    T* cast(PortValueBase* pt) {
        assert(isa<T>(pt));
        return static_cast<T*>(pt);
    }

    template <typename T>
    const T* cast(const PortValueBase* pt) {
        assert(isa<T>(pt));
        return static_cast<const T*>(pt);
    }

    template <typename T>
    T* dCast (PortValueBase* pt) {
        return isa<T>(pt) ? static_cast<T*>(pt) : nullptr;
    }

    template <typename T>
    const T* dCast(const PortValueBase* pt) {
        return isa<T>(pt) ? static_cast<const T*>(pt) : nullptr;
    }
}  // namespace sierra::odyplan::v2::core::logical::port

#endif  // SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORTTYPE_H
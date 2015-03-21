#pragma once
#include <boost/utility.hpp>
#include <cstddef>
#include <cstring>
#include <type_traits>
#include <utility>
#include <vector>

namespace mill {
    class Type;

    struct Value {
        Type* type;
    };

    struct Field {
        Field(Type* type, std::size_t offset) : type(type), offset(offset) { }

        Type* type;
        std::size_t offset;

        Value* get(Value* container) const {
            Value* member;
            std::memcpy(&member, (char*)container + offset, sizeof(Value*));
            return member;
        }

        void set(Value* container, Value* member) const {
            std::memcpy((char*)container + offset, &member, sizeof(Value*));
        }
    };

    class Type : boost::noncopyable {
    public:
        virtual ~Type() = 0;

        virtual std::size_t size() const = 0;
        virtual std::size_t alignment() const = 0;

        virtual std::pair<Field const*, Field const*> fields() const = 0;
    };
    inline Type::~Type() = default;

    template<typename T>
    class PrimitiveType : public Type {
    public:
        static_assert(std::is_trivially_copyable<T>(), "oops");

        static PrimitiveType& instance() {
            static PrimitiveType type;
            return type;
        }

        T get(Value* container) {
            return reinterpret_cast<Representation*>(container)->value;
        }

        void set(Value* container, T value) {
            reinterpret_cast<Representation*>(container)->value = value;
        }

        std::size_t size() const override {
            return sizeof(Representation);
        }

        std::size_t alignment() const override {
            return alignof(Representation);
        }

        std::pair<Field const*, Field const*> fields() const override {
            return std::make_pair(nullptr, nullptr);
        }

    private:
        struct Representation {
            Type* type;
            T value;
        };

        PrimitiveType() = default;
    };

    class StructType : public Type {
    public:
        explicit StructType(std::vector<Type*> const& fieldTypes)
            : size_(sizeof(Type*)), alignment_(alignof(Type*)) {
            fields_.reserve(fieldTypes.size());
            for (auto fieldType : fieldTypes) {
                fields_.emplace_back(fieldType, size_);
                size_ += sizeof(Type*);
            }
        }

        std::size_t size() const override {
            return size_;
        }

        std::size_t alignment() const override {
            return alignment_;
        }

        std::pair<Field const*, Field const*> fields() const override {
            return std::make_pair(&*fields_.begin(), &*fields_.end());
        }

    private:
        std::size_t size_;
        std::size_t alignment_;
        std::vector<Field> fields_;
    };
}

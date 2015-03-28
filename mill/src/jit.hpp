#pragma once
#include <algorithm>
#include <baka/io/io_error.hpp>
#include <boost/intrusive_ptr.hpp>
#include <cstdarg>
#include <cstdint>
#include "instructions.hpp"
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/TargetSelect.h>
#include <mutex>
#include "object.hpp"
#include <string>
#include <vector>
#include "value.hpp"
#include "vm.hpp"

namespace mill {
    namespace detail {
        template<typename ReaderSeeker>
        class JITCompiler {
        public:
            JITCompiler(VM& vm, Object const& object, llvm::LLVMContext& ctx, llvm::Module& module, ReaderSeeker& source)
                : vm(&vm), object(&object), ctx(&ctx), module(&module), builder(ctx), source(&source) { }

            llvm::Function* operator()() {
                auto functionConstant = module->getOrInsertFunction(
                    std::to_string(uniqueID()),
                    valueType(),
                    llvm::Type::getInt8PtrTy(*ctx),
                    llvm::Type::getInt64Ty(*ctx),
                    llvm::PointerType::getUnqual(valueType()),
                    nullptr
                );
                function = llvm::cast<llvm::Function>(functionConstant);

                auto entry = llvm::BasicBlock::Create(*ctx, "", function);
                builder.SetInsertPoint(entry);

                try {
                    for (;;) {
                        readInstruction(*source, *this);
                    }
                } catch (baka::io::eof_error const&) { }

                return function;
            }

            void visitPushGlobal(std::uint32_t nameIndex) {
                auto ptr = vm->global(*object, nameIndex).get();
                stack.push_back(pointerLiteral(ptr));
            }

            void visitPushString(std::uint32_t index) {
                auto ptr = vm->string(*object, index).get();
                stack.push_back(pointerLiteral(ptr));
            }

            void visitPushBoolean(std::uint8_t) {
                assert(0);
            }

            void visitPushUnit() {
                auto unit = make<Unit>();
                retain(*unit); // TODO: Fix memory leak.
                stack.push_back(pointerLiteral(unit.get()));
            }

            void visitPushParameter(std::uint32_t index) {
                auto argv = &*++++function->getArgumentList().begin();
                stack.push_back(builder.CreateGEP(argv, {llvm::ConstantInt::get(llvm::Type::getInt64Ty(*ctx), index)}));
            }

            void visitPop() {
                stack.pop_back();
            }

            void visitSwap() {
                using std::swap;
                swap(stack[stack.size() - 1], stack[stack.size() - 2]);
            }

            void visitCall(std::uint32_t argc) {
                std::vector<llvm::Value*> argv(argc);
                for (decltype(argc) i = 0; i < argc; ++i) {
                    argv[i] = stack.back();
                    stack.pop_back();
                }
                std::reverse(argv.begin(), argv.end());

                auto callee = stack.back();
                stack.pop_back();

                auto llvmThunkI8Ptr = pointerLiteral(reinterpret_cast<void*>(callThunk));
                auto llvmThunkPtr = builder.CreatePointerCast(
                    llvmThunkI8Ptr,
                    llvm::PointerType::getUnqual(
                        llvm::FunctionType::get(
                            valueType(),
                            {
                                valueType(),
                                llvm::Type::getInt8PtrTy(*ctx),
                                llvm::Type::getInt64Ty(*ctx),
                            },
                            true
                        )
                    )
                );

                std::vector<llvm::Value*> llvmArgv{
                    callee,
                    pointerLiteral(vm),
                    llvm::ConstantInt::get(llvm::Type::getInt64Ty(*ctx), argc),
                };
                for (auto&& arg : argv) {
                    llvmArgv.push_back(arg);
                }
                stack.push_back(builder.CreateCall(llvmThunkPtr, llvmArgv));
            }

            void visitReturn() {
                builder.CreateRet(stack.back());
                stack.pop_back();
            }

        private:
            VM* vm;
            Object const* object;
            llvm::LLVMContext* ctx;
            llvm::Module* module;
            llvm::IRBuilder<> builder;
            ReaderSeeker* source;
            llvm::Function* function;
            std::vector<llvm::Value*> stack;

            static auto uniqueID() {
                static std::uintmax_t id = 0;
                static std::mutex mutex;
                std::lock_guard<decltype(mutex)> lock(mutex);
                return ++id;
            }

            static void* callThunk(void* vcallee, void* vvm, std::uint64_t vargc, ...) {
                auto& callee = dynamic_cast<Subroutine&>(*static_cast<Value*>(vcallee));
                auto& vm = *static_cast<VM*>(vvm);

                auto argc = static_cast<std::size_t>(vargc);
                std::vector<Value*> argv(argc);
                va_list args; va_start(args, vargc);
                for (decltype(argc) i = 0; i < argc; ++i) {
                    argv[i] = static_cast<Value*>(va_arg(args, void*));
                }
                va_end(args);

                return callee(vm, argc, argv.data()).get();
            }

            llvm::Value* pointerLiteral(void const* ptr) {
                auto llvmI64Ptr = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*ctx), (std::uint64_t)ptr);
                return llvm::ConstantExpr::getIntToPtr(llvmI64Ptr, valueType());
            }

            llvm::Type* valueType() {
                return llvm::Type::getInt8PtrTy(*ctx);
            }
        };
    }

    template<typename ReaderSeeker>
    auto jitCompile(VM& vm, Object const& object, ReaderSeeker& source) {
        static llvm::LLVMContext ctx;
        static auto module = new llvm::Module("main", ctx);
        static auto engine = (llvm::InitializeNativeTarget(), llvm::EngineBuilder(module).create());
        static std::mutex mutex;
        std::lock_guard<decltype(mutex)> lock(mutex);

        detail::JITCompiler<ReaderSeeker> jitCompiler(vm, object, ctx, *module, source);
        auto llvmPointer = jitCompiler();

        auto functionPointer = reinterpret_cast<void*(*)(void*, std::uint64_t, void**)>(engine->getPointerToFunction(llvmPointer));
        return [=] (VM& vm, std::size_t argc, Value** argv) -> boost::intrusive_ptr<Value> {
            return static_cast<Value*>(functionPointer(&vm, argc, reinterpret_cast<void**>(argv)));
        };
    }
}

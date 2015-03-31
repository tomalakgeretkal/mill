#pragma once
#include <algorithm>
#include <baka/io/io_error.hpp>
#include <boost/intrusive_ptr.hpp>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include "instructions.hpp"
#include <llvm/Analysis/Passes.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Transforms/Scalar.h>
#include <map>
#include <mutex>
#include "object.hpp"
#include <string>
#include "util.hpp"
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
                auto fakeEntry = llvm::BasicBlock::Create(*ctx, "", function);

                builder.SetInsertPoint(entry);
                builder.CreateBr(fakeEntry);

                builder.SetInsertPoint(fakeEntry);

                stack = builder.CreateAlloca(
                    valueType(),
                    llvm::ConstantInt::get(llvm::Type::getInt64Ty(*ctx), 32)
                );
                stackSize = builder.CreateAlloca(
                    llvm::Type::getInt32Ty(*ctx),
                    llvm::ConstantInt::get(llvm::Type::getInt64Ty(*ctx), 1)
                );
                builder.CreateStore(
                    llvm::ConstantInt::get(llvm::Type::getInt32Ty(*ctx), 0),
                    stackSize
                );

                makeBlocks(function, fakeEntry);
                source->seek_begin(0);
                try {
                    for (;;) {
                        auto oldBlock = builder.GetInsertBlock();
                        auto newBlock = blockAt(source->tell());
                        if (newBlock != oldBlock && !oldBlock->back().isTerminator()) {
                            builder.CreateBr(newBlock);
                        }
                        builder.SetInsertPoint(newBlock);
                        readInstruction(*source, *this);
                    }
                } catch (baka::io::eof_error const&) { }

                llvm::legacy::FunctionPassManager fpm(module);
                fpm.add(llvm::createPromoteMemoryToRegisterPass());
                fpm.add(llvm::createInstructionCombiningPass());
                fpm.add(llvm::createGVNPass());
                fpm.add(llvm::createCFGSimplificationPass());
                fpm.doInitialization();

                for (auto it = module->begin(); it != module->end(); ++it) {
                    fpm.run(*it);
                }

                return function;
            }

            void makeBlocks(llvm::Function* function, llvm::BasicBlock* entry) {
                blocks.emplace(0, entry);
                class Visitor : public DefaultInstructionVisitor<void, Visitor> {
                public:
                    Visitor(JITCompiler& self, llvm::Function* function)
                        : self(&self), function(function) { }

                    void visitDefault() { }

                    void visitConditionalJump(std::uint32_t offset) {
                        addBlock(offset);
                        addBlock(self->source->tell());
                    }

                    void visitUnconditionalJump(std::uint32_t offset) {
                        addBlock(offset);
                    }

                    void addBlock(std::uint32_t offset) {
                        if (!self->blocks.count(offset)) {
                            self->blocks.emplace(offset, llvm::BasicBlock::Create(*self->ctx, "", function));
                        }
                    }

                    JITCompiler* self;
                    llvm::Function* function;
                } visitor(*this, function);
                try {
                    for (;;) {
                        readInstruction(*source, visitor);
                    }
                } catch (baka::io::eof_error const&) { }
            }

            void visitPushGlobal(std::uint32_t nameIndex) {
                auto ptr = vm->global(*object, nameIndex).get();
                push(pointerLiteral(ptr));
                emitRetain(top());
            }

            void visitPushString(std::uint32_t index) {
                auto ptr = vm->string(*object, index).get();
                push(pointerLiteral(ptr));
                emitRetain(top());
            }

            void visitPushBoolean(std::uint8_t value) {
                auto boolean = make<Boolean>(!!value);
                retain(*boolean); // TODO: Fix memory leak;
                push(pointerLiteral(boolean.get()));
            }

            void visitPushUnit() {
                push(pointerLiteral(&Unit::instance()));
                emitRetain(top());
            }

            void visitPushParameter(std::uint32_t index) {
                auto argv = &*++++function->getArgumentList().begin();
                auto argPtr = builder.CreateGEP(argv, {
                    llvm::ConstantInt::get(llvm::Type::getInt64Ty(*ctx), index)
                });
                push(builder.CreateLoad(argPtr));
                emitRetain(top());
            }

            void visitPop() {
                emitRelease(pop());
            }

            void visitSwap() {
                auto a = pop();
                auto b = pop();
                push(a);
                push(b);
            }

            void visitCall(std::uint32_t argc) {
                std::vector<llvm::Value*> argv(argc);
                for (decltype(argc) i = 0; i < argc; ++i) {
                    argv[i] = pop();
                }
                std::reverse(argv.begin(), argv.end());

                auto callee = pop();

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
                push(builder.CreateCall(llvmThunkPtr, llvmArgv));

                for (auto it = argv.rbegin(); it != argv.rend(); ++it) {
                    emitRelease(*it);
                }
                emitRelease(callee);
            }

            void visitReturn() {
                builder.CreateRet(top());
            }

            void visitUnconditionalJump(std::uint32_t offset) {
                builder.CreateBr(blockAt(offset));
            }

            void visitConditionalJump(std::uint32_t offset) {
                auto conditionValue = pop();
                auto extractorPtr = builder.CreatePointerCast(
                    pointerLiteral(+[] (void* value) -> char {
                        return dynamic_cast<Boolean&>(*static_cast<Value*>(value)).value;
                    }),
                    llvm::PointerType::getUnqual(llvm::FunctionType::get(llvm::Type::getInt8Ty(*ctx), { valueType() }, false))
                );
                auto conditionI8 = builder.CreateCall(extractorPtr, conditionValue);
                auto condition = builder.CreateICmpNE(conditionI8, llvm::ConstantInt::get(llvm::Type::getInt8Ty(*ctx), 0));
                builder.CreateCondBr(condition, blockAt(offset), blockAt(source->tell()));
            }

        private:
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

                auto result = callee(vm, argc, argv.data());
                retain(*result.get());
                return result.get();
            }

            void push(llvm::Value* value) {
                auto stackSizeInt = builder.CreateLoad(stackSize);
                auto newStackSize = builder.CreateAdd(
                    stackSizeInt,
                    llvm::ConstantInt::get(llvm::Type::getInt32Ty(*ctx), 1)
                );
                builder.CreateStore(newStackSize, stackSize);

                auto entry = builder.CreateGEP(stack, {stackSizeInt});
                builder.CreateStore(value, entry);
            }

            llvm::Value* pop() {
                auto stackSizeInt = builder.CreateLoad(stackSize);
                auto newStackSize = builder.CreateSub(
                    stackSizeInt,
                    llvm::ConstantInt::get(llvm::Type::getInt32Ty(*ctx), 1)
                );
                builder.CreateStore(newStackSize, stackSize);

                auto entry = builder.CreateGEP(stack, {newStackSize});
                return builder.CreateLoad(entry);
            }

            llvm::Value* top() {
                auto stackSizeInt = builder.CreateLoad(stackSize);
                auto topIndex = builder.CreateSub(
                    stackSizeInt,
                    llvm::ConstantInt::get(llvm::Type::getInt32Ty(*ctx), 1)
                );
                auto entry = builder.CreateGEP(stack, {topIndex});
                return builder.CreateLoad(entry);
            }

            template<typename T>
            llvm::Value* pointerLiteral(T const* ptr) {
                return pointerLiteral(ptr, valueType());
            }

            template<typename T>
            llvm::Value* pointerLiteral(T const* ptr, llvm::Type* type) {
                auto llvmI64Ptr = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*ctx), (std::uint64_t)ptr);
                return llvm::ConstantExpr::getIntToPtr(llvmI64Ptr, type);
            }

            llvm::Type* valueType() {
                return llvm::Type::getInt8PtrTy(*ctx);
            }

            void emitRetain(llvm::Value* value) {
                auto retainPtr = pointerLiteral(
                    +[] (void* v) { retain(*static_cast<Value*>(v)); },
                    llvm::PointerType::getUnqual(llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx), {valueType()}, false))
                );
                builder.CreateCall(retainPtr, value);
            }

            void emitRelease(llvm::Value* value) {
                auto releasePtr = pointerLiteral(
                    +[] (void* v) { release(*static_cast<Value*>(v)); },
                    llvm::PointerType::getUnqual(llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx), {valueType()}, false))
                );
                builder.CreateCall(releasePtr, value);
            }

            llvm::BasicBlock* blockAt(std::size_t offset) {
                return largestBeforeOrEqual(blocks, offset)->second;
            }

            VM* vm;
            Object const* object;
            llvm::LLVMContext* ctx;
            llvm::Module* module;
            llvm::IRBuilder<> builder;
            ReaderSeeker* source;
            llvm::Function* function;
            llvm::Value* stack;
            llvm::Value* stackSize;
            std::map<std::size_t, llvm::BasicBlock*> blocks;
        };
    }

    template<typename ReaderSeeker>
    auto jitCompile(VM& vm, Object const& object, ReaderSeeker& source) {
        static llvm::LLVMContext ctx;
        static auto module = new llvm::Module("main", ctx);
        static llvm::ExecutionEngine* engine = nullptr;
        static std::mutex mutex;
        std::lock_guard<decltype(mutex)> lock(mutex);

        if (!engine) {
            llvm::InitializeNativeTarget();
            auto builder = llvm::EngineBuilder(module);
            auto target = builder.selectTarget();
            target->Options.PrintMachineCode = 0;
            engine = builder.create(target);
        }

        detail::JITCompiler<ReaderSeeker> jitCompiler(vm, object, ctx, *module, source);
        auto llvmPointer = jitCompiler();

        auto functionPointer = reinterpret_cast<void*(*)(void*, std::uint64_t, void**)>(engine->getPointerToFunction(llvmPointer));
        return [=] (VM& vm, std::size_t argc, Value** argv) -> boost::intrusive_ptr<Value> {
            return static_cast<Value*>(functionPointer(&vm, argc, reinterpret_cast<void**>(argv)));
        };
    }
}

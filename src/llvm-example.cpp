#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/TargetParser/Host.h"

#include "llvm/IR/Verifier.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"

#include "llvm/Support/FileSystem.h"

#include <string>
#include <vector>
#include <string_view>

auto main() -> int {
    using namespace std::literals;

    auto context = llvm::LLVMContext{};
    auto module = llvm::Module{"module", context};

    auto triple = llvm::sys::getDefaultTargetTriple();
    module.setTargetTriple(triple);

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    auto error = std::string{};
    auto target = llvm::TargetRegistry::lookupTarget(triple, error);
    if (!target) {
        llvm::errs() << error;
        return 1;
    }
    auto options = llvm::TargetOptions{};
    auto machine = target->createTargetMachine(triple, "generic", "", options, llvm::Reloc::PIC_);
    module.setDataLayout(machine->createDataLayout());

    auto builder = llvm::IRBuilder{context};

    auto param_types = std::vector<llvm::Type *>{
        llvm::Type::getInt64Ty(context),
        llvm::Type::getInt64Ty(context),
    };
    auto result_type = llvm::Type::getInt64Ty(context);
    auto type = llvm::FunctionType::get(result_type, param_types, false);

    auto name = "foo"sv;
    auto function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, name, module);

    auto block = llvm::BasicBlock::Create(context, "entry", function);
    builder.SetInsertPoint(block);

    auto args = function->arg_begin();
    auto r = builder.CreateAdd(&args[0], &args[1]);
    builder.CreateRet(r);

    llvm::verifyModule(module);
    module.print(llvm::errs(), nullptr);

    auto ec = std::error_code{};
    auto stream = llvm::raw_fd_ostream{"-", ec, llvm::sys::fs::OF_Delete};
    if (ec) {
        llvm::errs() << "failed to open output: " << ec.message();
        return 1;
    }

    auto passes = llvm::legacy::PassManager{};
    auto format = llvm::CodeGenFileType::CGFT_AssemblyFile;
    if (machine->addPassesToEmitFile(passes, stream, nullptr, format)) {
        llvm::errs() << "target machine cannot emit a file of this type\n";
        return 1;
    }
    passes.run(module);
    stream.flush();
}
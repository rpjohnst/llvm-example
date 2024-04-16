find_package(llvm CONFIG REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(HandleLLVMOptions)
add_definitions(${LLVM_DEFINITIONS})

add_executable(llvm-example
    llvm-example.cpp)
target_compile_features(llvm-example PRIVATE cxx_std_20)
target_include_directories(llvm-example PRIVATE ${LLVM_INCLUDE_DIRS})
llvm_map_components_to_libnames(llvm_libs native)
target_link_libraries(llvm-example PRIVATE ${llvm_libs})
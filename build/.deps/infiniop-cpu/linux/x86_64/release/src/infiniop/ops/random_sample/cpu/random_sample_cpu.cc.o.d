{
    depfiles = "random_sample_cpu.o:  src/infiniop/ops/random_sample/cpu/random_sample_cpu.cc  src/infiniop/ops/random_sample/cpu/random_sample_cpu.h  src/infiniop/ops/random_sample/cpu/../random_sample.h  src/infiniop/ops/random_sample/cpu/../../../operator.h  include/infiniop/operator_descriptor.h include/infiniop/handle.h  include/infiniop/../infinicore.h include/infiniop/tensor_descriptor.h  src/infiniop/ops/random_sample/cpu/../info.h  src/infiniop/ops/random_sample/cpu/../../../../utils.h  src/infiniop/ops/random_sample/cpu/../../../../utils/custom_types.h  src/infiniop/ops/random_sample/cpu/../../../../utils/rearrange.h  src/infiniop/ops/random_sample/cpu/../../../../utils/result.hpp  src/infiniop/ops/random_sample/cpu/../../../../utils/check.h  include/infinicore.h  src/infiniop/ops/random_sample/cpu/../../../tensor.h  include/infiniop/tensor_descriptor.h  src/infiniop/ops/random_sample/cpu/../../../../utils.h  src/infiniop/ops/random_sample/cpu/../../../devices/cpu/common_cpu.h  src/infiniop/ops/random_sample/cpu/../../../devices/cpu/../../../utils.h  src/infiniop/ops/random_sample/cpu/../../../devices/cpu/cpu_handle.h  src/infiniop/ops/random_sample/cpu/../../../devices/cpu/../../handle.h  include/infiniop/handle.h src/infiniop/ops/random_sample/cpu/../info.h\
",
    depfiles_format = "gcc",
    values = {
        "/home/spack/spack/opt/spack/linux-ubuntu22.04-icelake/gcc-11.4.0/gcc-11.3.0-7tpmmhoar763gi2qhigyczd2vqqhpgxk/bin/g++",
        {
            "-m64",
            "-fvisibility=hidden",
            "-fvisibility-inlines-hidden",
            "-Wall",
            "-Werror",
            "-O3",
            "-std=c++17",
            "-Iinclude",
            "-DENABLE_CPU_API",
            "-DENABLE_OMP",
            "-DENABLE_NVIDIA_API",
            "-DENABLE_CUDNN_API",
            "-finput-charset=UTF-8",
            "-fexec-charset=UTF-8",
            "-fPIC",
            "-Wno-unknown-pragmas",
            "-fopenmp",
            "-DNDEBUG"
        }
    },
    files = {
        "src/infiniop/ops/random_sample/cpu/random_sample_cpu.cc"
    }
}
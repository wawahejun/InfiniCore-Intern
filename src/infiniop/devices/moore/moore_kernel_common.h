#define INFINIOP_MOORE_KERNEL __global__ void

#include <musa_bf16.h>
#include <musa_fp16.h>

// Posible maximum number of threads per block for MUSA architectures
// Used for picking correct kernel launch configuration
#define MOORE_BLOCK_SIZE_2048 2048
#define MOORE_BLOCK_SIZE_1024 1024
#define MOORE_BLOCK_SIZE_512 512

#define CHECK_MOORE(API) CHECK_INTERNAL(API, musaSuccess)

using cuda_bfloat16 = mt_bfloat16;
using cuda_bfloat162 = mt_bfloat162;

namespace device::moore {

// return the memory offset of original tensor, given the flattened index of broadcasted tensor
__forceinline__ __device__ __host__ size_t
indexToReducedOffset(
    size_t flat_index,
    size_t ndim,
    const ptrdiff_t *broadcasted_strides,
    const ptrdiff_t *target_strides) {
    size_t res = 0;
    for (size_t i = 0; i < ndim; ++i) {
        res += flat_index / broadcasted_strides[i] * target_strides[i];
        flat_index %= broadcasted_strides[i];
    }
    return res;
}

// get the memory offset of the given element in a tensor given its flat index
__forceinline__ __device__ __host__ size_t
indexToOffset(
    size_t flat_index,
    size_t ndim,
    const size_t *shape,
    const ptrdiff_t *strides) {
    size_t res = 0;
    for (size_t i = ndim; i-- > 0;) {
        res += (flat_index % shape[i]) * strides[i];
        flat_index /= shape[i];
    }
    return res;
}
} // namespace device::moore

__forceinline__ __device__ float
exp_(const float val) {
    return expf(val);
}

__forceinline__ __device__ long double
exp_(const long double val) {
    return exp(val);
}

__forceinline__ __device__ double
exp_(const double val) {
    return exp(val);
}

// <musa_bf16.h> may not support hexp
__forceinline__ __device__ __half
exp_(const __half x) {
    float f_val = __half2float(x);
    float f_result = expf(f_val);
    return __float2half(f_result);
}

// <musa_bf16.h> may not support hexp
__forceinline__ __device__ __mt_bfloat16
exp_(const __mt_bfloat16 x) {
    float f_val = __bfloat162float(x);
    float f_result = expf(f_val);
    return __float2bfloat16(f_result);
}

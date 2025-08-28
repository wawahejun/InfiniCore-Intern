#include "gemm_metax.h"
#include "../../../devices/metax/metax_common.h"
#include "../../../devices/metax/metax_handle.h"

namespace op::gemm::metax {

struct Descriptor::Opaque {
    std::shared_ptr<device::metax::Handle::Internal> internal;
};

Descriptor::~Descriptor() {
    delete _opaque;
}

infiniStatus_t Descriptor::create(
    infiniopHandle_t handle_,
    Descriptor **desc_ptr,
    infiniopTensorDescriptor_t c_desc,
    infiniopTensorDescriptor_t a_desc,
    infiniopTensorDescriptor_t b_desc) {
    auto handle = reinterpret_cast<device::metax::Handle *>(handle_);
    auto dtype = c_desc->dtype();

    CHECK_DTYPE(dtype, INFINI_DTYPE_F16, INFINI_DTYPE_F32, INFINI_DTYPE_BF16);

    auto result = MatmulInfo::create(c_desc, a_desc, b_desc, MatrixLayout::COL_MAJOR);
    CHECK_RESULT(result);

    *desc_ptr = new Descriptor(
        dtype, result.take(), 0,
        new Opaque{handle->internal()},
        handle->device, handle->device_id);
    return INFINI_STATUS_SUCCESS;
}

infiniStatus_t Descriptor::calculate(
    void *workspace,
    size_t workspace_size,
    void *c,
    float beta,
    const void *a,
    const void *b,
    float alpha,
    void *stream) const {

    hpccDataType a_type, b_type, c_type;
    hcblasComputeType_t compute_type;

    switch (_dtype) {
    case INFINI_DTYPE_F16:
        a_type = b_type = c_type = HPCC_R_16F;
        compute_type = HCBLAS_COMPUTE_32F;
        break;
    case INFINI_DTYPE_BF16:
        a_type = b_type = c_type = HPCC_R_16BF;
        compute_type = HCBLAS_COMPUTE_32F;
        break;
    case INFINI_DTYPE_F32:
        a_type = b_type = c_type = HPCC_R_32F;
        compute_type = HCBLAS_COMPUTE_32F_FAST_TF32;
        break;

    default:
        return INFINI_STATUS_BAD_TENSOR_DTYPE;
    }

    if (_info.is_transed) {
        std::swap(a, b);
    }

    auto op_a = _info.a_matrix.row_stride == 1 ? HCBLAS_OP_N : HCBLAS_OP_T;
    auto op_b = _info.b_matrix.row_stride == 1 ? HCBLAS_OP_N : HCBLAS_OP_T;

    CHECK_STATUS(_opaque->internal->useMcblas(
        (hcStream_t)stream,
        [&](hcblasHandle_t handle) {
            CHECK_MCBLAS(
                hcblasGemmStridedBatchedEx(
                    handle,
                    op_a,
                    op_b,
                    static_cast<int>(_info.m),
                    static_cast<int>(_info.n),
                    static_cast<int>(_info.k),
                    &alpha,
                    a,
                    a_type,
                    static_cast<int>(_info.a_matrix.ld()),
                    _info.a_matrix.stride,
                    b,
                    b_type,
                    static_cast<int>(_info.b_matrix.ld()),
                    _info.b_matrix.stride,
                    &beta,
                    c,
                    c_type,
                    static_cast<int>(_info.c_matrix.ld()),
                    _info.c_matrix.stride,
                    static_cast<int>(_info.batch),
                    compute_type,
                    HCBLAS_GEMM_DEFAULT_TENSOR_OP));
            return INFINI_STATUS_SUCCESS;
        }));
    return INFINI_STATUS_SUCCESS;
}

} // namespace op::gemm::metax

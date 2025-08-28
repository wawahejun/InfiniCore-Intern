#include "../../operator.h"
#include "../../handle.h"
#include "infiniop/ops/relu_backward.h"

#ifdef ENABLE_CPU_API
#include "cpu/relu_backward_cpu.h"
#endif

#if defined(ENABLE_NVIDIA_API) || defined(ENABLE_ILUVATAR_API)
#include "nvidia/relu_backward_nvidia.cuh"
#endif

#ifdef ENABLE_METAX_API
#include "metax/relu_backward_metax.h"
#endif

#ifdef ENABLE_KUNLUN_API
#include "kunlun/relu_backward_kunlun.h"
#endif

__C infiniStatus_t infiniopCreateReluBackwardDescriptor(
    infiniopHandle_t handle,
    infiniopReluBackwardDescriptor_t *desc_ptr,
    infiniopTensorDescriptor_t grad_input_desc,
    infiniopTensorDescriptor_t input_desc,
    infiniopTensorDescriptor_t grad_output_desc) {

#define CREATE(CASE, NAMESPACE)                                                     \
    case CASE:                                                                      \
        return op::relu_backward::NAMESPACE::Descriptor::create(                    \
            handle,                                                                 \
            reinterpret_cast<op::relu_backward::NAMESPACE::Descriptor **>(desc_ptr), \
            grad_input_desc,                                                        \
            {input_desc, grad_output_desc})

    switch (handle->device) {

#ifdef ENABLE_CPU_API
        CREATE(INFINI_DEVICE_CPU, cpu);
#endif

#ifdef ENABLE_NVIDIA_API
        CREATE(INFINI_DEVICE_NVIDIA, nvidia);
#endif

#ifdef ENABLE_ILUVATAR_API
        CREATE(INFINI_DEVICE_ILUVATAR, nvidia);
#endif

#ifdef ENABLE_METAX_API
        CREATE(INFINI_DEVICE_METAX, metax);
#endif

#ifdef ENABLE_KUNLUN_API
        CREATE(INFINI_DEVICE_KUNLUN, kunlun);
#endif

    default:
        return INFINI_STATUS_DEVICE_TYPE_NOT_SUPPORTED;
    }

#undef CREATE
}

__C infiniStatus_t infiniopGetReluBackwardWorkspaceSize(infiniopReluBackwardDescriptor_t desc, size_t *size) {
#define GET_WORKSPACE_SIZE(CASE, NAMESPACE)                                 \
    case CASE:                                                              \
        *size = reinterpret_cast<op::relu_backward::NAMESPACE::Descriptor *>(desc) \
            ->workspaceSize();                                              \
        return INFINI_STATUS_SUCCESS

    switch (reinterpret_cast<InfiniopDescriptor *>(desc)->device_type) {

#ifdef ENABLE_CPU_API
        GET_WORKSPACE_SIZE(INFINI_DEVICE_CPU, cpu);
#endif

#ifdef ENABLE_NVIDIA_API
        GET_WORKSPACE_SIZE(INFINI_DEVICE_NVIDIA, nvidia);
#endif

#ifdef ENABLE_ILUVATAR_API
        GET_WORKSPACE_SIZE(INFINI_DEVICE_ILUVATAR, nvidia);
#endif

#ifdef ENABLE_METAX_API
        GET_WORKSPACE_SIZE(INFINI_DEVICE_METAX, metax);
#endif

#ifdef ENABLE_KUNLUN_API
        GET_WORKSPACE_SIZE(INFINI_DEVICE_KUNLUN, kunlun);
#endif

    default:
        return INFINI_STATUS_DEVICE_TYPE_NOT_SUPPORTED;
    }

#undef GET_WORKSPACE_SIZE
}

__C infiniStatus_t infiniopReluBackward(
    infiniopReluBackwardDescriptor_t desc,
    void *workspace,
    size_t workspace_size,
    void *grad_input,
    const void *input,
    const void *grad_output,
    void *stream) {

#define CALCULATE(CASE, NAMESPACE)                                                  \
    case CASE:                                                                      \
        return reinterpret_cast<op::relu_backward::NAMESPACE::Descriptor *>(desc)   \
            ->calculate(workspace, workspace_size, grad_input, {input, grad_output}, stream)

    switch (reinterpret_cast<InfiniopDescriptor *>(desc)->device_type) {

#ifdef ENABLE_CPU_API
        CALCULATE(INFINI_DEVICE_CPU, cpu);
#endif

#ifdef ENABLE_NVIDIA_API
        CALCULATE(INFINI_DEVICE_NVIDIA, nvidia);
#endif

#ifdef ENABLE_ILUVATAR_API
        CALCULATE(INFINI_DEVICE_ILUVATAR, nvidia);
#endif

#ifdef ENABLE_METAX_API
        CALCULATE(INFINI_DEVICE_METAX, metax);
#endif

#ifdef ENABLE_KUNLUN_API
        CALCULATE(INFINI_DEVICE_KUNLUN, kunlun);
#endif

    default:
        return INFINI_STATUS_DEVICE_TYPE_NOT_SUPPORTED;
    }

#undef CALCULATE
}

__C infiniStatus_t
infiniopDestroyReluBackwardDescriptor(infiniopReluBackwardDescriptor_t desc) {
#define DESTROY(CASE, NAMESPACE)                                                    \
    case CASE:                                                                      \
        delete reinterpret_cast<op::relu_backward::NAMESPACE::Descriptor *>(desc);  \
        return INFINI_STATUS_SUCCESS

    switch (reinterpret_cast<InfiniopDescriptor *>(desc)->device_type) {

#ifdef ENABLE_CPU_API
        DESTROY(INFINI_DEVICE_CPU, cpu);
#endif

#ifdef ENABLE_NVIDIA_API
        DESTROY(INFINI_DEVICE_NVIDIA, nvidia);
#endif

#ifdef ENABLE_ILUVATAR_API
        DESTROY(INFINI_DEVICE_ILUVATAR, nvidia);
#endif

#ifdef ENABLE_METAX_API
        DESTROY(INFINI_DEVICE_METAX, metax);
#endif

#ifdef ENABLE_KUNLUN_API
        DESTROY(INFINI_DEVICE_KUNLUN, kunlun);
#endif

    default:
        return INFINI_STATUS_DEVICE_TYPE_NOT_SUPPORTED;
    }

#undef DESTROY
}
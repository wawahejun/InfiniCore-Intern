#include "../../operator.h"
#include "../../handle.h"
#include "infiniop/ops/index_copy_inplace.h"
#include <cstdio>

#ifdef ENABLE_CPU_API
#include "cpu/index_copy_inplace_cpu.h"
#endif
#if defined(ENABLE_NVIDIA_API) || defined(ENABLE_ILUVATAR_API)
#include "nvidia/index_copy_inplace_nvidia.cuh"
#endif
#ifdef ENABLE_METAX_API
#include "metax/index_copy_inplace_metax.h"
#endif
#ifdef ENABLE_KUNLUN_API
#include "kunlun/index_copy_inplace_kunlun.h"
#endif
#ifdef ENABLE_CAMBRICON_API
#include "bang/index_copy_inplace_bang.h"
#endif

__C infiniStatus_t infiniopCreateIndexCopyInplaceDescriptor(
    infiniopHandle_t handle,
    infiniopIndexCopyInplaceDescriptor_t *desc_ptr,
    infiniopTensorDescriptor_t target,
    infiniopTensorDescriptor_t source,
    int dim,
    infiniopTensorDescriptor_t index) {

    if (desc_ptr == nullptr) {
        return INFINI_STATUS_NULL_POINTER;
    }
    if (handle == nullptr) {
        return INFINI_STATUS_NULL_POINTER;
    }

#define CREATE(CASE, NAMESPACE)                                            \
    case CASE:                                                             \
        return op::index_copy_inplace::NAMESPACE::Descriptor::create(      \
            handle,                                                        \
            reinterpret_cast<op::index_copy_inplace::NAMESPACE::Descriptor **>(desc_ptr), \
            target,                                                        \
            source,                                                        \
            dim,                                                           \
            index)

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
#ifdef ENABLE_CAMBRICON_API
        CREATE(INFINI_DEVICE_CAMBRICON, bang);
#endif

    default:
        return INFINI_STATUS_DEVICE_TYPE_NOT_SUPPORTED;
    }

#undef CREATE
}

__C infiniStatus_t infiniopGetIndexCopyInplaceWorkspaceSize(infiniopIndexCopyInplaceDescriptor_t desc, size_t *size) {

#define GET(CASE, NAMESPACE)                                                               \
    case CASE:                                                                             \
        *size = reinterpret_cast<op::index_copy_inplace::NAMESPACE::Descriptor *>(desc)->workspaceSize(); \
        return INFINI_STATUS_SUCCESS

    switch (desc->device_type) {
#ifdef ENABLE_CPU_API
        GET(INFINI_DEVICE_CPU, cpu);
#endif
#ifdef ENABLE_NVIDIA_API
        GET(INFINI_DEVICE_NVIDIA, nvidia);
#endif
#ifdef ENABLE_ILUVATAR_API
        GET(INFINI_DEVICE_ILUVATAR, nvidia);
#endif
#ifdef ENABLE_METAX_API
        GET(INFINI_DEVICE_METAX, metax);
#endif
#ifdef ENABLE_KUNLUN_API
        GET(INFINI_DEVICE_KUNLUN, kunlun);
#endif
#ifdef ENABLE_CAMBRICON_API
        GET(INFINI_DEVICE_CAMBRICON, bang);
#endif
    default:
        return INFINI_STATUS_DEVICE_TYPE_NOT_SUPPORTED;
    }
#undef GET

    return INFINI_STATUS_DEVICE_TYPE_NOT_SUPPORTED;
}

__C infiniStatus_t infiniopIndexCopyInplace(
    infiniopIndexCopyInplaceDescriptor_t desc,
    void *workspace,
    size_t workspace_size,
    void *target,
    const void *source,
    const void *index,
    void *stream) {


    if (!desc) {
        return INFINI_STATUS_BAD_PARAM;
    }

#define CALCULATE(CASE, NAMESPACE)                                                         \
    case CASE:                                                                             \
        return reinterpret_cast<op::index_copy_inplace::NAMESPACE::Descriptor *>(desc)->calculate( \
            workspace, workspace_size, target, source, index, stream)

    switch (desc->device_type) {
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
#ifdef ENABLE_CAMBRICON_API
        CALCULATE(INFINI_DEVICE_CAMBRICON, bang);
#endif
    default:
        return INFINI_STATUS_DEVICE_TYPE_NOT_SUPPORTED;
    }
#undef CALCULATE

    return INFINI_STATUS_DEVICE_TYPE_NOT_SUPPORTED;
}

__C infiniStatus_t
infiniopDestroyIndexCopyInplaceDescriptor(infiniopIndexCopyInplaceDescriptor_t desc) {

#define DESTROY(CASE, NAMESPACE)                                                           \
    case CASE:                                                                             \
        delete reinterpret_cast<op::index_copy_inplace::NAMESPACE::Descriptor *>(desc);   \
        return INFINI_STATUS_SUCCESS

    switch (desc->device_type) {
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
#ifdef ENABLE_CAMBRICON_API
        DESTROY(INFINI_DEVICE_CAMBRICON, bang);
#endif
    default:
        return INFINI_STATUS_DEVICE_TYPE_NOT_SUPPORTED;
    }
#undef DESTROY

    return INFINI_STATUS_DEVICE_TYPE_NOT_SUPPORTED;
}
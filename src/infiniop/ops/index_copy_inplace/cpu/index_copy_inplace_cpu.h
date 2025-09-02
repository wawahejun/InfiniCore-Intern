#ifndef __INDEX_COPY_INPLACE_CPU_H__
#define __INDEX_COPY_INPLACE_CPU_H__

#include "../../../operator.h"
#include "../../../devices/cpu/cpu_handle.h"

namespace op::index_copy_inplace::cpu {

class Descriptor : public InfiniopDescriptor {
public:
    Descriptor() = default;
    ~Descriptor();

    static infiniStatus_t create(
        infiniopHandle_t handle,
        Descriptor **desc_ptr,
        infiniopTensorDescriptor_t target_desc,
        infiniopTensorDescriptor_t source_desc,
        int dim,
        infiniopTensorDescriptor_t index_desc);

    size_t workspaceSize() const { return 0; }

    infiniStatus_t calculate(
        void *workspace,
        size_t workspace_size,
        void *target,
        const void *source,
        const void *index,
        void *stream) const;

private:
    infiniopTensorDescriptor_t _target_desc;
    infiniopTensorDescriptor_t _source_desc;
    infiniopTensorDescriptor_t _index_desc;
    int _dim;
    device::cpu::Handle *_handle;
};

} // namespace op::index_copy_inplace::cpu

#endif // __INDEX_COPY_INPLACE_CPU_H__
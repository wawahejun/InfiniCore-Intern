#ifndef __INFINIOP_TENSOR_H__
#define __INFINIOP_TENSOR_H__

#include "infiniop/tensor_descriptor.h"

#include "../utils.h"

#include <string>
#include <vector>

#define TRANSFORM_TENSOR_DESC(__TENSOR_DESC__, __OP__) \
    do {                                               \
        auto __RESULT__ = __TENSOR_DESC__->__OP__;     \
        CHECK_RESULT(__RESULT__);                      \
        __TENSOR_DESC__ = __RESULT__.take();           \
    } while (0)

struct InfiniopTensorDescriptor {
private:
    // Datatype
    infiniDtype_t _dtype;
    // Shape of the tensor
    std::vector<size_t> _shape;
    // Stride of each dimension in elements
    std::vector<ptrdiff_t> _strides;

public:
    InfiniopTensorDescriptor(infiniDtype_t dtype, size_t ndim, const size_t *shape, const ptrdiff_t *strides);
    ~InfiniopTensorDescriptor() = default;
    infiniDtype_t dtype() const;
    std::vector<size_t> shape() const;
    size_t dim(size_t i) const;
    size_t ndim() const;
    std::vector<ptrdiff_t> strides() const;
    ptrdiff_t stride(size_t i) const;
    std::vector<ptrdiff_t> getByteStrides() const;

    // Whether dimensions in [dim_start, dim_end] can be merged into a single dimension
    bool isMergable(size_t dim_start, size_t dim_end) const;
    bool isContiguous(size_t dim_) const;
    bool isContiguous(size_t dim_start, size_t dim_end) const;
    bool isContiguous() const;

    // Total number of elements in the tensor
    size_t numel() const;

    // a dim is broadcasted if it's corresponding stride is 0 but dim > 1
    bool hasBroadcastDim() const;
    std::vector<size_t> getBroadcastDim() const;

    utils::Result<infiniopTensorDescriptor_t> dimMerge(size_t dim_start, size_t dim_end) const;
    utils::Result<infiniopTensorDescriptor_t> dimSplit(size_t axis, const std::vector<size_t> &dims) const;
    utils::Result<infiniopTensorDescriptor_t> dimPermute(const std::vector<size_t> &order) const;

    std::string toString() const;
};

#endif // __INFINIOP_TENSOR_H__

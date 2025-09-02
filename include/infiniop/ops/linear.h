#ifndef __INFINIOP_LINEAR_API_H__
#define __INFINIOP_LINEAR_API_H__

#include "../operator_descriptor.h"

typedef struct InfiniopDescriptor *infiniopLinearDescriptor_t;

__C __export infiniStatus_t infiniopCreateLinearDescriptor(infiniopHandle_t handle,
                                                            infiniopLinearDescriptor_t *desc_ptr,
                                                            infiniopTensorDescriptor_t x,
                                                            infiniopTensorDescriptor_t w,
                                                            infiniopTensorDescriptor_t b,
                                                            infiniopTensorDescriptor_t y);

__C __export infiniStatus_t infiniopGetLinearWorkspaceSize(infiniopLinearDescriptor_t desc, size_t *size);

__C __export infiniStatus_t infiniopLinear(infiniopLinearDescriptor_t desc,
                                            void *workspace,
                                            size_t workspace_size,
                                            void *y,
                                            const void *x,
                                            const void *w,
                                            const void *b,
                                            void *stream);

__C __export infiniStatus_t infiniopDestroyLinearDescriptor(infiniopLinearDescriptor_t desc);

#endif
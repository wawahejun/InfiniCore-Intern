#ifndef __INFINIOP_CROSSENTROPYLOSS_BACKWARD_API_H__
#define __INFINIOP_CROSSENTROPYLOSS_BACKWARD_API_H__

#include "../operator_descriptor.h"

typedef struct InfiniopDescriptor *infiniopCrossEntropyLossBackwardDescriptor_t;

__C __export infiniStatus_t infiniopCreateCrossEntropyLossBackwardDescriptor(infiniopHandle_t handle,
                                                                             infiniopCrossEntropyLossBackwardDescriptor_t *desc_ptr,
                                                                             infiniopTensorDescriptor_t grad_logits,
                                                                             infiniopTensorDescriptor_t probs,
                                                                             infiniopTensorDescriptor_t target);

__C __export infiniStatus_t infiniopGetCrossEntropyLossBackwardWorkspaceSize(infiniopCrossEntropyLossBackwardDescriptor_t desc, size_t *size);

__C __export infiniStatus_t infiniopCrossEntropyLossBackward(infiniopCrossEntropyLossBackwardDescriptor_t desc,
                                                             void *workspace,
                                                             size_t workspace_size,
                                                             void *grad_logits,
                                                             const void *probs,
                                                             const void *target,
                                                             void *stream);

__C __export infiniStatus_t infiniopDestroyCrossEntropyLossBackwardDescriptor(infiniopCrossEntropyLossBackwardDescriptor_t desc);

#endif
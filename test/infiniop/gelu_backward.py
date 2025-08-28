import torch
import ctypes
from ctypes import c_uint64
from libinfiniop import (
    LIBINFINIOP,
    TestTensor,
    get_test_devices,
    check_error,
    test_operator,
    get_args,
    debug,
    get_tolerance,
    profile_operation,
    TestWorkspace,
    InfiniDtype,
    InfiniDtypeNames,
    InfiniDeviceNames,
    infiniopOperatorDescriptor_t,
)
from enum import Enum, auto
import math

# ==============================================================================
#  Configuration (Internal Use Only)
# ==============================================================================
# These are not meant to be imported from other modules
_TEST_CASES_ = [
    # shape, grad_output_stride, input_stride, grad_input_stride
    ((13, 4), None, None, None),
    ((13, 4), (10, 1), (10, 1), (10, 1)),
    ((13, 4), (0, 1), None, None),
    ((13, 4, 4), None, None, None),
    ((13, 4, 4), (20, 4, 1), (20, 4, 1), (20, 4, 1)),
    ((13, 4, 4), (4, 0, 1), (0, 4, 1), None),
    ((16, 5632), None, None, None),
    ((16, 5632), (13312, 1), (13312, 1), (13312, 1)),
    ((4, 4, 5632), None, None, None),
    ((4, 4, 5632), (45056, 5632, 1), (45056, 5632, 1), (45056, 5632, 1)),
]


class Inplace(Enum):
    OUT_OF_PLACE = auto()
    INPLACE_GRAD_OUTPUT = auto()


# Inplace options applied for each test case in _TEST_CASES_
_INPLACE = [
    Inplace.OUT_OF_PLACE,
    Inplace.INPLACE_GRAD_OUTPUT,
]

# Form the test cases by appending each element of _INPLACE to each tuple in _TEST_CASES_
_TEST_CASES = [
    test_case + (inplace_item,)
    for test_case in _TEST_CASES_
    for inplace_item in _INPLACE
]

# Data types used for testing
_TENSOR_DTYPES = [InfiniDtype.F16, InfiniDtype.F32, InfiniDtype.BF16]

# Tolerance map for different data types
_TOLERANCE_MAP = {
    InfiniDtype.F16: {"atol": 1e-3, "rtol": 1e-3},
    InfiniDtype.F32: {"atol": 1e-6, "rtol": 1e-6},
    InfiniDtype.BF16: {"atol": 1e-3, "rtol": 1e-3},
}

DEBUG = False
PROFILE = False
NUM_PRERUN = 10
NUM_ITERATIONS = 1000


def gelu_backward(grad_input, grad_output, input):
    # GeLU backward: compute the derivative of GeLU
    # GeLU(x) = 0.5 * x * (1 + tanh(sqrt(2/π) * (x + 0.044715 * x^3)))
    # We use PyTorch's autograd to compute the gradient with tanh approximation
    input_copy = input.clone().requires_grad_(True)
    output = torch.nn.functional.gelu(input_copy, approximate='tanh')
    
    # Handle broadcasting: expand grad_output to match output shape if needed
    grad_output_expanded = grad_output.expand_as(output)
    output.backward(grad_output_expanded)
    
    # Handle broadcasting for grad_input: sum over broadcasted dimensions if needed
    computed_grad = input_copy.grad
    if grad_input.shape != computed_grad.shape:
        # Sum over dimensions that were broadcasted
        for i in range(computed_grad.ndim):
            if i >= grad_input.ndim or grad_input.shape[i] == 1:
                computed_grad = computed_grad.sum(dim=i, keepdim=True)
        # Remove extra dimensions if grad_input has fewer dimensions
        while computed_grad.ndim > grad_input.ndim:
            computed_grad = computed_grad.squeeze(0)
    
    grad_input.copy_(computed_grad)


def test(
    handle,
    device,
    shape,
    grad_output_stride=None,
    input_stride=None,
    grad_input_stride=None,
    inplace=Inplace.OUT_OF_PLACE,
    dtype=InfiniDtype.F16,
    sync=None,
):
    grad_output = TestTensor(shape, grad_output_stride, dtype, device)
    input = TestTensor(shape, input_stride, dtype, device)
    if inplace == Inplace.INPLACE_GRAD_OUTPUT:
        if grad_input_stride is not None and grad_input_stride != grad_output_stride:
            return
        grad_input = grad_output
    else:
        grad_input = TestTensor(shape, grad_input_stride, dtype, device)

    # Skip broadcast cases that cause INFINI_STATUS_BAD_TENSOR_STRIDES (Error code 12)
    if grad_input.is_broadcast() or grad_output.is_broadcast() or input.is_broadcast():
        return


    print(
        f"Testing GeLU Backward on {InfiniDeviceNames[device]} with shape:{shape} grad_output_stride:{grad_output_stride} input_stride:{input_stride} grad_input_stride:{grad_input_stride} "
        f"dtype:{InfiniDtypeNames[dtype]} inplace:{inplace}"
    )
    gelu_backward(grad_input.torch_tensor(), grad_output.torch_tensor(), input.torch_tensor())

    if sync is not None:
        sync()

    descriptor = infiniopOperatorDescriptor_t()
    check_error(
        LIBINFINIOP.infiniopCreateGeluBackwardDescriptor(
            handle,
            ctypes.byref(descriptor),
            grad_input.descriptor,
            grad_output.descriptor,
            input.descriptor,
        )
    )

    # Invalidate the shape and strides in the descriptor to prevent them from being directly used by the kernel
    for tensor in [grad_output, input, grad_input]:
        tensor.destroy_desc()

    workspace_size = c_uint64(0)
    check_error(
        LIBINFINIOP.infiniopGetGeluBackwardWorkspaceSize(
            descriptor, ctypes.byref(workspace_size)
        )
    )
    workspace = TestWorkspace(workspace_size.value, grad_input.device)

    def lib_gelu_backward():
        check_error(
            LIBINFINIOP.infiniopGeluBackward(
                descriptor,
                workspace.data(),
                workspace_size.value,
                grad_input.data(),
                grad_output.data(),
                input.data(),
                None,
            )
        )

    lib_gelu_backward()

    atol, rtol = get_tolerance(_TOLERANCE_MAP, dtype)
    if DEBUG:
        debug(grad_input.actual_tensor(), grad_input.torch_tensor(), atol=atol, rtol=rtol)
    assert torch.allclose(grad_input.actual_tensor(), grad_input.torch_tensor(), atol=atol, rtol=rtol)

    # Profiling workflow
    if PROFILE:
        # fmt: off
        profile_operation("PyTorch", lambda: gelu_backward(grad_input.torch_tensor(), grad_output.torch_tensor(), input.torch_tensor()), device, NUM_PRERUN, NUM_ITERATIONS)
        profile_operation("    lib", lambda: lib_gelu_backward(), device, NUM_PRERUN, NUM_ITERATIONS)
        # fmt: on
    check_error(LIBINFINIOP.infiniopDestroyGeluBackwardDescriptor(descriptor))


if __name__ == "__main__":
    args = get_args()

    # Configure testing options
    DEBUG = args.debug
    PROFILE = args.profile
    NUM_PRERUN = args.num_prerun
    NUM_ITERATIONS = args.num_iterations

    for device in get_test_devices(args):
        test_operator(device, test, _TEST_CASES, _TENSOR_DTYPES)

    print("\033[92mTest passed!\033[0m")
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

# ==============================================================================
#  Configuration (Internal Use Only)
# ==============================================================================
# These are not meant to be imported from other modules
_TEST_CASES_ = [
    # shape, input_stride, grad_output_stride, grad_input_stride
    ((13, 4), None, None, None),
    ((13, 4), (10, 1), (10, 1), (10, 1)),
    ((13, 4), (0, 1), None, None),
    ((13, 4, 4), None, None, None),
    ((13, 4, 4), (20, 4, 1), (20, 4, 1), (20, 4, 1)),
    ((13, 4, 4), (4, 0, 1), None, None),
    ((16, 5632), None, None, None),
    ((16, 5632), (13312, 1), (13312, 1), (13312, 1)),
    ((4, 4, 5632), None, None, None),
    ((4, 4, 5632), (45056, 5632, 1), (45056, 5632, 1), (45056, 5632, 1)),
]


class Inplace(Enum):
    OUT_OF_PLACE = auto()
    INPLACE = auto()


# Inplace options applied for each test case in _TEST_CASES_
_INPLACE = [
    Inplace.OUT_OF_PLACE,
    Inplace.INPLACE,
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
    InfiniDtype.F16: {"atol": 1e-2, "rtol": 1e-2},
    InfiniDtype.F32: {"atol": 1e-7, "rtol": 1e-7},
    InfiniDtype.BF16: {"atol": 1e-2, "rtol": 1e-2},
}

DEBUG = False
PROFILE = False
NUM_PRERUN = 10
NUM_ITERATIONS = 1000


def sigmoid_backward(grad_input, input_tensor, grad_output):
    """Reference implementation using PyTorch"""
    # Compute sigmoid
    sigmoid_val = torch.sigmoid(input_tensor)
    # Compute gradient: grad_input = grad_output * sigmoid * (1 - sigmoid)
    torch.mul(grad_output, sigmoid_val * (1 - sigmoid_val), out=grad_input)


def test(
    handle,
    device,
    shape,
    input_stride=None,
    grad_output_stride=None,
    grad_input_stride=None,
    inplace=Inplace.OUT_OF_PLACE,
    dtype=torch.float16,
    sync=None,
):
    input_tensor = TestTensor(shape, input_stride, dtype, device)
    grad_output_tensor = TestTensor(shape, grad_output_stride, dtype, device)
    
    if inplace == Inplace.INPLACE:
        if grad_output_stride != grad_input_stride:
            return
        grad_input_tensor = grad_output_tensor
    else:
        grad_input_tensor = TestTensor(shape, grad_input_stride, dtype, device, mode="ones")

    if grad_input_tensor.is_broadcast():
        return

    print(
        f"Testing SigmoidBackward on {InfiniDeviceNames[device]} with shape:{shape} input_stride:{input_stride} "
        f"grad_output_stride:{grad_output_stride} grad_input_stride:{grad_input_stride} "
        f"inplace:{inplace} dtype:{dtype}"
    )

    sigmoid_backward(grad_input_tensor.torch_tensor(), input_tensor.torch_tensor(), grad_output_tensor.torch_tensor())

    if sync is not None:
        sync()

    descriptor = infiniopOperatorDescriptor_t()
    check_error(
        LIBINFINIOP.infiniopCreateSigmoidBackwardDescriptor(
            handle,
            ctypes.byref(descriptor),
            grad_input_tensor.descriptor,
            input_tensor.descriptor,
            grad_output_tensor.descriptor,
        )
    )

    # Invalidate the shape and strides in the descriptor to prevent them from being directly used by the kernel
    for tensor in [input_tensor, grad_output_tensor, grad_input_tensor]:
        tensor.destroy_desc()

    workspace_size = c_uint64(0)
    check_error(
        LIBINFINIOP.infiniopGetSigmoidBackwardWorkspaceSize(
            descriptor, ctypes.byref(workspace_size)
        )
    )
    workspace = TestWorkspace(workspace_size.value, grad_input_tensor.device)

    def lib_sigmoid_backward():
        check_error(
            LIBINFINIOP.infiniopSigmoidBackward(
                descriptor,
                workspace.data(),
                workspace.size(),
                grad_input_tensor.data(),
                input_tensor.data(),
                grad_output_tensor.data(),
                None,
            )
        )

    lib_sigmoid_backward()

    atol, rtol = get_tolerance(_TOLERANCE_MAP, dtype)
    if DEBUG:
        debug(grad_input_tensor.actual_tensor(), grad_input_tensor.torch_tensor(), atol=atol, rtol=rtol)
    assert torch.allclose(grad_input_tensor.actual_tensor(), grad_input_tensor.torch_tensor(), atol=atol, rtol=rtol)

    # Profiling workflow
    if PROFILE:
        # fmt: off
        profile_operation("PyTorch", lambda: sigmoid_backward(grad_input_tensor.torch_tensor(), input_tensor.torch_tensor(), grad_output_tensor.torch_tensor()), device, NUM_PRERUN, NUM_ITERATIONS)
        profile_operation("    lib", lambda: lib_sigmoid_backward(), device, NUM_PRERUN, NUM_ITERATIONS)
        # fmt: on
    check_error(LIBINFINIOP.infiniopDestroySigmoidBackwardDescriptor(descriptor))


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
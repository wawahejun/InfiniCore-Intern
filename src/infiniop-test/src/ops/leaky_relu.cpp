#include "ops.hpp"
#include "utils.hpp"
#include <infinirt.h>
#include <iomanip>
#include <iostream>

namespace infiniop_test::leaky_relu {
struct Test::Attributes {
    std::shared_ptr<Tensor> input;
    std::shared_ptr<Tensor> output;
    std::shared_ptr<Tensor> ans;
    float negative_slope;
};

std::shared_ptr<Test> Test::build(
    std::unordered_map<std::string, std::vector<uint8_t>> attributes,
    std::unordered_map<std::string, std::shared_ptr<Tensor>> tensors,
    double rtol, double atol) {
    auto test = std::shared_ptr<Test>(new Test(rtol, atol));
    test->_attributes = new Attributes();
    if (tensors.find("input") == tensors.end()
        || tensors.find("output") == tensors.end()
        || tensors.find("ans") == tensors.end()
        || attributes.find("negative_slope") == attributes.end()) {
        throw std::runtime_error("Invalid Test");
    }

    test->_attributes->input = tensors["input"];
    test->_attributes->output = tensors["output"];
    test->_attributes->ans = tensors["ans"];
    
    // Extract negative_slope from attributes
    auto negative_slope_data = attributes["negative_slope"];
    if (negative_slope_data.size() != sizeof(float)) {
        throw std::runtime_error("Invalid negative_slope attribute size");
    }
    test->_attributes->negative_slope = *reinterpret_cast<const float*>(negative_slope_data.data());

    return test;
}

std::shared_ptr<infiniop_test::Result> Test::run(
    infiniopHandle_t handle, infiniDevice_t device, int device_id, size_t warm_ups, size_t iterations) {
    infiniopLeakyReLUDescriptor_t op_desc;
    auto input = _attributes->input->to(device, device_id);
    auto output = _attributes->output->to(device, device_id);
    
    CHECK_OR(infiniopCreateLeakyReLUDescriptor(handle, &op_desc,
                                              output->desc(),
                                              input->desc(),
                                              _attributes->negative_slope),
             return TEST_FAILED(OP_CREATION_FAILED, "Failed to create leaky_relu descriptor."));
    size_t workspace_size;
    CHECK_OR(infiniopGetLeakyReLUWorkspaceSize(op_desc, &workspace_size),
             return TEST_FAILED(OP_CREATION_FAILED, "Failed to get workspace size."));
    void *workspace;
    CHECK_OR(infinirtMalloc(&workspace, workspace_size),
             return TEST_FAILED(OP_CREATION_FAILED, "Failed to allocate workspace."));
    CHECK_OR(infiniopLeakyReLU(op_desc, workspace, workspace_size,
                              output->data(),
                              input->data(),
                              nullptr),
             return TEST_FAILED(OP_EXECUTION_FAILED, "Failed during execution."));

    try {
        allClose(output, _attributes->ans, _rtol, _atol);
    } catch (const std::exception &e) {
        return TEST_FAILED(RESULT_INCORRECT, e.what());
    }

    double elapsed_time = 0.;

    elapsed_time = benchmark(
        [=]() {
            infiniopLeakyReLU(
                op_desc, workspace, workspace_size,
                output->data(),
                input->data(),
                nullptr);
        },
        warm_ups, iterations);

    infiniopDestroyLeakyReLUDescriptor(op_desc);
    infinirtFree(workspace);

    return TEST_PASSED(elapsed_time);
}

std::vector<std::string> Test::attribute_names() {
    return {"negative_slope"};
}

std::vector<std::string> Test::tensor_names() {
    return {"input", "output", "ans"};
}

std::vector<std::string> Test::output_names() {
    return {"output"};
}

std::string Test::toString() const {
    std::ostringstream oss;
    oss << op_name() << std::endl;
    oss << "- input: " << _attributes->input->info() << std::endl;
    oss << "- output: " << _attributes->output->info() << std::endl;
    oss << "- negative_slope: " << _attributes->negative_slope << std::endl;
    oss << std::scientific << std::setprecision(2);
    oss << "- rtol=" << _rtol << ", atol=" << _atol << std::endl;
    return oss.str();
}

Test::~Test() {
    delete _attributes;
}

} // namespace infiniop_test::leaky_relu
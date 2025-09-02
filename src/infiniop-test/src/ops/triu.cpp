#include "ops.hpp"
#include "utils.hpp"
#include <infinirt.h>
#include <iomanip>
#include <iostream>

namespace infiniop_test::triu {
struct Test::Attributes {
    std::shared_ptr<Tensor> input;
    std::shared_ptr<Tensor> output;
    std::shared_ptr<Tensor> ans;
    int diagonal;
};

std::shared_ptr<Test> Test::build(
    std::unordered_map<std::string, std::vector<uint8_t>> attributes,
    std::unordered_map<std::string, std::shared_ptr<Tensor>> tensors,
    double rtol, double atol) {
    auto test = std::shared_ptr<Test>(new Test(rtol, atol));
    test->_attributes = new Attributes();
    if (tensors.find("input") == tensors.end()
        || tensors.find("output") == tensors.end()
        || tensors.find("ans") == tensors.end()) {
        throw std::runtime_error("Invalid Test");
    }

    test->_attributes->input = tensors["input"];
    test->_attributes->output = tensors["output"];
    test->_attributes->ans = tensors["ans"];
    
    // Extract diagonal parameter
    if (attributes.find("diagonal") == attributes.end()) {
        throw std::runtime_error("Missing diagonal attribute");
    }
    test->_attributes->diagonal = *reinterpret_cast<const int*>(attributes["diagonal"].data());

    return test;
}

std::shared_ptr<infiniop_test::Result> Test::run(
    infiniopHandle_t handle, infiniDevice_t device, int device_id, size_t warm_ups, size_t iterations) {
    infiniopTriuDescriptor_t op_desc;
    auto input = _attributes->input->to(device, device_id);
    auto output = _attributes->output->to(device, device_id);
    
    CHECK_OR(infiniopCreateTriuDescriptor(handle, &op_desc,
                                         input->desc(),
                                         output->desc(),
                                         _attributes->diagonal),
             return TEST_FAILED(OP_CREATION_FAILED, "Failed to create triu descriptor."));
    size_t workspace_size;
    CHECK_OR(infiniopGetTriuWorkspaceSize(op_desc, &workspace_size),
             return TEST_FAILED(OP_CREATION_FAILED, "Failed to get workspace size."));
    void *workspace;
    CHECK_OR(infinirtMalloc(&workspace, workspace_size),
             return TEST_FAILED(OP_CREATION_FAILED, "Failed to allocate workspace."));
    CHECK_OR(infiniopTriu(op_desc, workspace, workspace_size,
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
            infiniopTriu(
                op_desc, workspace, workspace_size,
                output->data(),
                input->data(),
                nullptr);
        },
        warm_ups, iterations);

    infiniopDestroyTriuDescriptor(op_desc);
    infinirtFree(workspace);

    return TEST_PASSED(elapsed_time);
}

std::vector<std::string> Test::attribute_names() {
    return {"diagonal"};
}

std::vector<std::string> Test::tensor_names() {
    return {"input", "output", "ans"};
}

std::vector<std::string> Test::output_names() {
    return {"output"};
}

std::string Test::toString() const {
    std::ostringstream oss;
    oss << "Triu Test: "
        << "input=" << _attributes->input->info()
        << ", output=" << _attributes->output->info()
        << ", diagonal=" << _attributes->diagonal;
    return oss.str();
}

Test::~Test() {
    delete _attributes;
}

} // namespace infiniop_test::triu
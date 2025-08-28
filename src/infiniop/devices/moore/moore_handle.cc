#include "moore_common.h"

namespace device::moore {
Handle::Handle(infiniDevice_t device, int device_id)
    : InfiniopHandle{device, device_id},
      _internal(std::make_shared<Handle::Internal>(device_id)) {}

Handle::Handle(int device_id) : Handle(INFINI_DEVICE_MOORE, device_id) {}

auto Handle::internal() const -> const std::shared_ptr<Internal> & {
    return _internal;
}

Handle::Internal::Internal(int device_id) {
    musaDeviceProp prop;
    musaGetDeviceProperties(&prop, device_id);
    _warp_size = prop.warpSize;
    _max_threads_per_block = prop.maxThreadsPerBlock;
    _block_size[0] = prop.maxThreadsDim[0];
    _block_size[1] = prop.maxThreadsDim[1];
    _block_size[2] = prop.maxThreadsDim[2];
    _grid_size[0] = prop.maxGridSize[0];
    _grid_size[1] = prop.maxGridSize[1];
    _grid_size[2] = prop.maxGridSize[2];
}

infiniStatus_t Handle::Internal::useMublas(musaStream_t stream, const Fn<mublasHandle_t> &f) const {
    std::unique_ptr<mublasHandle_t> handle;
    auto opt_handle = mublas_handles.pop();
    if (opt_handle.has_value()) {
        handle = std::move(*opt_handle);
    } else {
        handle = std::make_unique<mublasHandle_t>();
        CHECK_MUBLAS(mublasCreate(&(*handle)));
    }
    CHECK_MUBLAS(mublasSetStream(*handle, stream));
    CHECK_STATUS(f(*handle));
    mublas_handles.push(std::move(handle));
    return INFINI_STATUS_SUCCESS;
}

infiniStatus_t Handle::Internal::useMudnn(musaStream_t stream, const Fn<::musa::dnn::Handle &> &f) const {
    std::unique_ptr<::musa::dnn::Handle> handle;
    auto opt_handle = mudnn_handles.pop();
    if (opt_handle.has_value()) {
        handle = std::move(*opt_handle);
    } else {
        handle = std::make_unique<::musa::dnn::Handle>();
    }
    CHECK_MUDNN(handle->SetStream(stream));
    CHECK_STATUS(f(*handle));
    mudnn_handles.push(std::move(handle));
    return INFINI_STATUS_SUCCESS;
}

int Handle::Internal::warpSize() const { return _warp_size; }
int Handle::Internal::maxThreadsPerBlock() const { return _max_threads_per_block; }
int Handle::Internal::blockSizeX() const { return _block_size[0]; }
int Handle::Internal::blockSizeY() const { return _block_size[1]; }
int Handle::Internal::blockSizeZ() const { return _block_size[2]; }
int Handle::Internal::gridSizeX() const { return _grid_size[0]; }
int Handle::Internal::gridSizeY() const { return _grid_size[1]; }
int Handle::Internal::gridSizeZ() const { return _grid_size[2]; }

infiniStatus_t Handle::create(InfiniopHandle **handle_ptr, int device_id) {
    *handle_ptr = new Handle(INFINI_DEVICE_MOORE, device_id);
    return INFINI_STATUS_SUCCESS;
}

} // namespace device::moore

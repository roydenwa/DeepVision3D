// Modified from https://github.com/CVMI-Lab/PAConv/tree/main/scene_seg/lib/pointops/src/knnquery_heap

#include <torch/serialize/tensor.h>
#include <torch/extension.h>
#include <vector>
// #include <THC/THC.h> see https://github.com/open-mmlab/mmdetection3d/issues/1332#issuecomment-2594408887
#include <ATen/cuda/CUDAEvent.h>
#include <ATen/cuda/CUDAContext.h>

// extern THCState *state;

#define CHECK_CUDA(x) TORCH_CHECK(x.is_cuda(), #x, " must be a CUDAtensor ")
#define CHECK_CONTIGUOUS(x) TORCH_CHECK(x.is_contiguous(), #x, " must be contiguous ")
#define CHECK_INPUT(x) CHECK_CUDA(x);CHECK_CONTIGUOUS(x)


void knn_kernel_launcher(
    int b,
    int n,
    int m,
    int nsample,
    const float *xyz,
    const int *xyz_cnt,
    const float* new_xyz,
    const int *new_xyz_cnt,
    int *idx,
    float *dist2,
    cudaStream_t stream
    );

void knn_wrapper(int b, int n, int m, int nsample,
    at::Tensor xyz_tensor, at::Tensor xyz_cnt_tensor,
    at::Tensor new_xyz_tensor, at::Tensor new_xyz_cnt_tensor,
    at::Tensor idx_tensor, at::Tensor dist2_tensor)
{
    CHECK_INPUT(xyz_tensor);
    CHECK_INPUT(xyz_cnt_tensor);

    CHECK_INPUT(new_xyz_tensor);
    CHECK_INPUT(new_xyz_cnt_tensor);

    const float *xyz = xyz_tensor.data_ptr<float>();
    const int *xyz_cnt = xyz_cnt_tensor.data_ptr<int>();

    const float *new_xyz = new_xyz_tensor.data_ptr<float>();
    const int *new_xyz_cnt = new_xyz_cnt_tensor.data_ptr<int>();

    int *idx = idx_tensor.data_ptr<int>();
    float *dist2 = dist2_tensor.data_ptr<float>();

    cudaStream_t stream = at::cuda::getCurrentCUDAStream();

    knn_kernel_launcher(
        b, n, m, nsample,
        xyz, xyz_cnt,
        new_xyz, new_xyz_cnt,
        idx, dist2, stream);
}


PYBIND11_MODULE(TORCH_EXTENSION_NAME, m) {
    m.def("knn_wrapper", &knn_wrapper, "knn_wrapper");
}

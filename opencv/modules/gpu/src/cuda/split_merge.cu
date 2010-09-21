#include "opencv2/gpu/devmem2d.hpp"
#include "cuda_shared.hpp"

namespace cv { namespace gpu { namespace split_merge {

    template <typename T, size_t elem_size = sizeof(T)>
    struct TypeTraits 
    {
        typedef T type;
        typedef T type2;
        typedef T type3;
        typedef T type4;
    };

    template <typename T>
    struct TypeTraits<T, 1>
    {
        typedef char type;
        typedef char2 type2;
        typedef char3 type3;
        typedef char4 type4;
    };

    template <typename T>
    struct TypeTraits<T, 2>
    {
        typedef short type;
        typedef short2 type2;
        typedef short3 type3;
        typedef short4 type4;
    };

    template <typename T>
    struct TypeTraits<T, 4> 
    {
        typedef int type;
        typedef int2 type2;
        typedef int3 type3;
        typedef int4 type4;
    };

    template <typename T>
    struct TypeTraits<T, 8> 
    {
        typedef double type;
        typedef double2 type2;
        //typedef double3 type3;
        //typedef double4 type3;
    };

    typedef void (*MergeFunction)(const DevMem2D* src, DevMem2D& dst, const cudaStream_t& stream);
    typedef void (*SplitFunction)(const DevMem2D& src, DevMem2D* dst, const cudaStream_t& stream);

    //------------------------------------------------------------
    // Merge

    template <typename T> 
    static void mergeC2_(const DevMem2D* src, DevMem2D& dst, const cudaStream_t& stream)
    {
        dim3 blockDim(32, 8);
        dim3 gridDim(divUp(dst.cols, blockDim.x), divUp(dst.rows, blockDim.y));
        mergeC2_<T><<<gridDim, blockDim, 0, stream>>>(
                src[0].ptr, src[0].step, 
                src[1].ptr, src[1].step,
                dst.rows, dst.cols, dst.ptr, dst.step);
        if (stream == 0)
            cudaSafeCall(cudaThreadSynchronize());
    }


    template <typename T> 
    static void mergeC3_(const DevMem2D* src, DevMem2D& dst, const cudaStream_t& stream)
    {
        dim3 blockDim(32, 8);
        dim3 gridDim(divUp(dst.cols, blockDim.x), divUp(dst.rows, blockDim.y));
        mergeC3_<T><<<gridDim, blockDim, 0, stream>>>(
                src[0].ptr, src[0].step, 
                src[1].ptr, src[1].step,
                src[2].ptr, src[2].step,
                dst.rows, dst.cols, dst.ptr, dst.step);
        if (stream == 0)
            cudaSafeCall(cudaThreadSynchronize());
    }


    template <typename T> 
    static void mergeC4_(const DevMem2D* src, DevMem2D& dst, const cudaStream_t& stream)
    {
        dim3 blockDim(32, 8);
        dim3 gridDim(divUp(dst.cols, blockDim.x), divUp(dst.rows, blockDim.y));
        mergeC4_<T><<<gridDim, blockDim, 0, stream>>>(
                src[0].ptr, src[0].step, 
                src[1].ptr, src[1].step,
                src[2].ptr, src[2].step,
                src[3].ptr, src[3].step,
                dst.rows, dst.cols, dst.ptr, dst.step);
        if (stream == 0)
            cudaSafeCall(cudaThreadSynchronize());
    }


    extern "C" void merge_caller(const DevMem2D* src, DevMem2D& dst, 
                                 int total_channels, int elem_size, 
                                 const cudaStream_t& stream) 
    {
        static MergeFunction merge_func_tbl[] = 
        {
            mergeC2_<char>, mergeC2_<short>, mergeC2_<int>, 0, mergeC2_<double>,
            mergeC3_<char>, mergeC3_<short>, mergeC3_<int>, 0, mergeC3_<double>,
            mergeC4_<char>, mergeC4_<short>, mergeC4_<int>, 0, mergeC4_<double>,
        };

        int merge_func_id = (total_channels - 2) * 5 + (elem_size >> 1);
        MergeFunction merge_func = merge_func_tbl[merge_func_id];

        if (merge_func == 0)
            cv::gpu::error("Unsupported channel count or data type", __FILE__, __LINE__);

        merge_func(src, dst, stream);
    }


    template <typename T>
    __global__ void mergeC2_(const uchar* src0, size_t src0_step, 
                             const uchar* src1, size_t src1_step, 
                             int rows, int cols, uchar* dst, size_t dst_step)
    {
        typedef typename TypeTraits<T>::type2 dst_type;

        const int x = blockIdx.x * blockDim.x + threadIdx.x;
        const int y = blockIdx.y * blockDim.y + threadIdx.y;

        const T* src0_y = (const T*)(src0 + y * src0_step);
        const T* src1_y = (const T*)(src1 + y * src1_step);
        dst_type* dst_y = (dst_type*)(dst + y * dst_step);

        if (x < cols && y < rows) 
        {                        
            dst_type dst_elem;
            dst_elem.x = src0_y[x];
            dst_elem.y = src1_y[x];
            dst_y[x] = dst_elem;
        }
    }


    template <typename T>
    __global__ void mergeC3_(const uchar* src0, size_t src0_step, 
                             const uchar* src1, size_t src1_step, 
                             const uchar* src2, size_t src2_step, 
                             int rows, int cols, uchar* dst, size_t dst_step)
    {
        typedef typename TypeTraits<T>::type3 dst_type;

        const int x = blockIdx.x * blockDim.x + threadIdx.x;
        const int y = blockIdx.y * blockDim.y + threadIdx.y;

        const T* src0_y = (const T*)(src0 + y * src0_step);
        const T* src1_y = (const T*)(src1 + y * src1_step);
        const T* src2_y = (const T*)(src2 + y * src2_step);
        dst_type* dst_y = (dst_type*)(dst + y * dst_step);

        if (x < cols && y < rows) 
        {                        
            dst_type dst_elem;
            dst_elem.x = src0_y[x];
            dst_elem.y = src1_y[x];
            dst_elem.z = src2_y[x];
            dst_y[x] = dst_elem;
        }
    }


    template <>
    __global__ void mergeC3_<double>(const uchar* src0, size_t src0_step, 
                             const uchar* src1, size_t src1_step, 
                             const uchar* src2, size_t src2_step, 
                             int rows, int cols, uchar* dst, size_t dst_step)
    {
        const int x = blockIdx.x * blockDim.x + threadIdx.x;
        const int y = blockIdx.y * blockDim.y + threadIdx.y;

        const double* src0_y = (const double*)(src0 + y * src0_step);
        const double* src1_y = (const double*)(src1 + y * src1_step);
        const double* src2_y = (const double*)(src2 + y * src2_step);
        double* dst_y = (double*)(dst + y * dst_step);

        if (x < cols && y < rows) 
        {                        
            dst_y[3 * x] = src0_y[x];
            dst_y[3 * x + 1] = src1_y[x];
            dst_y[3 * x + 2] = src2_y[x];
        }
    }


    template <typename T>
    __global__ void mergeC4_(const uchar* src0, size_t src0_step, 
                             const uchar* src1, size_t src1_step, 
                             const uchar* src2, size_t src2_step, 
                             const uchar* src3, size_t src3_step, 
                             int rows, int cols, uchar* dst, size_t dst_step)
    {
        typedef typename TypeTraits<T>::type4 dst_type;

        const int x = blockIdx.x * blockDim.x + threadIdx.x;
        const int y = blockIdx.y * blockDim.y + threadIdx.y;

        const T* src0_y = (const T*)(src0 + y * src0_step);
        const T* src1_y = (const T*)(src1 + y * src1_step);
        const T* src2_y = (const T*)(src2 + y * src2_step);
        const T* src3_y = (const T*)(src3 + y * src3_step);
        dst_type* dst_y = (dst_type*)(dst + y * dst_step);

        if (x < cols && y < rows) 
        {                        
            dst_type dst_elem;
            dst_elem.x = src0_y[x];
            dst_elem.y = src1_y[x];
            dst_elem.z = src2_y[x];
            dst_elem.w = src3_y[x];
            dst_y[x] = dst_elem;
        }
    }


    template <>
    __global__ void mergeC4_<double>(const uchar* src0, size_t src0_step, 
                             const uchar* src1, size_t src1_step, 
                             const uchar* src2, size_t src2_step, 
                             const uchar* src3, size_t src3_step, 
                             int rows, int cols, uchar* dst, size_t dst_step)
    {
        const int x = blockIdx.x * blockDim.x + threadIdx.x;
        const int y = blockIdx.y * blockDim.y + threadIdx.y;

        const double* src0_y = (const double*)(src0 + y * src0_step);
        const double* src1_y = (const double*)(src1 + y * src1_step);
        const double* src2_y = (const double*)(src2 + y * src2_step);
        const double* src3_y = (const double*)(src3 + y * src3_step);
        double2* dst_y = (double2*)(dst + y * dst_step);

        if (x < cols && y < rows) 
        {                        
            dst_y[2 * x] = make_double2(src0_y[x], src1_y[x]);
            dst_y[2 * x + 1] = make_double2(src2_y[x], src3_y[x]);
        }
    }

    //------------------------------------------------------------
    // Split


    template <typename T> 
    static void splitC2_(const DevMem2D& src, DevMem2D* dst, const cudaStream_t& stream)
    {
        dim3 blockDim(32, 8);
        dim3 gridDim(divUp(src.cols, blockDim.x), divUp(src.rows, blockDim.y));
        splitC2_<T><<<gridDim, blockDim, 0, stream>>>(
                src.ptr, src.step, src.rows, src.cols,
                dst[0].ptr, dst[0].step, 
                dst[1].ptr, dst[1].step);
        if (stream == 0)
            cudaSafeCall(cudaThreadSynchronize());
    }


    template <typename T> 
    static void splitC3_(const DevMem2D& src, DevMem2D* dst, const cudaStream_t& stream)
    {
        dim3 blockDim(32, 8);
        dim3 gridDim(divUp(src.cols, blockDim.x), divUp(src.rows, blockDim.y));
        splitC3_<T><<<gridDim, blockDim, 0, stream>>>(
                src.ptr, src.step, src.rows, src.cols,
                dst[0].ptr, dst[0].step, 
                dst[1].ptr, dst[1].step,
                dst[2].ptr, dst[2].step);         
        if (stream == 0)
            cudaSafeCall(cudaThreadSynchronize());
    }


    template <typename T> 
    static void splitC4_(const DevMem2D& src, DevMem2D* dst, const cudaStream_t& stream)
    {
        dim3 blockDim(32, 8);
        dim3 gridDim(divUp(src.cols, blockDim.x), divUp(src.rows, blockDim.y));
        splitC4_<T><<<gridDim, blockDim, 0, stream>>>(
                 src.ptr, src.step, src.rows, src.cols,
                 dst[0].ptr, dst[0].step, 
                 dst[1].ptr, dst[1].step,
                 dst[2].ptr, dst[2].step,
                 dst[3].ptr, dst[3].step);
        if (stream == 0)
            cudaSafeCall(cudaThreadSynchronize());
    }


    extern "C" void split_caller(const DevMem2D& src, DevMem2D* dst, 
                                 int num_channels, int elem_size1, 
                                 const cudaStream_t& stream) 
    {
        static SplitFunction split_func_tbl[] = 
        {
            splitC2_<char>, splitC2_<short>, splitC2_<int>, 0, splitC2_<double>,
            splitC3_<char>, splitC3_<short>, splitC3_<int>, 0, splitC3_<double>,
            splitC4_<char>, splitC4_<short>, splitC4_<int>, 0, splitC4_<double>,
        };

        int split_func_id = (num_channels - 2) * 5 + (elem_size1 >> 1);
        SplitFunction split_func = split_func_tbl[split_func_id];

        if (split_func == 0)
            cv::gpu::error("Unsupported channel count or data type", __FILE__, __LINE__);

        split_func(src, dst, stream);
    }


    template <typename T>
    __global__ void splitC2_(const uchar* src, size_t src_step, 
                            int rows, int cols,
                            uchar* dst0, size_t dst0_step,
                            uchar* dst1, size_t dst1_step)
    {
        typedef typename TypeTraits<T>::type2 src_type;

        const int x = blockIdx.x * blockDim.x + threadIdx.x;
        const int y = blockIdx.y * blockDim.y + threadIdx.y;

        const src_type* src_y = (const src_type*)(src + y * src_step);
        T* dst0_y = (T*)(dst0 + y * dst0_step);
        T* dst1_y = (T*)(dst1 + y * dst1_step);

        if (x < cols && y < rows) 
        {
            src_type src_elem = src_y[x];
            dst0_y[x] = src_elem.x;
            dst1_y[x] = src_elem.y;
        }
    }


    template <typename T>
    __global__ void splitC3_(const uchar* src, size_t src_step, 
                            int rows, int cols,
                            uchar* dst0, size_t dst0_step,
                            uchar* dst1, size_t dst1_step,
                            uchar* dst2, size_t dst2_step)
    {
        typedef typename TypeTraits<T>::type3 src_type;

        const int x = blockIdx.x * blockDim.x + threadIdx.x;
        const int y = blockIdx.y * blockDim.y + threadIdx.y;

        const src_type* src_y = (const src_type*)(src + y * src_step);
        T* dst0_y = (T*)(dst0 + y * dst0_step);
        T* dst1_y = (T*)(dst1 + y * dst1_step);
        T* dst2_y = (T*)(dst2 + y * dst2_step);

        if (x < cols && y < rows) 
        {
            src_type src_elem = src_y[x];
            dst0_y[x] = src_elem.x;
            dst1_y[x] = src_elem.y;
            dst2_y[x] = src_elem.z;
        }
    }


    template <>
    __global__ void splitC3_<double>(
            const uchar* src, size_t src_step, int rows, int cols,
            uchar* dst0, size_t dst0_step,
            uchar* dst1, size_t dst1_step,
            uchar* dst2, size_t dst2_step)
    {
        const int x = blockIdx.x * blockDim.x + threadIdx.x;
        const int y = blockIdx.y * blockDim.y + threadIdx.y;

        const double* src_y = (const double*)(src + y * src_step);
        double* dst0_y = (double*)(dst0 + y * dst0_step);
        double* dst1_y = (double*)(dst1 + y * dst1_step);
        double* dst2_y = (double*)(dst2 + y * dst2_step);

        if (x < cols && y < rows) 
        {
            dst0_y[x] = src_y[3 * x];
            dst1_y[x] = src_y[3 * x + 1];
            dst2_y[x] = src_y[3 * x + 2];
        }
    }


    template <typename T>
    __global__ void splitC4_(const uchar* src, size_t src_step, int rows, int cols,
                            uchar* dst0, size_t dst0_step,
                            uchar* dst1, size_t dst1_step,
                            uchar* dst2, size_t dst2_step,
                            uchar* dst3, size_t dst3_step)
    {
        typedef typename TypeTraits<T>::type4 src_type;

        const int x = blockIdx.x * blockDim.x + threadIdx.x;
        const int y = blockIdx.y * blockDim.y + threadIdx.y;

        const src_type* src_y = (const src_type*)(src + y * src_step);
        T* dst0_y = (T*)(dst0 + y * dst0_step);
        T* dst1_y = (T*)(dst1 + y * dst1_step);
        T* dst2_y = (T*)(dst2 + y * dst2_step);
        T* dst3_y = (T*)(dst3 + y * dst3_step);

        if (x < cols && y < rows) 
        {
            src_type src_elem = src_y[x];
            dst0_y[x] = src_elem.x;
            dst1_y[x] = src_elem.y;
            dst2_y[x] = src_elem.z;
            dst3_y[x] = src_elem.w;
        }
    }


    template <>
    __global__ void splitC4_<double>(
            const uchar* src, size_t src_step, int rows, int cols,
            uchar* dst0, size_t dst0_step,
            uchar* dst1, size_t dst1_step,
            uchar* dst2, size_t dst2_step,
            uchar* dst3, size_t dst3_step)
    {
        const int x = blockIdx.x * blockDim.x + threadIdx.x;
        const int y = blockIdx.y * blockDim.y + threadIdx.y;

        const double2* src_y = (const double2*)(src + y * src_step);
        double* dst0_y = (double*)(dst0 + y * dst0_step);
        double* dst1_y = (double*)(dst1 + y * dst1_step);
        double* dst2_y = (double*)(dst2 + y * dst2_step);
        double* dst3_y = (double*)(dst3 + y * dst3_step);

        if (x < cols && y < rows) 
        {
            double2 src_elem1 = src_y[2 * x];
            double2 src_elem2 = src_y[2 * x + 1];
            dst0_y[x] = src_elem1.x;
            dst1_y[x] = src_elem1.y;
            dst2_y[x] = src_elem2.x;
            dst3_y[x] = src_elem2.y;
        }
    }

}}} // namespace cv::gpu::split_merge
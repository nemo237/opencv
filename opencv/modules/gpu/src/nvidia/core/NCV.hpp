/*M///////////////////////////////////////////////////////////////////////////////////////
//
// IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING. 
// 
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2009-2010, NVIDIA Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#ifndef _ncv_hpp_
#define _ncv_hpp_

#if (defined WIN32 || defined _WIN32 || defined WINCE) && defined CVAPI_EXPORTS //&& !defined(__CUDACC__) 
    #define NCV_EXPORTS __declspec(dllexport)
#else
    #define NCV_EXPORTS
#endif

#include <cuda_runtime.h>


//==============================================================================
//
// Compile-time assert functionality
//
//==============================================================================


/**
* Compile-time assert namespace
*/
namespace NcvCTprep
{
    template <bool x>
    struct CT_ASSERT_FAILURE;

    template <>
    struct CT_ASSERT_FAILURE<true> {};

    template <int x>
    struct assertTest{};
}


#define NCV_CT_PREP_PASTE_AUX(a,b)      a##b                           ///< Concatenation indirection macro
#define NCV_CT_PREP_PASTE(a,b)          NCV_CT_PREP_PASTE_AUX(a, b)  ///< Concatenation macro


/**
* Performs compile-time assertion of a condition on the file scope
*/
#define NCV_CT_ASSERT(X) \
    typedef NcvCTprep::assertTest<sizeof(NcvCTprep::CT_ASSERT_FAILURE< (bool)(X) >)> \
    NCV_CT_PREP_PASTE(__ct_assert_typedef_, __LINE__)



//==============================================================================
//
// Alignment macros
//
//==============================================================================


#if !defined(__align__) && !defined(__CUDACC__)
    #if defined(_WIN32) || defined(_WIN64)
        #define __align__(n)         __declspec(align(n))
    #elif defined(__unix__)
        #define __align__(n)         __attribute__((__aligned__(n)))
    #endif
#endif


//==============================================================================
//
// Integral and compound types of guaranteed size
//
//==============================================================================


typedef               bool NcvBool;
typedef          long long Ncv64s;
typedef unsigned long long Ncv64u;
typedef                int Ncv32s;
typedef       unsigned int Ncv32u;
typedef              short Ncv16s;
typedef     unsigned short Ncv16u;
typedef               char Ncv8s;
typedef      unsigned char Ncv8u;
typedef              float Ncv32f;
typedef             double Ncv64f;


struct NcvRect8u
{
    Ncv8u x;
    Ncv8u y;
    Ncv8u width;
    Ncv8u height;
    NcvRect8u() : x(0), y(0), width(0), height(0) {};
    NcvRect8u(Ncv8u x, Ncv8u y, Ncv8u width, Ncv8u height) : x(x), y(y), width(width), height(height) {}
};


struct NcvRect32s
{
    Ncv32s x;          ///< x-coordinate of upper left corner.
    Ncv32s y;          ///< y-coordinate of upper left corner.
    Ncv32s width;      ///< Rectangle width.
    Ncv32s height;     ///< Rectangle height.
    NcvRect32s() : x(0), y(0), width(0), height(0) {};
    NcvRect32s(Ncv32s x, Ncv32s y, Ncv32s width, Ncv32s height) : x(x), y(y), width(width), height(height) {}
};


struct NcvRect32u
{
    Ncv32u x;          ///< x-coordinate of upper left corner.
    Ncv32u y;          ///< y-coordinate of upper left corner.
    Ncv32u width;      ///< Rectangle width.
    Ncv32u height;     ///< Rectangle height.
    NcvRect32u() : x(0), y(0), width(0), height(0) {};
    NcvRect32u(Ncv32u x, Ncv32u y, Ncv32u width, Ncv32u height) : x(x), y(y), width(width), height(height) {}
};


struct NcvSize32s
{
    Ncv32s width;  ///< Rectangle width.
    Ncv32s height; ///< Rectangle height.
    NcvSize32s() : width(0), height(0) {};
    NcvSize32s(Ncv32s width, Ncv32s height) : width(width), height(height) {}
};


struct NcvSize32u
{
    Ncv32u width;  ///< Rectangle width.
    Ncv32u height; ///< Rectangle height.
    NcvSize32u() : width(0), height(0) {};
    NcvSize32u(Ncv32u width, Ncv32u height) : width(width), height(height) {}
};


NCV_CT_ASSERT(sizeof(NcvBool) <= 4);
NCV_CT_ASSERT(sizeof(Ncv64s) == 8);
NCV_CT_ASSERT(sizeof(Ncv64u) == 8);
NCV_CT_ASSERT(sizeof(Ncv32s) == 4);
NCV_CT_ASSERT(sizeof(Ncv32u) == 4);
NCV_CT_ASSERT(sizeof(Ncv16s) == 2);
NCV_CT_ASSERT(sizeof(Ncv16u) == 2);
NCV_CT_ASSERT(sizeof(Ncv8s) == 1);
NCV_CT_ASSERT(sizeof(Ncv8u) == 1);
NCV_CT_ASSERT(sizeof(Ncv32f) == 4);
NCV_CT_ASSERT(sizeof(Ncv64f) == 8);
NCV_CT_ASSERT(sizeof(NcvRect8u) == sizeof(Ncv32u));
NCV_CT_ASSERT(sizeof(NcvRect32s) == 4 * sizeof(Ncv32s));
NCV_CT_ASSERT(sizeof(NcvRect32u) == 4 * sizeof(Ncv32u));
NCV_CT_ASSERT(sizeof(NcvSize32u) == 2 * sizeof(Ncv32u));


//==============================================================================
//
// Persistent constants
//
//==============================================================================


const Ncv32u K_WARP_SIZE = 32;
const Ncv32u K_LOG2_WARP_SIZE = 5;


//==============================================================================
//
// Error handling
//
//==============================================================================


#define NCV_CT_PREP_STRINGIZE_AUX(x)    #x
#define NCV_CT_PREP_STRINGIZE(x)        NCV_CT_PREP_STRINGIZE_AUX(x)


NCV_EXPORTS void ncvDebugOutput(const char *msg, ...);


typedef void NCVDebugOutputHandler(const char* msg);


NCV_EXPORTS void ncvSetDebugOutputHandler(NCVDebugOutputHandler* func);


#define ncvAssertPrintCheck(pred, msg) \
    ((pred) ? true : (ncvDebugOutput("\n%s\n", \
    "NCV Assertion Failed: " msg ", file=" __FILE__ ", line=" NCV_CT_PREP_STRINGIZE(__LINE__) \
    ), false))


#define ncvAssertPrintReturn(pred, msg, err) \
    if (ncvAssertPrintCheck(pred, msg)) ; else return err


#define ncvAssertReturn(pred, err) \
    do \
    { \
        if (!(pred)) \
        { \
            ncvDebugOutput("\n%s%d%s\n", "NCV Assertion Failed: retcode=", (int)err, ", file=" __FILE__ ", line=" NCV_CT_PREP_STRINGIZE(__LINE__)); \
            return err; \
        } \
    } while (0)


#define ncvAssertReturnNcvStat(ncvOp) \
    do \
    { \
        NCVStatus _ncvStat = ncvOp; \
        if (NCV_SUCCESS != _ncvStat) \
        { \
            ncvDebugOutput("\n%s%d%s\n", "NCV Assertion Failed: NcvStat=", (int)_ncvStat, ", file=" __FILE__ ", line=" NCV_CT_PREP_STRINGIZE(__LINE__)); \
            return _ncvStat; \
        } \
    } while (0)


#define ncvAssertCUDAReturn(cudacall, errCode) \
    do \
    { \
        cudaError_t resCall = cudacall; \
        cudaError_t resGLE = cudaGetLastError(); \
        if (cudaSuccess != resCall || cudaSuccess != resGLE) \
        { \
            ncvDebugOutput("\n%s%d%s\n", "NCV CUDA Assertion Failed: cudaError_t=", (int)(resCall | resGLE), ", file=" __FILE__ ", line=" NCV_CT_PREP_STRINGIZE(__LINE__)); \
            return errCode; \
        } \
    } while (0)


/**
* Return-codes for status notification, errors and warnings
*/
enum NCVStatus
{
    //NCV statuses
    NCV_SUCCESS,

    NCV_CUDA_ERROR,
    NCV_NPP_ERROR,
    NCV_FILE_ERROR,

    NCV_NULL_PTR,
    NCV_INCONSISTENT_INPUT,
    NCV_TEXTURE_BIND_ERROR,
    NCV_DIMENSIONS_INVALID,

    NCV_INVALID_ROI,
    NCV_INVALID_STEP,
    NCV_INVALID_SCALE,

    NCV_ALLOCATOR_NOT_INITIALIZED,
    NCV_ALLOCATOR_BAD_ALLOC,
    NCV_ALLOCATOR_BAD_DEALLOC,
    NCV_ALLOCATOR_INSUFFICIENT_CAPACITY,
    NCV_ALLOCATOR_DEALLOC_ORDER,
    NCV_ALLOCATOR_BAD_REUSE,

    NCV_MEM_COPY_ERROR,
    NCV_MEM_RESIDENCE_ERROR,
    NCV_MEM_INSUFFICIENT_CAPACITY,

    NCV_HAAR_INVALID_PIXEL_STEP,
    NCV_HAAR_TOO_MANY_FEATURES_IN_CLASSIFIER,
    NCV_HAAR_TOO_MANY_FEATURES_IN_CASCADE,
    NCV_HAAR_TOO_LARGE_FEATURES,
    NCV_HAAR_XML_LOADING_EXCEPTION,

    NCV_NOIMPL_HAAR_TILTED_FEATURES,

    NCV_WARNING_HAAR_DETECTIONS_VECTOR_OVERFLOW,

    //NPP statuses
    NPPST_SUCCESS = NCV_SUCCESS,              ///< Successful operation (same as NPP_NO_ERROR)
    NPPST_ERROR,                              ///< Unknown error
    NPPST_CUDA_KERNEL_EXECUTION_ERROR,        ///< CUDA kernel execution error
    NPPST_NULL_POINTER_ERROR,                 ///< NULL pointer argument error
    NPPST_TEXTURE_BIND_ERROR,                 ///< CUDA texture binding error or non-zero offset returned
    NPPST_MEMCPY_ERROR,                       ///< CUDA memory copy error
    NPPST_MEM_ALLOC_ERR,                      ///< CUDA memory allocation error
    NPPST_MEMFREE_ERR,                        ///< CUDA memory deallocation error

    //NPPST statuses
    NPPST_INVALID_ROI,                        ///< Invalid region of interest argument
    NPPST_INVALID_STEP,                       ///< Invalid image lines step argument (check sign, alignment, relation to image width)
    NPPST_INVALID_SCALE,                      ///< Invalid scale parameter passed
    NPPST_MEM_INSUFFICIENT_BUFFER,            ///< Insufficient user-allocated buffer
    NPPST_MEM_RESIDENCE_ERROR,                ///< Memory residence error detected (check if pointers should be device or pinned)
    NPPST_MEM_INTERNAL_ERROR,                 ///< Internal memory management error
};


#define NCV_SET_SKIP_COND(x) \
    bool __ncv_skip_cond = x


#define NCV_RESET_SKIP_COND(x) \
    __ncv_skip_cond = x


#define NCV_SKIP_COND_BEGIN \
    if (!__ncv_skip_cond) {


#define NCV_SKIP_COND_END \
    }


//==============================================================================
//
// Timer
//
//==============================================================================


typedef struct _NcvTimer *NcvTimer;

NCV_EXPORTS NcvTimer ncvStartTimer(void);

NCV_EXPORTS double ncvEndQueryTimerUs(NcvTimer t);

NCV_EXPORTS double ncvEndQueryTimerMs(NcvTimer t);


//==============================================================================
//
// Memory management classes template compound types
//
//==============================================================================


/**
* Calculates the aligned top bound value
*/
NCV_EXPORTS Ncv32u alignUp(Ncv32u what, Ncv32u alignment);


/**
* NCVMemoryType
*/
enum NCVMemoryType
{
    NCVMemoryTypeNone,
    NCVMemoryTypeHostPageable,
    NCVMemoryTypeHostPinned,
    NCVMemoryTypeDevice
};


/**
* NCVMemPtr
*/
struct NCV_EXPORTS NCVMemPtr
{
    void *ptr;
    NCVMemoryType memtype;
    void clear();
};


/**
* NCVMemSegment
*/
struct NCV_EXPORTS NCVMemSegment
{
    NCVMemPtr begin;
    size_t size;
    void clear();
};


/**
* INCVMemAllocator (Interface)
*/
class NCV_EXPORTS INCVMemAllocator
{
public:
    virtual ~INCVMemAllocator() = 0;

    virtual NCVStatus alloc(NCVMemSegment &seg, size_t size) = 0;
    virtual NCVStatus dealloc(NCVMemSegment &seg) = 0;

    virtual NcvBool isInitialized(void) const = 0;
    virtual NcvBool isCounting(void) const = 0;
    
    virtual NCVMemoryType memType(void) const = 0;
    virtual Ncv32u alignment(void) const = 0;
    virtual size_t maxSize(void) const = 0;
};

inline INCVMemAllocator::~INCVMemAllocator() {}


/**
* NCVMemStackAllocator
*/
class NCV_EXPORTS NCVMemStackAllocator : public INCVMemAllocator
{
    NCVMemStackAllocator();
    NCVMemStackAllocator(const NCVMemStackAllocator &);

public:

    explicit NCVMemStackAllocator(Ncv32u alignment);
    NCVMemStackAllocator(NCVMemoryType memT, size_t capacity, Ncv32u alignment, void *reusePtr=NULL);
    virtual ~NCVMemStackAllocator();

    virtual NCVStatus alloc(NCVMemSegment &seg, size_t size);
    virtual NCVStatus dealloc(NCVMemSegment &seg);

    virtual NcvBool isInitialized(void) const;
    virtual NcvBool isCounting(void) const;

    virtual NCVMemoryType memType(void) const;
    virtual Ncv32u alignment(void) const;
    virtual size_t maxSize(void) const;

private:

    NCVMemoryType _memType;
    Ncv32u _alignment;
    Ncv8u *allocBegin;
    Ncv8u *begin;
    Ncv8u *end;
    size_t currentSize;
    size_t _maxSize;
    NcvBool bReusesMemory;
};


/**
* NCVMemNativeAllocator
*/
class NCV_EXPORTS NCVMemNativeAllocator : public INCVMemAllocator
{
public:

    NCVMemNativeAllocator(NCVMemoryType memT, Ncv32u alignment);
    virtual ~NCVMemNativeAllocator();

    virtual NCVStatus alloc(NCVMemSegment &seg, size_t size);
    virtual NCVStatus dealloc(NCVMemSegment &seg);

    virtual NcvBool isInitialized(void) const;
    virtual NcvBool isCounting(void) const;

    virtual NCVMemoryType memType(void) const;
    virtual Ncv32u alignment(void) const;
    virtual size_t maxSize(void) const;

private:

    NCVMemNativeAllocator();
    NCVMemNativeAllocator(const NCVMemNativeAllocator &);

    NCVMemoryType _memType;
    Ncv32u _alignment;
    size_t currentSize;
    size_t _maxSize;
};


/**
* Copy dispatcher
*/
NCV_EXPORTS NCVStatus memSegCopyHelper(void *dst, NCVMemoryType dstType,
                                       const void *src, NCVMemoryType srcType,
                                       size_t sz, cudaStream_t cuStream);


/**
* NCVVector (1D)
*/
template <class T>
class NCVVector
{
    NCVVector(const NCVVector &);

public:

    NCVVector()
    {
        clear();
    }

	virtual ~NCVVector() {}

    void clear()
    {
        _ptr = NULL;
        _length = 0;
        _memtype = NCVMemoryTypeNone;
    }

    NCVStatus copySolid(NCVVector<T> &dst, cudaStream_t cuStream, size_t howMuch=0)
    {
        if (howMuch == 0)
        {
            ncvAssertReturn(dst._length == this->_length, NCV_MEM_COPY_ERROR);
            howMuch = this->_length * sizeof(T);
        }
        else
        {
            ncvAssertReturn(dst._length * sizeof(T) >= howMuch && 
                this->_length * sizeof(T) >= howMuch &&
                howMuch > 0, NCV_MEM_COPY_ERROR);
        }
        ncvAssertReturn((this->_ptr != NULL || this->_memtype == NCVMemoryTypeNone) && 
                        (dst._ptr != NULL || dst._memtype == NCVMemoryTypeNone), NCV_NULL_PTR);

        NCVStatus ncvStat = NCV_SUCCESS;
        if (this->_memtype != NCVMemoryTypeNone)
        {
            ncvStat = memSegCopyHelper(dst._ptr, dst._memtype,
                                       this->_ptr, this->_memtype,
                                       howMuch, cuStream);
        }

        return ncvStat;
    }

    T *ptr() const {return this->_ptr;}
    size_t length() const {return this->_length;}
    NCVMemoryType memType() const {return this->_memtype;}

protected:

    T *_ptr;
    size_t _length;
    NCVMemoryType _memtype;
};


/**
* NCVVectorAlloc
*/
template <class T>
class NCVVectorAlloc : public NCVVector<T>
{
    NCVVectorAlloc();
    NCVVectorAlloc(const NCVVectorAlloc &);
	NCVVectorAlloc& operator=(const NCVVectorAlloc<T>&);	

public:

    NCVVectorAlloc(INCVMemAllocator &allocator, Ncv32u length)
        :
        allocator(allocator)
    {
        NCVStatus ncvStat;

        this->clear();
        this->allocatedMem.clear();

        ncvStat = allocator.alloc(this->allocatedMem, length * sizeof(T));
        ncvAssertPrintReturn(ncvStat == NCV_SUCCESS, "NCVVectorAlloc ctor:: alloc failed", );

        this->_ptr = (T *)this->allocatedMem.begin.ptr;
        this->_length = length;
        this->_memtype = this->allocatedMem.begin.memtype;
    }


    ~NCVVectorAlloc()
    {
        NCVStatus ncvStat;

        ncvStat = allocator.dealloc(this->allocatedMem);
        ncvAssertPrintCheck(ncvStat == NCV_SUCCESS, "NCVVectorAlloc dtor:: dealloc failed");

        this->clear();
    }


    NcvBool isMemAllocated() const
    {
        return (this->allocatedMem.begin.ptr != NULL) || (this->allocator.isCounting());
    }


    Ncv32u getAllocatorsAlignment() const
    {
        return allocator.alignment();
    }


    NCVMemSegment getSegment() const
    {
        return allocatedMem;
    }

private:		
    INCVMemAllocator &allocator;
    NCVMemSegment allocatedMem;
};


/**
* NCVVectorReuse
*/
template <class T>
class NCVVectorReuse : public NCVVector<T>
{
    NCVVectorReuse();
    NCVVectorReuse(const NCVVectorReuse &);

public:

    explicit NCVVectorReuse(const NCVMemSegment &memSegment)
    {
        this->bReused = false;
        this->clear();

        this->_length = memSegment.size / sizeof(T);
        this->_ptr = (T *)memSegment.begin.ptr;
        this->_memtype = memSegment.begin.memtype;

        this->bReused = true;
    }


    NCVVectorReuse(const NCVMemSegment &memSegment, Ncv32u length)
    {
        this->bReused = false;
        this->clear();

        ncvAssertPrintReturn(length * sizeof(T) <= memSegment.size, \
            "NCVVectorReuse ctor:: memory binding failed due to size mismatch", );

        this->_length = length;
        this->_ptr = (T *)memSegment.begin.ptr;
        this->_memtype = memSegment.begin.memtype;

        this->bReused = true;
    }


    NcvBool isMemReused() const
    {
        return this->bReused;
    }

private:

    NcvBool bReused;
};


/**
* NCVMatrix (2D)
*/
template <class T>
class NCVMatrix
{
    NCVMatrix(const NCVMatrix &);

public:

    NCVMatrix()
    {
        clear();
    }

	virtual ~NCVMatrix() {}


    void clear()
    {
        _ptr = NULL;
        _pitch = 0;
        _width = 0;
        _height = 0;
        _memtype = NCVMemoryTypeNone;
    }


    Ncv32u stride() const
    {
        return _pitch / sizeof(T);
    }


    NCVStatus copySolid(NCVMatrix<T> &dst, cudaStream_t cuStream, size_t howMuch=0)
    {
        if (howMuch == 0)
        {
            ncvAssertReturn(dst._pitch == this->_pitch &&
                            dst._height == this->_height, NCV_MEM_COPY_ERROR);
            howMuch = this->_pitch * this->_height;
        }
        else
        {
            ncvAssertReturn(dst._pitch * dst._height >= howMuch && 
                            this->_pitch * this->_height >= howMuch &&
                            howMuch > 0, NCV_MEM_COPY_ERROR);
        }
        ncvAssertReturn((this->_ptr != NULL || this->_memtype == NCVMemoryTypeNone) && 
                        (dst._ptr != NULL || dst._memtype == NCVMemoryTypeNone), NCV_NULL_PTR);

        NCVStatus ncvStat = NCV_SUCCESS;
        if (this->_memtype != NCVMemoryTypeNone)
        {
            ncvStat = memSegCopyHelper(dst._ptr, dst._memtype, 
                                       this->_ptr, this->_memtype, 
                                       howMuch, cuStream);
        }

        return ncvStat;
    }

    T *ptr() const {return this->_ptr;}
    Ncv32u width() const {return this->_width;}
    Ncv32u height() const {return this->_height;}
    Ncv32u pitch() const {return this->_pitch;}
    NCVMemoryType memType() const {return this->_memtype;}

protected:

    T *_ptr;
    Ncv32u _width;
    Ncv32u _height;
    Ncv32u _pitch;
    NCVMemoryType _memtype;
};


/**
* NCVMatrixAlloc
*/
template <class T>
class NCVMatrixAlloc : public NCVMatrix<T>
{
    NCVMatrixAlloc();
    NCVMatrixAlloc(const NCVMatrixAlloc &);
	NCVMatrixAlloc& operator=(const NCVMatrixAlloc &);
public:

    NCVMatrixAlloc(INCVMemAllocator &allocator, Ncv32u width, Ncv32u height, Ncv32u pitch=0)
        :
        allocator(allocator)
    {
        NCVStatus ncvStat;

        this->clear();
        this->allocatedMem.clear();

        Ncv32u widthBytes = width * sizeof(T);
        Ncv32u pitchBytes = alignUp(widthBytes, allocator.alignment());

        if (pitch != 0)
        {
            ncvAssertPrintReturn(pitch >= pitchBytes &&
                (pitch & (allocator.alignment() - 1)) == 0,
                "NCVMatrixAlloc ctor:: incorrect pitch passed", );
            pitchBytes = pitch;
        }

        Ncv32u requiredAllocSize = pitchBytes * height;

        ncvStat = allocator.alloc(this->allocatedMem, requiredAllocSize);
        ncvAssertPrintReturn(ncvStat == NCV_SUCCESS, "NCVMatrixAlloc ctor:: alloc failed", );

        this->_ptr = (T *)this->allocatedMem.begin.ptr;
        this->_width = width;
        this->_height = height;
        this->_pitch = pitchBytes;
        this->_memtype = this->allocatedMem.begin.memtype;
    }

    ~NCVMatrixAlloc()
    {
        NCVStatus ncvStat;

        ncvStat = allocator.dealloc(this->allocatedMem);
        ncvAssertPrintCheck(ncvStat == NCV_SUCCESS, "NCVMatrixAlloc dtor:: dealloc failed");

        this->clear();
    }


    NcvBool isMemAllocated() const
    {
        return (this->allocatedMem.begin.ptr != NULL) || (this->allocator.isCounting());
    }


    Ncv32u getAllocatorsAlignment() const
    {
        return allocator.alignment();
    }


    NCVMemSegment getSegment() const
    {
        return allocatedMem;
    }

private:

    INCVMemAllocator &allocator;
    NCVMemSegment allocatedMem;
};


/**
* NCVMatrixReuse
*/
template <class T>
class NCVMatrixReuse : public NCVMatrix<T>
{
    NCVMatrixReuse();
    NCVMatrixReuse(const NCVMatrixReuse &);

public:

    NCVMatrixReuse(const NCVMemSegment &memSegment, Ncv32u alignment, Ncv32u width, Ncv32u height, Ncv32u pitch=0, NcvBool bSkipPitchCheck=false)
    {
        this->bReused = false;
        this->clear();

        Ncv32u widthBytes = width * sizeof(T);
        Ncv32u pitchBytes = alignUp(widthBytes, alignment);

        if (pitch != 0)
        {
            if (!bSkipPitchCheck)
            {
                ncvAssertPrintReturn(pitch >= pitchBytes &&
                    (pitch & (alignment - 1)) == 0,
                    "NCVMatrixReuse ctor:: incorrect pitch passed", );
            }
            else
            {
                ncvAssertPrintReturn(pitch >= widthBytes, "NCVMatrixReuse ctor:: incorrect pitch passed", );
            }
            pitchBytes = pitch;
        }

        ncvAssertPrintReturn(pitchBytes * height <= memSegment.size, \
            "NCVMatrixReuse ctor:: memory binding failed due to size mismatch", );

        this->_width = width;
        this->_height = height;
        this->_pitch = pitchBytes;
        this->_ptr = (T *)memSegment.begin.ptr;
        this->_memtype = memSegment.begin.memtype;

        this->bReused = true;
    }


    NcvBool isMemReused() const
    {
        return this->bReused;
    }

private:

    NcvBool bReused;
};

#endif // _ncv_hpp_
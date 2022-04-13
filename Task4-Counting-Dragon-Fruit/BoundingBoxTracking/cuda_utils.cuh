#ifndef __CUDA_UTILITY_CUH_
#define __CUDA_UTILITY_CUH_

#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <vpi/Array.h>

#define ONE_MBYTE (1024*1024)
#define THREAD 32

#define CHECK(status)                                                   \
    do                                                                  \
    {                                                                   \
        auto ret = (status);                                            \
        if (ret != 0)                                                   \
        {                                                               \
            std::cerr << "[ERROR] Cuda failure: " << ret << std::endl;  \
            exit(0);                                                    \
        }                                                               \
    } while (0)

#define CHECK_STATUS(STMT)                                                    \
    do                                                                        \
    {                                                                         \
        VPIStatus status = (STMT);                                            \
        if (status != VPI_SUCCESS)                                            \
        {                                                                     \
            std::cerr << "[ERROR] " << vpiStatusGetName(status) << std::endl; \
            exit(0);                                                          \
        }                                                                     \
    } while (0);


class myClock
{
public:
    inline void tic() { clock_gettime(CLOCK_REALTIME, &t1); };
    inline void total_tic() { clock_gettime(CLOCK_REALTIME, &total_t1); };

    void toc(std::string prefix)
    {
        clock_gettime(CLOCK_REALTIME, &t2);
        double t_us = ((double)(t2.tv_sec - t1.tv_sec)) * 1000000.0 + ((double)(t2.tv_nsec - t1.tv_nsec) / 1000.0);
        std::cout << prefix << t_us;
    }

    void total_toc(std::string prefix)
    {
        clock_gettime(CLOCK_REALTIME, &total_t2);
        double t_us = ((double)(total_t2.tv_sec - total_t1.tv_sec)) * 1000000.0 + ((double)(total_t2.tv_nsec - total_t1.tv_nsec) / 1000.0);
        std::cout << prefix << t_us;
    }

private:
    timespec t1, t2;
    timespec total_t1, total_t2;
};


static void printMemInfo()
{
    size_t free_byte ;
    size_t total_byte ;
    cudaError_t cuda_status = cudaMemGetInfo( &free_byte, &total_byte ) ;

    if ( cudaSuccess != cuda_status ){
        printf("Error: cudaMemGetInfo fails, %s\n", cudaGetErrorString(cuda_status));
        exit(1);
    }

    double free_db = (double)free_byte ;
    double total_db = (double)total_byte ;
    double used_db = total_db - free_db ;

    printf(" GPU memory usage: used = %.2f MB, free = %.2f MB, total = %.2f MB\n", used_db/ONE_MBYTE, free_db/ONE_MBYTE, total_db/ONE_MBYTE);
}


static void MatrixMultiply(VPIPerspectiveTransform &r, const VPIPerspectiveTransform &a,
                           const VPIPerspectiveTransform &b)
{
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            r[i][j] = a[i][0] * b[0][j];
            for (int k = 1; k < 3; ++k)
            {
                r[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

int cuda_feature2bbox(cudaStream_t &stream, void *kpts, void *input_box, void *input_pred, uint32_t size);

#endif


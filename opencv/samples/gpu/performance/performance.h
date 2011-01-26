#ifndef OPENCV_GPU_SAMPLE_PERFORMANCE_H_
#define OPENCV_GPU_SAMPLE_PERFORMANCE_H_

#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <opencv2/core/core.hpp>

#define TAB "    "

class Runnable
{
public:
    explicit Runnable(const std::string& name): name_(name) {}

    const std::string& name() const { return name_; }

    virtual void run() = 0;

private:
    std::string name_;
};


class TestSystem
{
public:
    static TestSystem* instance()
    {
        static TestSystem me;
        return &me;
    }

    void addInit(Runnable* init) { inits_.push_back(init); }

    void addTest(Runnable* test) { tests_.push_back(test); }

    void run();

    // Ends current subtest and starts new one
    std::stringstream& subtest()
    {
        flushSubtestData();
        return description_;
    }

    void cpuOn() { cpu_started_ = cv::getTickCount(); }

    void cpuOff() 
    {
        int64 delta = cv::getTickCount() - cpu_started_;
        cpu_elapsed_ += delta;
        can_flush_ = true;
    }  

    void gpuOn() { gpu_started_ = cv::getTickCount(); }

    void gpuOff() 
    {
        int64 delta = cv::getTickCount() - gpu_started_;
        gpu_elapsed_ += delta;
        can_flush_ = true;
    }

    void setWorkingDir(const std::string& val);

    const std::string& workingDir() const { return working_dir_; }

    void printError(const std::string& msg);

private:
    TestSystem(): can_flush_(false), cpu_elapsed_(0), gpu_elapsed_(0), 
                  speedup_total_(0.0), num_subtests_called_(0) {};

    void flushSubtestData();

    void resetSubtestData() 
    {
        cpu_elapsed_ = 0;
        gpu_elapsed_ = 0;
        description_.str("");
        can_flush_ = false;
    }

    void printHeading();
    void printSummary();
    void printItem(double cpu_time, double gpu_time, double speedup);

    std::string working_dir_;

    std::vector<Runnable*> inits_;
    std::vector<Runnable*> tests_;

    // Current test (subtest) description
    std::stringstream description_;

    bool can_flush_;

    int64 cpu_started_, cpu_elapsed_;
    int64 gpu_started_, gpu_elapsed_;

    double speedup_total_;
    int num_subtests_called_;
};


#define INIT(name) \
    struct name##_init: Runnable \
    { \
        name##_init(): Runnable(#name) { \
            TestSystem::instance()->addInit(this); \
        } \
        void run(); \
    } name##_init_instance; \
    void name##_init::run()


#define TEST(name) \
    struct name##_test: Runnable \
    { \
        name##_test(): Runnable(#name) { \
            TestSystem::instance()->addTest(this); \
        } \
        void run(); \
    } name##_test_instance; \
    void name##_test::run()

#define SUBTEST TestSystem::instance()->subtest()
#define DESCRIPTION TestSystem::instance()->subtest()
#define CPU_ON TestSystem::instance()->cpuOn()
#define GPU_ON TestSystem::instance()->gpuOn()
#define CPU_OFF TestSystem::instance()->cpuOff()
#define GPU_OFF TestSystem::instance()->gpuOff()

void gen(cv::Mat& mat, int rows, int cols, int type, cv::Scalar low, 
         cv::Scalar high);

std::string abspath(const std::string& relpath);

#endif // OPENCV_GPU_SAMPLE_PERFORMANCE_H_
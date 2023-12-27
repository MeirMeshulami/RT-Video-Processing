#pragma once
#include <filesystem>
#include <future>
#include <fstream>
#include <iostream>
#include <ctime>
#include "FrameReceiver.h"
#include "FrameWithDetails.h"
#include "ThreadSafeQueue.h"
#include "YoloDetection.h"
#include "DBManager.h"
#include "JsonManager.h"


class FrameProcessor {
public:
    ThreadSafeQueue<FrameWithDetails> mFrameProcessQueue;
    cv::VideoWriter mVideoWriter;
    DBManager mDBManager;
    std::mutex mDBMutex;
    std::vector<std::thread> frameProcessingThreads;
    std::thread frameReceiverThread;
    std::thread monitorKeyboardInputThread;
    std::atomic<bool> isServerRunning;
    FrameReceiver service;
    std::unique_ptr<grpc::Server> server;
    std::filesystem::path videoPath;
    std::filesystem::path dbPath;
    std::filesystem::path imagePath;
    std::filesystem::path imagesDirectory;
    nlohmann::json configJson;

    void InitializeVideoWriter();

    void InitializeTable();

    void CreateImageDirectory();

    void StartFrameProcessingThread(int threadIndex);

    void WaitForFrameProcessingThreadsToFinish();

    void ReleaseVideoWriter();

    void ProcessFrame(YoloDetection& yoloDetection, int threadIndex);

    void DetectFrame(FrameWithDetails& frame, YoloDetection& yoloDetection);

    cv::Scalar CalculateAverageRGB(const cv::Rect& box, const cv::Mat& frame);

    void SaveFrameAsImage(const cv::Mat& frame, const int frameNum);

    void StartFrameReceiverThread();

    void RunFrameReceiver();

public:
    FrameProcessor();

    void ProcessFrames();

    void StopRunServer();

    FrameReceiver& GetService();


};

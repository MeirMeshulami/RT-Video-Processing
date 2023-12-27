#pragma once
#include <grpcpp/grpcpp.h>
#include <service.grpc.pb.h>
#include "LogManager.h"
#include "FrameWithDetails.h"
#include "ThreadSafeQueue.h"

class FrameReceiver final : public FrameService::Service {
private:
    ThreadSafeQueue<FrameWithDetails>& frameProcessQueue;
    ThreadSafeQueue<FrameWithDetails> frameShowQueue;
    std::atomic<bool> isShowingFrames;
    std::atomic<bool> isProcessingFrames;

public:
    FrameReceiver(ThreadSafeQueue<FrameWithDetails>& _frameProcessQueue);

    grpc::Status SendFrame(grpc::ServerContext* context, const FrameMessage* request, google::protobuf::Empty* response);

    void PushFrameIntoProcessQueue(const cv::Mat frame, const int frameNum, const std::chrono::system_clock::time_point frameTime);
   
    void PushFrameIntoShowQueue(const cv::Mat frame, const int frameNum, const std::chrono::system_clock::time_point frameTime);

    ThreadSafeQueue<FrameWithDetails>& GetFrameShowQueue();

    void ControlShowingFrames();

    void ControlProcessingFrames();
};

#define _CRT_SECURE_NO_WARNINGS
#include <ctime>
#include "FrameReceiver.h"

FrameReceiver::FrameReceiver(ThreadSafeQueue<FrameWithDetails>& _frameProcessQueue)
    :frameProcessQueue(_frameProcessQueue), isShowingFrames(true), isProcessingFrames(true) {}

grpc::Status FrameReceiver::SendFrame(grpc::ServerContext* context, const FrameMessage* request, google::protobuf::Empty* response) {
    LOG_DEBUG("Received data from camera.");

    const auto& imageData = request->image_data();
    std::vector<uchar> buffer(imageData.begin(), imageData.end());
    auto receivedFrame = cv::imdecode(buffer, cv::IMREAD_COLOR);

    if (receivedFrame.empty()) {
        LOG_INFO("receivedFrame empty");

        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Error: Failed to decode the received image.");
    }
    else {
        auto frameTimePoint = std::chrono::system_clock::time_point(std::chrono::seconds(request->frame_time().seconds()) + std::chrono::milliseconds(request->frame_time().nanos() / 1000000));

        auto timeT = std::chrono::system_clock::to_time_t(frameTimePoint);
        std::tm* timeInfo = std::localtime(&timeT);
        if (timeInfo == nullptr)
            return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Error converting time.");
        else
        {
            LOG_INFO("Received frame {} with timestamp {}", request->frame_number(), frameTimePoint);

            PushFrameIntoShowQueue(receivedFrame, request->frame_number(), frameTimePoint);
            LOG_INFO("Frame {} pushed into frameShowQueue.", request->frame_number());

            PushFrameIntoProcessQueue(receivedFrame, request->frame_number(), frameTimePoint);
            LOG_INFO("Frame {} pushed into frameProcessQueue.", request->frame_number());
        }
    }
    return grpc::Status(grpc::StatusCode::OK, "Success.");
}

void FrameReceiver::PushFrameIntoProcessQueue(const cv::Mat frame, const int frameNum, const std::chrono::system_clock::time_point frameTime) {
    if (isProcessingFrames)
    {
        if (frameProcessQueue.GetQueueSize() > 4)
        {
            FrameWithDetails toThrowOut;
            frameProcessQueue.TryPop(toThrowOut);
            LOG_ERROR("The frameProcessQueue reached its maximum and popped out frame {}.", toThrowOut.GetFrameNum());
        }
        frameProcessQueue.Push(FrameWithDetails(frame, frameNum, frameTime));
    }
}

void FrameReceiver::PushFrameIntoShowQueue(const cv::Mat frame, const int frameNum, const std::chrono::system_clock::time_point frameTime) {
    if (isShowingFrames)
    {
        if (frameShowQueue.GetQueueSize() > 30)
        {
            FrameWithDetails toThrowOut;
            frameShowQueue.TryPop(toThrowOut);
            LOG_ERROR("The frameShowQueue reached its maximum and popped out frame {}.", toThrowOut.GetFrameNum());
        }
        frameShowQueue.Push(FrameWithDetails(frame, frameNum, frameTime));
    }
}

ThreadSafeQueue<FrameWithDetails>& FrameReceiver::GetFrameShowQueue() {
    return frameShowQueue;
}

void FrameReceiver::ControlShowingFrames() {
    isShowingFrames = !isShowingFrames;
    if(isShowingFrames)
        LOG_INFO("Showing video started.");
    else
        LOG_INFO("Showing video stoped.");
}

void FrameReceiver::ControlProcessingFrames() {
    isProcessingFrames = !isProcessingFrames;
    if (isProcessingFrames)
        LOG_INFO("Processing frames started.");
    else
        LOG_INFO("Processing frames stoped.");
}


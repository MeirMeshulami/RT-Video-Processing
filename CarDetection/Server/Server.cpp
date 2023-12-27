#include <iostream>
#include <thread>
#include <atomic>
#include <opencv2/core/utils/logger.hpp>
#include "FrameProcessor.h"
#include "LogManager.h"
#include "ServerAPI.h"
#include "FrameWithDetails.h"


int main() {
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);

	try {
        ServerAPI serverAPI;
        std::atomic<bool> exitFlag(false);

        std::thread serverThread([&serverAPI]() {
            serverAPI.StartServer();
            });

        std::thread keyboardThread([&serverAPI, &exitFlag]() {
            while (!exitFlag) {
                if (GetAsyncKeyState('S') & 0x8000) {
                    serverAPI.StartStopShowingFrames();
                    std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Sleep to avoid multiple key presses
                }
                if (GetAsyncKeyState('P') & 0x8000) {
                    serverAPI.StartStopProcessingFrames();
                    std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Sleep to avoid multiple key presses
                }
                if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
                    serverAPI.StopServer();
                    exitFlag = true;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep to avoid high CPU usage
            } 
            });

        std::thread imageDisplayThread([&serverAPI, &exitFlag]() {
            auto& frameShowQueue = serverAPI.GetFrameShowQueue();

            while (!exitFlag) {
                FrameWithDetails frame;

                if (frameShowQueue.TryPop(frame)) {
                    cv::imshow("Image", frame.GetFrame());
                    cv::waitKey(1); // Wait for a short time to display the image
                }
                else {
                    cv::Mat noVideoImage(480, 640, CV_8UC3, cv::Scalar(0, 0, 0)); // Black image
                    cv::putText(noVideoImage, "No video", cv::Point(180, 240), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(255, 255, 255), 2);
                    cv::imshow("Image", noVideoImage);

                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            });

        serverThread.join();
        keyboardThread.join();
        imageDisplayThread.join();

	}
	catch (const std::exception& e) {
		LOG_ERROR("An exception occurred: {}", e.what());
	}
	LOG_INFO("Program closed.");
	return 0;
}
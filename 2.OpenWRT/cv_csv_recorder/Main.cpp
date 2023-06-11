#include <iostream>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <chrono>
#include <unistd.h>
#include <signal.h>

#include "VideoHelper.h"

using namespace cv;
using namespace std;

/* The flag to quit the program */
bool quit = false;

/**
 * @brief Handle the Ctrl+C signal
 * @param sig
 */
void handle_sigint(int sig)
{
    if (sig == SIGINT) {
        /* 打印信号值 */
        printf("Caught signal %d\n", sig);

        /* 设置退出标志 */
        quit = true;
    }
}

int main() {

    /* Open the camera */
    VideoCapture cap = VideoHelper::openCamera();

    /* Set the video resolution to 640x480 */
    int codec = CV_FOURCC('M', 'J', 'P', 'G');
    int frameWidth = static_cast<int>(cap.get(CV_CAP_PROP_FRAME_WIDTH));
    int frameHeight = static_cast<int>(cap.get(CV_CAP_PROP_FRAME_HEIGHT));

    // 创建视频文件写入器
    VideoWriter writer = VideoHelper::openVideoWriter("output.avi", codec, 30.0, Size(frameWidth, frameHeight));

    // 打开二进制文件进行记录
    ofstream hashFile("frame_hash.bin", ios::out | ios::binary);
    if (!hashFile.is_open()) {
        cout << "Cannot open the file!" << endl;
        return -1;
    }

    /* Set the quit signal handler */
    quit = false;

    /* Register the signal handler */
    signal(SIGINT, handle_sigint);

    while (!quit) {
        Mat frame;
        cap.read(frame);

        // 写入视频文件
        writer.write(frame);

        // 获取帧的哈希值和时间戳
        string frameHash = VideoHelper::getHashValue(frame, "AverageHash");
        auto timestamp = chrono::system_clock::now();
        auto time = chrono::system_clock::to_time_t(timestamp);

        // 写入哈希值和时间戳到二进制文件
        hashFile.write(reinterpret_cast<const char*>(&time), sizeof(time));
        hashFile.write(frameHash.c_str(), frameHash.size() + 1);

        imshow("Video", frame);

        // 按下Esc键退出循环
        if (waitKey(1) == 27)
            break;
    }

    // 释放资源
    cap.release();
    writer.release();
    hashFile.close();
    destroyAllWindows();

    return 0;
}
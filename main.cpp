#define _CRT_SECURE_NO_WARNINGS 1
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <thread>
#include <chrono>
#include <sstream>
#include <exception>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace std;
namespace fs = std::filesystem;

// 手动设置的线程数，默认为0表示使用计算机的最大CPU线程数
const int THREAD_NUM = 0;

// 所支持的图片格式
const vector<string> IMAGE_FORMATS = { "jpg", "jpeg", "png", "bmp", "tiff" };

// 压缩质量，默认为80
const int QUALITY = 80;

void compressImage(const string& inputFile, const string& outputFile) {
    try {
        int width, height, bpp;
        unsigned char* data = stbi_load(inputFile.c_str(), &width, &height, &bpp, 0);
        if (!data) {
            throw runtime_error("Failed to load image: " + inputFile);
        }

        stbi_flip_vertically_on_write(1);
        int result = stbi_write_jpg(outputFile.c_str(), width, height, bpp, data, QUALITY);
        if (!result) {
            throw runtime_error("Failed to compress image: " + inputFile);
        }

        stbi_image_free(data);
    }
    catch (exception& ex) {
        cout << "Error occurred while compressing " << inputFile << ": " << ex.what() << endl;
    }
}

int main() {
    cout << "Welcome to image compressor!" << endl;

    // 获取输入和输出文件夹路径
    cout << "Please enter the input image directory path:" << endl;
    string inputDir;
    getline(cin, inputDir);

    if (!fs::exists(inputDir) || !fs::is_directory(inputDir)) {
        cout << "Invalid input directory path." << endl;
        return 1;
    }

    cout << "Please enter the output directory path:" << endl;
    string outputDir;
    getline(cin, outputDir);

    if (!fs::exists(outputDir) || !fs::is_directory(outputDir)) {
        cout << "Invalid output directory path." << endl;
        return 1;
    }

    // 获取计算机最大CPU线程数并设置线程数
    int threadNum = THREAD_NUM == 0 ? thread::hardware_concurrency() : THREAD_NUM;

    cout << "Threads used: " << threadNum << endl;

    // 获取所有主流格式的图片文件列表
    vector<string> fileList;
    for (const auto& file : fs::directory_iterator(inputDir)) {
        if (fs::is_regular_file(file) && find(IMAGE_FORMATS.begin(), IMAGE_FORMATS.end(), file.path().extension().string().substr(1)) != IMAGE_FORMATS.end()) {
            fileList.push_back(file.path().string());
        }
    }

    // 开始压缩图片
    vector<thread> threads;
    for (size_t i = 0; i < fileList.size(); ++i) {
        stringstream ss;
        ss << outputDir << "\\" << fs::path(fileList[i]).filename().string();
        threads.emplace_back(compressImage, fileList[i], ss.str());
        if (threads.size() == threadNum || i == fileList.size() - 1) {
            for (auto& thread : threads) {
                thread.join();
            }
            threads.clear();
        }
    }

    cout << "Compression complete. Press any key to exit..." << endl;
    cin.get();

    return 0;
}

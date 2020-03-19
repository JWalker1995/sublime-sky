#include "gamerecorder.h"

#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>
#include <locale>

#include "graphics/gl.h"
#include "game/gamecontext.h"

namespace application {

GameRecorder::GameRecorder(game::GameContext &context) {
    (void) context;

    loadScreenDims();
    chooseDir();
    initDir();
}

GameRecorder::~GameRecorder() {
    std::string cmd = "ffmpeg -r 60 -f image2 -s " + std::to_string(width) + "x" + std::to_string(height) + " -i " + dir + "/img%06d.bmp -vcodec libx264 -crf 15 -pix_fmt yuv420p " + dir + "/video.mp4";
    std::cout << "Wrote " << nextFileIndex << " bitmap files to " << dir << std::endl;
    std::cout << "Run this cmd to convert to mp4:" << std::endl;
    std::cout << cmd << std::endl;
}

void GameRecorder::loadScreenDims() {
    int screenStats[4];
    glGetIntegerv(GL_VIEWPORT, screenStats);
    width = static_cast<unsigned int>(screenStats[2]);
    height = static_cast<unsigned int>(screenStats[3]);
}

void GameRecorder::chooseDir() {
    char buf[64];
    time_t now = std::time(nullptr);
    std::size_t len = strftime(buf, sizeof(buf), "recording_%Y_%m_%d_%H_%M_%S", std::localtime(&now));
    if (!len) {
        std::cerr << "strftime failed" << std::endl;
        enabled = false;
        return;
    }
    dir = std::string(buf, len);

    nextFileIndex = 0;
}

void GameRecorder::initDir() {
    int res = mkdir(dir.data(), 0755);
    if (res) {
        std::cerr << "mkdir returned " << res << std::endl;
        enabled = false;
        return;
    }
}

void GameRecorder::takeSnapshot() {
    if (!enabled) {
        return;
    }

    unsigned int dataSize = width * height * 3;
    char *pixels = new char[dataSize];
    glReadPixels(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height), GL_BGR, GL_UNSIGNED_BYTE, pixels);
    graphics::GL::catchErrors();

    unsigned int fileSize = 54 + dataSize;

    char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
    char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
    char bmppad[3] = {0,0,0};

    bmpfileheader[ 2] = static_cast<char>(fileSize >>  0);
    bmpfileheader[ 3] = static_cast<char>(fileSize >>  8);
    bmpfileheader[ 4] = static_cast<char>(fileSize >> 16);
    bmpfileheader[ 5] = static_cast<char>(fileSize >> 24);

    bmpinfoheader[ 4] = static_cast<char>(width >>  0);
    bmpinfoheader[ 5] = static_cast<char>(width >>  8);
    bmpinfoheader[ 6] = static_cast<char>(width >> 16);
    bmpinfoheader[ 7] = static_cast<char>(width >> 24);
    bmpinfoheader[ 8] = static_cast<char>(height >>  0);
    bmpinfoheader[ 9] = static_cast<char>(height >>  8);
    bmpinfoheader[10] = static_cast<char>(height >> 16);
    bmpinfoheader[11] = static_cast<char>(height >> 24);

    std::string fileIndexStr = std::to_string(nextFileIndex++);
    std::string fileName = std::string(6 - fileIndexStr.size(), '0') + fileIndexStr + ".bmp";
    std::ofstream ofs(dir + "/img" + fileName, std::ofstream::binary);
    ofs.write(bmpfileheader, 14);
    ofs.write(bmpinfoheader, 40);
    for (unsigned int i = 0; i < height; i++) {
        ofs.write(pixels + i * width * 3, width * 3);
        ofs.write(bmppad, (4 - (width * 3) % 4) % 4);
    }
    ofs.close();

    delete[] pixels;
}

}

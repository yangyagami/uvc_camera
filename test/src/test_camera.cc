#include <unistd.h>

#include <iostream>

#include "camera.h"
#include "opencv2/opencv.hpp"

int main() {
  uvc::Camera camera(0x1871, 0x3b49);
  if (camera.Init(640, 480, 30) == false) {
    return 1;
  }
  if (camera.Open() == false) {
    return 1;
  }
  int count = 0;
  while (true) {
    cv::Mat frame;
    bool ret = camera.Read(frame);
    if (ret && !frame.empty()) {
      cv::imshow("window", frame);
      cv::waitKey(1);
    }
    if (camera.Opened() == false) {
      camera.Open();
    }
    count++;
    if (count >= 60000000) {
      break; 
    }
  }
  return 0;
}

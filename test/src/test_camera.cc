#include <unistd.h>

#include "camera.h"
#include "opencv2/opencv.hpp"

int main() {
  int i = 0;
  uvc::Camera camera(0x1187, 0x3a49);
  camera.Init(640, 480, 30);
  camera.Open();
  while (true) {
    if (camera.IsConnected() == false) {
      camera.ReOpenWhenDisconnected(640, 480, 30);
    }
    sleep(1);
    i++;
    if (i >= 30) {
      break;
    }
  }
  return 0;
}

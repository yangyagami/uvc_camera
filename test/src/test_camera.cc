#include <unistd.h>

#include "camera.h"
#include "opencv2/opencv.hpp"

int main() {
  uvc::Camera camera_rgb("1187", "3a49");
  //uvc::Camera camera_nir("1871", "3b49");
  if (camera_rgb.Init(640, 480, 30) /*&& camera_nir.Init(640, 480, 30)*/) {
    if (camera_rgb.Open()/* && camera_nir.Open()*/) {
      [[maybe_unused]]
      int rgb_no_frame_times = 0;
      [[maybe_unused]]
      int nir_no_frame_times = 0;
      while (true) {
        cv::Mat frame_rgb;
        cv::Mat frame_nir;
        bool ret_rgb = camera_rgb.Read(frame_rgb);
        //bool ret_nir = camera_nir.Read(frame_nir);
      [[maybe_unused]]
        bool rgb_frame_flag = false;
      [[maybe_unused]]
        bool nir_frame_flag = false;
        if (ret_rgb) {
          rgb_frame_flag = true;
          rgb_no_frame_times = 0;
          cv::imshow("rgb", frame_rgb);
          if (cv::waitKey(10) == 27) {
            break; 
          }
        }
        //if (ret_nir) {
        //  nir_frame_flag = true;
        //  nir_no_frame_times = 0;
        //  cv::imshow("nir", frame_nir);
        //  if (cv::waitKey(10) == 27) {
        //    break; 
        //  }
        //}
        if (camera_rgb.IsConnected() == false) {
          camera_rgb.Close();
          if (camera_rgb.ReInitWhenDisconnected(640, 480, 30)) {
            camera_rgb.Open();
          }
          sleep(1);
        }
        //if (camera_nir.IsConnected() == false) {
        //  std::cout << "nir not connected" << std::endl; 
        //}
      } 
    }
  }
  cv::destroyWindow("rgb");
  //cv::destroyWindow("nir");
  return 0;
}

// Author: yangsiyu
// 采用libuvc作为后端实现的camera类。
#ifndef CAMERA_CAMERA_H_
#define CAMERA_CAMERA_H_

#include <queue>
#include <mutex>
#include <thread>
#include <memory>
#include <condition_variable>

#include "libuvc/libuvc.h"
#include "opencv2/opencv.hpp"

namespace uvc {

class Camera {
 public:
  Camera(int vid, int pid)
      : ctx_(nullptr),
        dev_(nullptr),
        devh_(nullptr),
        vid_(vid),
        pid_(pid) {
  }
  ~Camera();

  bool Init(int width, int height, int fps);
  bool Open();
  void Close();
  bool Read(cv::Mat &frame);

  bool Opened() {
    return uvc_is_opened_with_vid_pid(ctx_, vid_, pid_);
  }
 private:
  static void FrameCallback(uvc_frame_t *frame, void *ptr);

  void EnableAutoExposure();

  void FreeResources();
 private:
  const int kThreadWaitSeconds = 5;

  std::queue<cv::Mat> frame_queue_;

  std::mutex frame_lock_;

  uvc_context_t *ctx_;
  uvc_device_t *dev_;
  uvc_device_handle_t *devh_;
  uvc_stream_ctrl_t ctrl_;
  uvc_error_t res_;  // 错误处理

  int vid_;
  int pid_;

  int frame_width_;
  int frame_height_;
  int fps_;
};

}  // namespace uvc

#endif  // CAMERA_CAMERA_H_

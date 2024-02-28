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
        pid_(pid),
        opened_(false),
        connected_(false),
        camera_connect_status_check_thread_running_(false),
        frame_flag_(false),
        camera_connect_status_check_thread_(nullptr) {
  }
  ~Camera();

  bool Init(int width, int height, int fps);
  bool Open();
  void Close();
  bool Read(cv::Mat &frame);
  bool ReOpenWhenDisconnected(int width, int height, int fps);
  bool IsConnected();

  bool opened() {
    std::lock_guard<std::mutex> lock(opened_lock_);
    return opened_;
  }
 private:
  static void FrameCallback(uvc_frame_t *frame, void *ptr);

  void EnableAutoExposure();
  void CameraConnectStatusCheck();

  void FreeResources();
 private:
  const int kThreadWaitSeconds = 5;

  std::queue<cv::Mat> frame_queue_;

  std::mutex frame_lock_;
  std::mutex connected_lock_;
  std::mutex opened_lock_;
  std::mutex camera_connect_status_check_thread_running_lock_;

  uvc_context_t *ctx_;
  uvc_device_t *dev_;
  uvc_device_handle_t *devh_;
  uvc_stream_ctrl_t ctrl_;
  uvc_error_t res_;  // 错误处理

  int vid_;
  int pid_;

  bool opened_;
  bool connected_;
  bool camera_connect_status_check_thread_running_;
  bool frame_flag_;

  std::shared_ptr<std::thread> camera_connect_status_check_thread_;
  std::condition_variable camera_connect_status_check_cv_;
};

}  // namespace uvc

#endif  // CAMERA_CAMERA_H_

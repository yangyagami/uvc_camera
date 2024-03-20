#include "camera.h"

#include <iostream>
#include <string>
#include <queue>
#include <mutex>
#include <thread>
#include <unistd.h>

#include <unistd.h>

#include "libuvc/libuvc.h"
#include "opencv2/opencv.hpp"

#define CAMERA_CAPTURE_ERROR_HANDLE(x) { \
  if (res_ < 0) { \
    uvc_perror(res_, x); \
    return false; \
  } \
}

namespace uvc {

Camera::~Camera() {
  Close();

  FreeResources();
}

bool Camera::Init(int width, int height, int fps) {
  frame_width_ = width;
  frame_height_ = height;
  fps_ = fps;

  res_ = uvc_init(&ctx_, nullptr);
  CAMERA_CAPTURE_ERROR_HANDLE("uvc_init");

  std::cout << "UVC initialized" << std::endl;

  return true;
}

bool Camera::Open() {
  // 说明相机硬件中途断开,再次打开需要释放资源。
  if (dev_ != nullptr) {
    uvc_unref_device(dev_);
    dev_ = nullptr;
  }

  res_ = uvc_find_device(ctx_, &dev_, vid_, pid_, nullptr);
  CAMERA_CAPTURE_ERROR_HANDLE("uvc_find_device");

  std::cout << "Device found" << std::endl;

  res_ = uvc_open(dev_, &devh_);
  CAMERA_CAPTURE_ERROR_HANDLE("uvc_open");

  std::cout << "Device opened" << std::endl;

  uvc_print_diag(devh_, nullptr);

  const uvc_format_desc_t *format_desc = uvc_get_format_descs(devh_);
  [[maybe_unused]]
  const uvc_frame_desc_t *frame_desc = format_desc->frame_descs;
  enum uvc_frame_format frame_format;

  switch (format_desc->bDescriptorSubtype) {
    case UVC_VS_FORMAT_MJPEG:
      frame_format = UVC_COLOR_FORMAT_MJPEG;
      break;
    case UVC_VS_FORMAT_FRAME_BASED:
      frame_format = UVC_FRAME_FORMAT_H264;
      break;
    default:
      frame_format = UVC_FRAME_FORMAT_YUYV;
      break;
  }
  res_ = uvc_get_stream_ctrl_format_size(
      devh_,
      &ctrl_,
      frame_format,
      frame_width_,
      frame_height_,
      fps_);
  uvc_print_stream_ctrl(&ctrl_, nullptr);
  CAMERA_CAPTURE_ERROR_HANDLE("get_mode");

  res_ = uvc_start_streaming(devh_, &ctrl_, &Camera::FrameCallback,
                             (void *)this, 0);
  CAMERA_CAPTURE_ERROR_HANDLE("start_streaming");

  std::cout << "Streaming" << std::endl;

  std::cout << "Enabling auto exposure" << std::endl;
  EnableAutoExposure();

  return true;
}

void Camera::Close() {
  if (Opened() && devh_ != nullptr) {
    uvc_stop_streaming(devh_);
    std::cout << "Stream Closed" << std::endl;
    if (devh_ != nullptr) {
      uvc_close(devh_);
      devh_ = nullptr;
    }
  }

  if (dev_ != nullptr) {
    uvc_unref_device(dev_);
    dev_ = nullptr;
  }
}

bool Camera::Read(cv::Mat &frame) {
  std::lock_guard<std::mutex> lock(frame_lock_);
  if (frame_queue_.empty()) {
    return false;
  }
  frame = frame_queue_.front();
  frame_queue_.pop();
  return true;
}

void Camera::EnableAutoExposure() {
  const uint8_t UVC_AUTO_EXPOSURE_MODE_AUTO = 2;
  res_ = uvc_set_ae_mode(devh_, UVC_AUTO_EXPOSURE_MODE_AUTO);
  if (res_ == UVC_SUCCESS) {
    std::cout << " ... enabled auto exposure" << std::endl;
  } else if (res_ == UVC_ERROR_PIPE) {
    /* this error indicates that the camera does not support the full AE mode;
     * try again, using aperture priority mode 
     * (fixed aperture, variable exposure time) */
    std::cout << " ... full AE not supported, trying aperture priority mode"
              << std::endl;
    const uint8_t UVC_AUTO_EXPOSURE_MODE_APERTURE_PRIORITY = 8;
    res_ = uvc_set_ae_mode(devh_, UVC_AUTO_EXPOSURE_MODE_APERTURE_PRIORITY);
    if (res_ < 0) {
      uvc_perror(
          res_,
          " ... uvc_set_ae_mode failed to enable aperture priority mode");
    } else {
      std::cout << " ... enabled aperture priority auto exposure mode"
                << std::endl;
    }
  } else {
    uvc_perror(res_, "... uvc_set_ae_mode failed to enable auto exposure mode");
  }
}

// **暂时只处理jpeg格式**。
void Camera::FrameCallback(uvc_frame_t *frame, void *ptr) {
  auto self = reinterpret_cast<Camera*>(ptr);
  cv::Mat f = cv::imdecode(cv::Mat(1, frame->data_bytes, CV_8UC1, frame->data),
                           cv::IMREAD_COLOR);

  std::lock_guard<std::mutex> lock(self->frame_lock_);
  if (self->frame_queue_.size() >= 4) {
    self->frame_queue_.pop();
  }
  self->frame_queue_.push(f);
}

void Camera::FreeResources() {
  if (ctx_ != nullptr) {
    uvc_exit(ctx_);
    ctx_ = nullptr;
  }
}

}  // namespace uvc

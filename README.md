# HOW TO USE
```cpp
#include "camera.h"

int main() {
    uvc::Camera camera(vid, pid);
    if (camera.Init(640, 480, 30)) {  // width height fps
        if (camera.Open()) {
            cv::Mat frame;
            bool ret = camera.Read(frame);
            if (ret) {
                // TODO
            }
        } 
    }

   return 0;
}
```

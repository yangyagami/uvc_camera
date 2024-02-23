{
    values = {
        "/usr/bin/g++",
        {
            "-m64",
            "-L3rdparty/libuvc/libs/amd64",
            "-Lbuild/linux/x86_64/debug",
            "-Wl,-rpath=$ORIGIN",
            "-lcamera",
            "-luvc",
            "-lopencv_core",
            "-lopencv_highgui"
        }
    },
    files = {
        "build/.objs/test_camera/linux/x86_64/debug/test/src/test_camera.cc.o"
    }
}
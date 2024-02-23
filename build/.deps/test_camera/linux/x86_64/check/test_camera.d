{
    values = {
        "/usr/bin/g++",
        {
            "-m64",
            "-L3rdparty/libuvc/libs/amd64",
            "-Lbuild/linux/x86_64/check",
            "-Wl,-rpath=$ORIGIN",
            "-lcamera",
            "-luvc",
            "-lopencv_core",
            "-lopencv_highgui",
            "-fsanitize=address"
        }
    },
    files = {
        "build/.objs/test_camera/linux/x86_64/check/test/src/test_camera.cc.o"
    }
}
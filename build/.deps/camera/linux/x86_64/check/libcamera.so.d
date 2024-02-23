{
    files = {
        "build/.objs/camera/linux/x86_64/check/src/camera.cc.o"
    },
    values = {
        "/usr/bin/g++",
        {
            "-shared",
            "-m64",
            "-fPIC",
            "-L3rdparty/libuvc/libs/amd64",
            "-luvc",
            "-lopencv_core",
            "-lopencv_highgui",
            "-fsanitize=address"
        }
    }
}
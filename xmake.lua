add_rules("mode.debug", "mode.release", "mode.check")

if is_mode("debug") then
    add_defines("_DEBUG")
end

add_cxflags("-Werror", "-Wall")

target("camera")
    set_kind("shared")

    add_files("src/*.cc")
    add_includedirs("3rdparty/libuvc/include", "src", "/usr/include/opencv4", { public = true })

    if is_arch("x86_64") then
        add_linkdirs("3rdparty/libuvc/libs/amd64", { public = true })
    end
    add_links("uvc", "opencv_core", "opencv_highgui", { public = true })

target("test_camera")
    set_kind("binary")

    add_deps("camera")

    add_files("test/src/*.cc")

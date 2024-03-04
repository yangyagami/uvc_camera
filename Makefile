INCLUDE_DIRS := -I3rdparty/libuvc/include -Isrc
LIB_DIRS := -L3rdparty/libuvc/libs
CXFLAGS := -Werror -Wall -std=c++11 -fPIC
RELEASE_FLAGS := -O3
SOURCES := $(wildcard src/*.cc)
INCLUDES := $(wildcard src/*.h)

amd64: $(SOURCES) $(INCLUDES)
	echo "Start building"
	g++ -c $(SOURCES) $(INCLUDE_DIRS) -I/usr/include/opencv4 $(CXFLAGS) $(RELEASE_FLAGS)
	ar -x 3rdparty/libuvc/libs/amd64/libuvc.a
	ar -cr libcamera.a *.o

arm64: $(SOURCES) $(INCLUDES)
	echo "Start building"
	aarch64-linux-gnu-g++-9 -c $(SOURCES) $(INCLUDE_DIRS) -I../3rdparty/Baidu_Face_Offline_SDK_Linux_ARM_7.3/third_party $(CXFLAGS) $(RELEASE_FLAGS)
	aarch64-linux-gnu-ar -x 3rdparty/libuvc/libs/arm64/libuvc.a
	aarch64-linux-gnu-ar -cr libcamera.a *.o

test: camera
	echo "Start testing"

clean:
	rm *.o

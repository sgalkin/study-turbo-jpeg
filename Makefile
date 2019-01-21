CFLAGS += \
	-Wall -Wextra \
	-O3 -DNDEBUG \

# Non-portable
X86_CFLAGS += \
	-march=native -mfpmath=sse,387 -m64 -msse4.2 -msse4 -msse3 -msse2 -msse -mavx

H264_CFLAGS += \
	-I /opt/openh264/include \

H264_LDFLAGS += \
	-L /opt/openh264/lib \
	-Wl,-rpath=/opt/openh264/lib \
	-lopenh264

JPEG_CFLAGS += \
	-I/opt/libjpeg-turbo/include

JPEG_LDFLAGS += \
	-L/opt/libjpeg-turbo/lib64 \
	-Wl,-rpath=/opt/libjpeg-turbo/lib64 \
	-lturbojpeg


H264_SRCS = \
	h264d.cpp

JPEG_SRCS = \
	jpg.c


h264d: ${H264_SRCS:.cpp=.o}
	${LINK.cc} -o $@ $^ ${LDGLAGS} ${H264_LDFLAGS}

${H264_SRCS:.cpp=.o}: ${H264_SRCS:.cpp=.cpp}
	${CXX} -c -o $@ ${CFLAGS} ${X86_CFLAGS} ${H264_CFLAGS} $^


jpg: ${JPEG_SRCS:.c=.o}
	${LINK.cc} -o $@ $^ ${LDGLAGS} ${JPEG_LDFLAGS}

${JPEG_SRCS:.c=.o}: ${JPEG_SRCS:.c=.c}
	${CC} -c -o $@ ${CFLAGS} ${X86_CFLAGS} ${JPEG_CFLAGS} $^

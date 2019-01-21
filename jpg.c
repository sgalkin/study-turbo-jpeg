#include <turbojpeg.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

void read(const char* img, unsigned char **jpegBuf, unsigned long* jpegSize) {
    long size;
    FILE* jpegFile;

    /* Read the JPEG file into memory. */
    if ((jpegFile = fopen(img, "rb")) == NULL) abort();
    if (fseek(jpegFile, 0, SEEK_END) < 0 ||
        ((size = ftell(jpegFile)) < 0) || fseek(jpegFile, 0, SEEK_SET) < 0) abort();
    if (size == 0) abort();

    *jpegSize = (unsigned long)size;
    if ((*jpegBuf = (unsigned char *)tjAlloc(*jpegSize)) == NULL) abort();
    if (fread(*jpegBuf, *jpegSize, 1, jpegFile) < 1) abort();
    fclose(jpegFile);  jpegFile = NULL;
}

void decode(tjhandle tjInstance, unsigned char* jpegBuf, unsigned long jpegSize, unsigned char** imgBuf) {
    int inSubsamp, inColorspace;
    int width, height;
    int pixelFormat;
    int flags = TJFLAG_NOREALLOC | TJFLAG_FASTUPSAMPLE | TJFLAG_FASTDCT; /* / TJFLAG_ACCURATEDCT */

    if (tjDecompressHeader3(tjInstance, jpegBuf, jpegSize, &width, &height,
                            &inSubsamp, &inColorspace) < 0) abort();
    /*
    printf("Image:  %d x %d pixels, %d subsampling, %d colorspace\n",
           width, height, inSubsamp, inColorspace);
    */
    pixelFormat = TJPF_RGBX;  // ~3ms faster than TJPF_RGB

    if ((*imgBuf = (unsigned char *)tjAlloc(width * height *
                                           tjPixelSize[pixelFormat])) == NULL) abort();

    if (tjDecompress2(tjInstance, jpegBuf, jpegSize, *imgBuf, width, 0, height,
                      pixelFormat, flags) < 0) abort();
}

int main(int argc, char** argv) {
    if(argc != 2) {
        printf("Usage: %s file.jpg\n", argv[0]);
        return 1;
    }

    static const unsigned long long sec = 1000000000;

    unsigned long long avg = 0;
    const size_t samples = 100;
	for(size_t i = 0; i < samples; ++i) {
        unsigned char *jpegBuf = NULL, *imgBuf = NULL;
        unsigned long jpegSize = 0;
        read("test.jpg", &jpegBuf, &jpegSize);

        tjhandle tjInstance = NULL;
        if ((tjInstance = tjInitDecompress()) == NULL) abort();

        struct timespec start, end;
        if(clock_gettime(CLOCK_REALTIME, &start) != 0) abort();
        decode(tjInstance, jpegBuf, jpegSize, &imgBuf);
        if(clock_gettime(CLOCK_REALTIME, &end) != 0) abort();

        tjFree(jpegBuf);  jpegBuf = NULL;
        tjFree(imgBuf); imgBuf = NULL;
        tjDestroy(tjInstance);  tjInstance = NULL;

        unsigned long long nstart = sec*start.tv_sec + start.tv_nsec;
        unsigned long long nend = sec*end.tv_sec + end.tv_nsec;
        avg += (nend - nstart);
    }

	printf("Avg time (%ld sameples): %.09f s\n", samples, (avg/(float)(sec))/samples);
	return 0;
}

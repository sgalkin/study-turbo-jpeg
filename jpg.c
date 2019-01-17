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

void decode(tjhandle tjInstance, unsigned char* jpegBuf, unsigned long jpegSize) {
    int inSubsamp, inColorspace;
    unsigned char *imgBuf = NULL;
    int width, height;
    int pixelFormat;
    int flags = TJFLAG_FASTUPSAMPLE | TJFLAG_FASTDCT; /* / TJFLAG_ACCURATEDCT */

    if (tjDecompressHeader3(tjInstance, jpegBuf, jpegSize, &width, &height,
                            &inSubsamp, &inColorspace) < 0) abort();

    printf("Image:  %d x %d pixels, %d subsampling, %d colorspace\n",
           width, height, inSubsamp, inColorspace);

    pixelFormat = TJPF_RGB;

    if ((imgBuf = (unsigned char *)tjAlloc(width * height *
                                           tjPixelSize[pixelFormat])) == NULL) abort();

    if (tjDecompress2(tjInstance, jpegBuf, jpegSize, imgBuf, width, 0, height,
                      pixelFormat, flags) < 0) abort();
}

int main() {
	for(int i = 0; i < 5; ++i) {
	unsigned char *jpegBuf = NULL;
	unsigned long jpegSize = 0;
	read("test.jpg", &jpegBuf, &jpegSize);

	tjhandle tjInstance = NULL;
	if ((tjInstance = tjInitDecompress()) == NULL) abort();
	
	struct timespec start, end;
	if(clock_gettime(CLOCK_REALTIME, &start) != 0) abort();
	decode(tjInstance, jpegBuf, jpegSize);
	if(clock_gettime(CLOCK_REALTIME, &end) != 0) abort();
        tjFree(jpegBuf);  jpegBuf = NULL;
	tjDestroy(tjInstance);  tjInstance = NULL;

	time_t dsec = end.tv_sec - start.tv_sec;
	long dnsec = end.tv_nsec - start.tv_nsec;
	if(dnsec < 0) { dnsec += 1000000000; dsec -= 1; }
	printf("Detlta %ld.%09ld\n", dsec, dnsec);
	}
	return 0;
}

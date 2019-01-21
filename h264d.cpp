#include <wels/codec_api.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <memory.h>

void read(const char* name, unsigned char **buf, unsigned long* size) {
    long length;
    FILE* file;

    if ((file = fopen(name, "rb")) == NULL) abort();
    if (fseek(file, 0, SEEK_END) < 0 ||
        ((length = ftell(file)) < 0) || fseek(file, 0, SEEK_SET) < 0) abort();
    if (length == 0) abort();

    *size = (unsigned long)length;
    if ((*buf = (unsigned char *)malloc(*size)) == NULL) abort();
    if (fread(*buf, *size, 1, file) < 1) abort();
    fclose(file);  file = NULL;
}

void decode(ISVCDecoder* decoder, unsigned char* buf, unsigned long size, unsigned char** frame)
{
    SBufferInfo sDstBufInfo;
    memset(&sDstBufInfo, 0, sizeof(sDstBufInfo));
    if(decoder->DecodeFrameNoDelay(buf, size, frame, &sDstBufInfo) != 0) {
        abort();
    }
}

int main(int argc, char** argv) {
    if(argc != 2) {
        printf("Usage: %s file.h264\n", argv[0]);
        return 1;
    }
    unsigned char* buf = NULL;
    unsigned long size = 0;
    read(argv[1], &buf, &size);


    ISVCDecoder* pSvcDecoder = NULL;
    WelsCreateDecoder(&pSvcDecoder);
    if(pSvcDecoder == NULL) abort();
    //    int traceLevel = WELS_LOG_DETAIL;
    //if(pSvcDecoder->SetOption(DECODER_OPTION_TRACE_LEVEL, &traceLevel) != 0) abort();

    SDecodingParam sDecParam;
    memset(&sDecParam, 0, sizeof(sDecParam));
    sDecParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_AVC;
    if(pSvcDecoder->Initialize(&sDecParam) != 0) abort();


    static const unsigned long long sec = 1000000000;

    unsigned long long avg = 0;
    const size_t samples = 100;
	for(size_t i = 0; i < samples; ++i) {
        unsigned char* frame[3] = {NULL, NULL, NULL};
        struct timespec start, end;

        if(clock_gettime(CLOCK_REALTIME, &start) != 0) abort();
        decode(pSvcDecoder, buf, size, frame);
        if(clock_gettime(CLOCK_REALTIME, &end) != 0) abort();

        // free frame???

        unsigned long long nstart = sec*start.tv_sec + start.tv_nsec;
        unsigned long long nend = sec*end.tv_sec + end.tv_nsec;
        avg += (nend - nstart);
    }
    if(pSvcDecoder->Uninitialize() != 0) abort();
    WelsDestroyDecoder(pSvcDecoder); pSvcDecoder = NULL;

    free(buf); buf = NULL;

    printf("Avg time (%ld sameples): %.09f s\n", samples, (avg/(float)(sec))/samples);

    return 0;
};


//
// Created by William.Hua on 2021/3/29.
//

#if defined(__cplusplus)
extern "C"
{
#endif

#include <libavcodec/avcodec.h>

#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/frame.h>
#include <libavutil/samplefmt.h>
#include <libavformat/avformat.h>

#if defined(__cplusplus)
}
#endif

#include <iostream>
#include <string>
using namespace std;

AVOutputFormat *av_guess_format(const char *short_name, const char *filename,
                                const char *mime_type)
{
    AVOutputFormat *fmt = NULL, *fmt_found;
    int score_max, score;

    /* Find the proper file type. */
    fmt_found = NULL;
    score_max = 0;
    while ((fmt = av_oformat_next(fmt))) {
        printf("%s\n", fmt->name);
    }
    return fmt_found;
}

void printFormats(){
    AVOutputFormat *fmt = NULL, *fmt_found;
    fmt_found = NULL;
    while ((fmt = av_oformat_next(fmt))) {
        printf("format name: %s\n", fmt->name);
    }
}

void printCodecs(){
    AVCodec * codec = av_codec_next(NULL);
    while(codec != NULL)
    {
        fprintf(stderr, "%s, %d, %s\n", codec->long_name, codec->id, codec->name);
        codec = av_codec_next(codec);
    }
}

int main()
{
    printFormats();
    printCodecs();
}
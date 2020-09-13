
//
// Created by William.Hua on 2020/9/2.
//

#if defined(__cplusplus)
extern "C"
{
#endif

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>

#if defined(__cplusplus)
}
#endif

#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

int main(int argc, char* argv[])
{
    const string path = argv[1];

    AVFormatContext *pFormatContext = avformat_alloc_context();
    if(avformat_open_input(&pFormatContext, path.c_str(), NULL, NULL) < 0){
        cerr << "open file failed\n";
        return -1;
    }

    avformat_find_stream_info(pFormatContext, NULL);

    // find the audio stream info
    int audio_stream_index = -1;
    AVCodec *pCodec = NULL;
    AVCodecParameters *pCodecParameters = NULL;
    for(int i = 0; i < pFormatContext->nb_streams; ++i){
        if(pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
            audio_stream_index = i;
            pCodecParameters = pFormatContext->streams[i]->codecpar;
            pCodec = avcodec_find_decoder(pCodecParameters->codec_id);
        }
    }

    // create codec context
    AVCodecContext *pCodecContext = avcodec_alloc_context3(pCodec);
    avcodec_parameters_to_context(pCodecContext, pCodecParameters);
    avcodec_open2(pCodecContext, pCodec, NULL);

    AVPacket *pPacket = av_packet_alloc();
    AVFrame *pFrame = av_frame_alloc();

    const int output_channel = 2;

    struct SwrContext* swr = swr_alloc();
    av_opt_set_int(swr, "in_channel_count",  pCodecContext->channels, 0);
    av_opt_set_int(swr, "out_channel_count", output_channel, 0);
    av_opt_set_int(swr, "in_channel_layout", pCodecContext->channel_layout, 0);
    av_opt_set_int(swr, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
    av_opt_set_int(swr, "in_sample_rate", pCodecContext->sample_rate, 0);
    av_opt_set_int(swr, "out_sample_rate", pCodecContext->sample_rate, 0);
    av_opt_set_sample_fmt(swr, "in_sample_fmt", pCodecContext->sample_fmt, 0);
    av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_FLTP, 0);
    swr_init(swr);

    uint8_t* internal_buffer[output_channel];

    std::fstream out("outfile.pcm", std::ios::out | std::ios::binary);

    for(;av_read_frame(pFormatContext, pPacket) >= 0;){
        if(pPacket->stream_index == audio_stream_index){
            // decode audio packet
            int ret = avcodec_send_packet(pCodecContext, pPacket);

            for(;ret >= 0;){
                ret = avcodec_receive_frame(pCodecContext, pFrame);

                if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
                else if(ret < 0) {
                    cerr << "error while receiving a frame from decoder" << endl;
                    break;
                }

                av_samples_alloc(internal_buffer, NULL, output_channel, pFrame->nb_samples, AV_SAMPLE_FMT_FLTP, 0);
                swr_convert(swr, internal_buffer, pFrame->nb_samples, (const uint8_t**) pFrame->data, pFrame->nb_samples);
                out.write((char*)(internal_buffer[0]), sizeof(float) * pFrame->nb_samples);

                av_freep(&internal_buffer[0]);
            }

            av_packet_unref(pPacket);
        }
    }


    swr_free(&swr);
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    avcodec_free_context(&pCodecContext);
    avformat_close_input(&pFormatContext);

    return 0;

}
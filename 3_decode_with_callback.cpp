
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
#include <stdio.h>
using namespace std;

class Reader
{
public:
    int open(const string& input_file)
    {
        fp = fopen(input_file.c_str(), "rb");
        if(!fp){
            cerr << "open file failed";
            return -1;
        }

        fseek(fp, 0, SEEK_END);
        len = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        return 0;
    }

    int read(uint8_t* dst_buf, int size)
    {
        if(fp){
            return fread(dst_buf, sizeof(uint8_t), size, fp);
        }

        return 0;
    }

    int seek(int pos, int mode){
        return fseek(fp, pos, SEEK_SET);
    }

    size_t length() {
        return len;
    }

    ~Reader()
    {
        if(fp){
            fclose(fp);
            fp = NULL;
        }
    }

    static int readCallback(void* client_data, uint8_t* buf, int buf_size)
    {
        auto* self = static_cast<Reader*>(client_data);
        int read_ret = self->read(buf, buf_size);
        if(read_ret == 0){
            cout << "read_ret:" << read_ret << endl;
            self->read(buf, buf_size);
        }
        return read_ret;
    }

    static int64_t seekCallback(void *client_data, int64_t offset, int whence){
        auto* self = static_cast<Reader*>(client_data);

        if(whence == AVSEEK_SIZE){
            return self->length();
        }else{
            int seek_ret = self->seek(offset, whence);
            cout << seek_ret << endl;
            return seek_ret;
        }
    }

    FILE* fp = NULL;
    size_t len={0};
};

int main(int argc, char* argv[])
{
    if(argc < 2){
        cerr << "Usage: decoder_audio /full/path/to/audio_file\n";
        return -1;
    }

    const string path = argv[1];

    const int kAVIOCtxBufferSize = 4096;

    AVFormatContext *pFormatContext = avformat_alloc_context();

    Reader reader;
    reader.open(path);

    auto* avio_ctx_buffer = (uint8_t*)av_malloc(kAVIOCtxBufferSize);
    AVIOContext* avio_ctx = avio_alloc_context(avio_ctx_buffer, kAVIOCtxBufferSize,
                                              0, &reader, &Reader::readCallback, NULL, &Reader::seekCallback);

    pFormatContext->pb = avio_ctx;
    avformat_open_input(&pFormatContext, NULL, NULL, NULL);


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
                // only output left channel
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
    av_freep(&avio_ctx->buffer);
    avio_context_free(&avio_ctx);

    return 0;

}
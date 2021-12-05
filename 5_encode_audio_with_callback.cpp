//
// Created by bytedance on 2021/12/3.
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

#include "testing_file_byte_stream.h"
#include <iostream>
#include <string>
using namespace std;

static void encode(AVCodecContext *ctx, AVFrame *frame, AVPacket *pkt,
                   AVFormatContext* pFormatCtx, AVStream* audio_st)
{
    int ret;

    /* send the frame for encoding */
    ret = avcodec_send_frame(ctx, frame);
    if (ret < 0) {
        fprintf(stderr, "Error sending the frame to the encoder\n");
        exit(1);
    }

    /* read all the available output packets (in general there may be any
     * number of them */
    while (ret >= 0) {
        ret = avcodec_receive_packet(ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error encoding audio frame\n");
            exit(1);
        }

        pkt->stream_index = audio_st->index;
        if(frame){
            pkt->pts = frame->pts;
            frame->pts += 100;
        }
        av_write_frame(pFormatCtx, pkt);
        av_packet_unref(pkt);
    }
}

int readPacketCallback(void *opaque, uint8_t *buf, int buf_size){
    cout << "readPacketCallback" << endl;
    auto* byte_stream = static_cast<mammon::ByteStream*>(opaque);
    int ret = byte_stream->read(buf, buf_size);
    if(ret == 0){
        return AVERROR_EOF;
    }
    return ret;
}
int writePacketCallback(void *opaque, uint8_t *buf, int buf_size){
    cout << "writePacketCallback" << endl;
    auto* byte_stream = static_cast<mammon::ByteStream*>(opaque);
    return byte_stream->write(buf, buf_size);
}
int64_t seekCallback(void *opaque, int64_t offset, int whence){
    cout << "seek" << endl;
    auto* byte_stream = static_cast<mammon::ByteStream*>(opaque);

    if(whence == AVSEEK_SIZE){
        return byte_stream->length();
    }else{
        int seek_ret = byte_stream->seekp(offset, SEEK_SET);
        cout << seek_ret << endl;
        return seek_ret;
    }
}


int main(int argc, char* argv[])
{
    if(argc < 2){
        cerr << "Usage:" << argv[0] <<  " /full/path/to/output_file\n";
        return -1;
    }

    const string output_file = argv[1];

    {
        const int kAVIOCtxBufferSize = 4096;
        const std::string format_name = "mp3";
        mammon::FileByteStream byte_stream(output_file);

        AVFormatContext *pFormatContext = NULL;
        avformat_alloc_output_context2(&pFormatContext, NULL, format_name.c_str(), NULL);
        if(!pFormatContext){
            cerr << "Cannot alloc output context\n";
            return -1;
        }

        auto* avio_ctx_buffer = (uint8_t*)av_malloc(kAVIOCtxBufferSize);

        AVIOContext* avio_ctx = avio_alloc_context(avio_ctx_buffer, kAVIOCtxBufferSize,
                                                   1, &byte_stream,
                                                   NULL,
                                                   writePacketCallback,
                                                   NULL);
        assert(avio_ctx != NULL);

        pFormatContext->pb = avio_ctx;
        pFormatContext->flags=AVFMT_FLAG_CUSTOM_IO;

        // create new audio stream
        AVStream* audio_st = avformat_new_stream(pFormatContext, 0);
        if(!audio_st){
            cerr << "Cannot create audio stream\n";
            return -1;
        }

        // set codec context parameters
        const int sample_rate = 44100;
        const int num_channels = 2;
        const int bit_rate = 64000;
        AVCodecContext* codec_ctx = audio_st->codec;
        codec_ctx->codec_id = pFormatContext->oformat->audio_codec;
        codec_ctx->codec_type = AVMEDIA_TYPE_AUDIO;
        codec_ctx->sample_rate = sample_rate;
        codec_ctx->channels = num_channels;
        codec_ctx->sample_fmt = AV_SAMPLE_FMT_FLTP; // planar float
        codec_ctx->channel_layout = av_get_default_channel_layout(num_channels);
        codec_ctx->bit_rate = bit_rate;

        // print detailed information about output format
        av_dump_format(pFormatContext, 0, output_file.c_str(), 1);

        // find encode codec
        AVCodec* codec = avcodec_find_encoder(codec_ctx->codec_id);
        if(!codec){
            cerr << "Cannot find encode codec\n";
            return -1;
        }

        // open encode codec
        if(avcodec_open2(codec_ctx, codec, NULL) < 0){
            cerr << "Cannot open codec\n";
            return -1;
        }

        // alloc frame
        AVFrame* frame = av_frame_alloc();
        if(!frame){
            cerr << "Cannot alloc frame\n";
            return -1;
        }
        frame->nb_samples = codec_ctx->frame_size != 0 ? codec_ctx->frame_size : 1024;
        frame->format = codec_ctx->sample_fmt;
        frame->channel_layout = codec_ctx->channel_layout;

        // allocate buffer for frame
        if(av_frame_get_buffer(frame, 0) < 0){
            cerr << "Cannot allocate buffer for frame\n";
            return -1;
        }
        // make sure frame is writeable
        if(av_frame_make_writable(frame) < 0){
            cerr << "Cannot make frame writeable\n";
            return -1;
        }

        // alloc packet
        AVPacket* pkt = av_packet_alloc();
        if(!pkt){
            cerr << "Cannot allocate packet\n";
            return -1;
        }

        // write header
        if(avformat_write_header(pFormatContext, NULL) < 0){
            cerr << "Cannot write header\n";
            return -1;
        }

        // write some
        const int num_output_frame = 500;
        float t = 0;
        float tincr = 2 * M_PI * 440.0f / sample_rate; // 440Hz sine wave
        auto* left_channel = reinterpret_cast<float*>(frame->data[0]);
        auto* right_channel = reinterpret_cast<float*>(frame->data[1]);
        frame->pts = 0;
        for(int i = 0; i < num_output_frame; ++i){
            // generate sine wave
            for(int j = 0; j < frame->nb_samples; ++j){
                left_channel[j] = sin(t);
                right_channel[j] = left_channel[j];

                t += tincr;
            }

            // encode sine wave, and send them to output file
            encode(codec_ctx, frame, pkt, pFormatContext, audio_st);
        }

        // flush
        encode(codec_ctx, NULL, pkt, pFormatContext, audio_st);

        av_freep(&avio_ctx->buffer);
        avio_context_free(&avio_ctx);
        avformat_close_input(&pFormatContext);
    }
}
// ═══════════════════════════════════════════════════════════════════════════════
// FILE: src/pipeline/decoder/decoder_base.cpp
// ═══════════════════════════════════════════════════════════════════════════════

#include "aether/pipeline/decoder.hpp"
#include "aether/utils/logging.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
}

namespace aether {

// ═══════════════════════════════════════════════════════════════════════════════
// FFmpeg Video Decoder Implementation
// ═══════════════════════════════════════════════════════════════════════════════

class FFmpegVideoDecoder : public Decoder {
public:
    FFmpegVideoDecoder() = default;
    ~FFmpegVideoDecoder() override { Close(); }

    Result<void> Open(const DecoderConfig& config) override {
        config_ = config;

        // Find decoder
        const AVCodec* codec = FindFFmpegCodec(config.codec_id);
        if (!codec) {
            return Error::Make(ErrorCode::UnsupportedCodec, "No decoder found for codec");
        }

        // Allocate context
        codec_ctx_ = avcodec_alloc_context3(codec);
        if (!codec_ctx_) {
            return Error::Make(ErrorCode::OutOfMemory, "Failed to allocate decoder context");
        }

        // Set options
        codec_ctx_->thread_count = config.threads > 0 ? config.threads : 0;
        codec_ctx_->thread_type = FF_THREAD_FRAME | FF_THREAD_SLICE;

        if (config.low_latency) {
            codec_ctx_->flags |= AV_CODEC_FLAG_LOW_DELAY;
        }

        // Hardware acceleration
        if (config.hardware_accel) {
            SetupHardwareAcceleration();
        }

        // Open codec
        int ret = avcodec_open2(codec_ctx_, codec, nullptr);
        if (ret < 0) {
            char err_buf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, err_buf, sizeof(err_buf));
            return Error::Make(ErrorCode::DecoderError, "Failed to open decoder: {}", err_buf);
        }

        initialized_ = true;
        GetLogger().Debug("Opened video decoder: {}", codec->name);
        return {};
    }

    void Close() override {
        if (codec_ctx_) {
            avcodec_free_context(&codec_ctx_);
            codec_ctx_ = nullptr;
        }
        initialized_ = false;
    }

    Result<FramePtr> Decode(const Packet& packet) override {
        if (!initialized_) {
            return Error::Make(ErrorCode::InvalidState, "Decoder not initialized");
        }

        // Send packet
        AVPacket* av_pkt = av_packet_alloc();
        av_pkt->data = const_cast<u8*>(packet.data.data());
        av_pkt->size = static_cast<i32>(packet.data.size());
        av_pkt->pts = packet.pts;
        av_pkt->dts = packet.dts;

        int ret = avcodec_send_packet(codec_ctx_, av_pkt);
        av_packet_free(&av_pkt);

        if (ret < 0) {
            return Error::Make(ErrorCode::DecoderError, "Failed to send packet");
        }

        // Receive frame
        AVFrame* frame = av_frame_alloc();
        ret = avcodec_receive_frame(codec_ctx_, frame);

        if (ret == AVERROR(EAGAIN)) {
            av_frame_free(&frame);
            return Error::Make(ErrorCode::NeedMoreData, "Need more data");
        }

        if (ret == AVERROR_EOF) {
            av_frame_free(&frame);
            return Error::Make(ErrorCode::EndOfStream, "End of stream");
        }

        if (ret < 0) {
            av_frame_free(&frame);
            return Error::Make(ErrorCode::DecoderError, "Decode error");
        }

        // Convert to our frame format
        auto out_frame = std::make_unique<Frame>();
        out_frame->m_type = Frame::Type::Video;
        out_frame->m_video.size = SizeU{static_cast<u32>(frame->width), static_cast<u32>(frame->height)};
        out_frame->m_video.format = MapFFmpegPixelFormat(static_cast<AVPixelFormat>(frame->format));
        out_frame->m_video.pts = frame->pts;
        out_frame->m_video.dts = frame->dts;
        out_frame->m_video.duration = frame->duration;
        out_frame->m_video.is_keyframe = frame->key_frame;

        for (int i = 0; i < 4; ++i) {
            out_frame->m_video.data[i] = frame->data[i];
            out_frame->m_video.linesize[i] = frame->linesize[i];
        }

        out_frame->m_valid = true;
        av_frame_free(&frame);
        return out_frame;
    }

    Result<void> SendPacket(const Packet& packet) override {
        if (!initialized_) {
            return Error::Make(ErrorCode::InvalidState, "Decoder not initialized");
        }

        AVPacket* av_pkt = av_packet_alloc();
        av_pkt->data = const_cast<u8*>(packet.data.data());
        av_pkt->size = static_cast<i32>(packet.data.size());
        av_pkt->pts = packet.pts;
        av_pkt->dts = packet.dts;

        int ret = avcodec_send_packet(codec_ctx_, av_pkt);
        av_packet_free(&av_pkt);

        if (ret < 0) {
            return Error::Make(ErrorCode::DecoderError, "Failed to send packet");
        }

        return {};
    }

    Result<FramePtr> ReceiveFrame() override {
        AVFrame* frame = av_frame_alloc();
        int ret = avcodec_receive_frame(codec_ctx_, frame);

        if (ret == AVERROR(EAGAIN)) {
            av_frame_free(&frame);
            return Error::Make(ErrorCode::NeedMoreData, "Need more data");
        }

        if (ret == AVERROR_EOF) {
            av_frame_free(&frame);
            return Error::Make(ErrorCode::EndOfStream, "End of stream");
        }

        if (ret < 0) {
            av_frame_free(&frame);
            return Error::Make(ErrorCode::DecoderError, "Decode error");
        }

        auto out_frame = std::make_unique<Frame>();
        out_frame->m_type = Frame::Type::Video;
        out_frame->m_video.size = SizeU{static_cast<u32>(frame->width), static_cast<u32>(frame->height)};
        out_frame->m_video.format = MapFFmpegPixelFormat(static_cast<AVPixelFormat>(frame->format));
        out_frame->m_video.pts = frame->pts;
        out_frame->m_video.dts = frame->dts;
        out_frame->m_video.duration = frame->duration;
        out_frame->m_video.is_keyframe = frame->key_frame;
        out_frame->m_valid = true;

        av_frame_free(&frame);
        return out_frame;
    }

    Result<void> Flush() override {
        if (!initialized_) {
            return Error::Make(ErrorCode::InvalidState, "Decoder not initialized");
        }

        avcodec_flush_buffers(codec_ctx_);
        return {};
    }

    [[nodiscard]] CodecInfo GetCodecInfo() const override {
        CodecInfo info;
        if (codec_ctx_ && codec_ctx_->codec) {
            info.id = MapFFmpegCodec(codec_ctx_->codec_id);
            info.name = codec_ctx_->codec->name;
            info.long_name = codec_ctx_->codec->long_name;
            info.type = CodecType::Video;
            info.is_decoder = true;
        }
        return info;
    }

    [[nodiscard]] DecoderInfo GetInfo() const override {
        DecoderInfo info;
        info.name = "FFmpeg";
        info.long_name = "FFmpeg Video Decoder";
        info.type = CodecType::Video;
        info.is_hardware = config_.hardware_accel;
        return info;
    }

    [[nodiscard]] bool IsHardware() const override {
        return config_.hardware_accel;
    }

private:
    const AVCodec* FindFFmpegCodec(CodecID id) {
        AVCodecID av_id = AV_CODEC_ID_NONE;
        switch (id) {
            case CodecID::H264: av_id = AV_CODEC_ID_H264; break;
            case CodecID::H265: av_id = AV_CODEC_ID_HEVC; break;
            case CodecID::VP9: av_id = AV_CODEC_ID_VP9; break;
            case CodecID::AV1: av_id = AV_CODEC_ID_AV1; break;
            case CodecID::MPEG2: av_id = AV_CODEC_ID_MPEG2VIDEO; break;
            case CodecID::MPEG4: av_id = AV_CODEC_ID_MPEG4; break;
            case CodecID::VP8: av_id = AV_CODEC_ID_VP8; break;
            default: break;
        }
        return avcodec_find_decoder(av_id);
    }

    void SetupHardwareAcceleration() {
        // Hardware acceleration setup would go here
        // VAAPI, NVDEC, VideoToolbox, etc.
    }

    PixelFormat MapFFmpegPixelFormat(AVPixelFormat fmt) {
        switch (fmt) {
            case AV_PIX_FMT_YUV420P: return PixelFormat::YUV420P;
            case AV_PIX_FMT_YUV422P: return PixelFormat::YUV422P;
            case AV_PIX_FMT_YUV444P: return PixelFormat::YUV444P;
            case AV_PIX_FMT_YUV420P10LE: return PixelFormat::YUV420P10;
            case AV_PIX_FMT_NV12: return PixelFormat::NV12;
            case AV_PIX_FMT_RGB24: return PixelFormat::RGB24;
            case AV_PIX_FMT_RGBA: return PixelFormat::RGBA32;
            default: return PixelFormat::Unknown;
        }
    }

    CodecID MapFFmpegCodec(AVCodecID id) {
        switch (id) {
            case AV_CODEC_ID_H264: return CodecID::H264;
            case AV_CODEC_ID_HEVC: return CodecID::H265;
            case AV_CODEC_ID_VP9: return CodecID::VP9;
            case AV_CODEC_ID_AV1: return CodecID::AV1;
            case AV_CODEC_ID_MPEG2VIDEO: return CodecID::MPEG2;
            case AV_CODEC_ID_MPEG4: return CodecID::MPEG4;
            case AV_CODEC_ID_VP8: return CodecID::VP8;
            default: return CodecID::Unknown;
        }
    }

private:
    DecoderConfig config_;
    AVCodecContext* codec_ctx_ = nullptr;
    bool initialized_ = false;
};

// ═══════════════════════════════════════════════════════════════════════════════
// Factory Functions
// ═══════════════════════════════════════════════════════════════════════════════

std::unique_ptr<Decoder> CreateVideoDecoder(CodecID codec_id, const DecoderConfig& config) {
    (void)codec_id;
    auto decoder = std::make_unique<FFmpegVideoDecoder>();
    decoder->Open(config);
    return decoder;
}

std::unique_ptr<Decoder> CreateAudioDecoder(CodecID codec_id, const DecoderConfig& config) {
    (void)codec_id;
    (void)config;
    return nullptr;  // Would create audio decoder
}

std::unique_ptr<Decoder> CreateFFmpegDecoder(CodecID codec_id) {
    (void)codec_id;
    return std::make_unique<FFmpegVideoDecoder>();
}

} // namespace aether

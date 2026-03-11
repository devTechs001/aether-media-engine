// ═══════════════════════════════════════════════════════════════════════════════
// FILE: src/pipeline/demuxer/demuxer_ffmpeg.cpp
// ═══════════════════════════════════════════════════════════════════════════════

#include "aether/pipeline/demuxer.hpp"
#include "aether/core/media_source.hpp"
#include "aether/utils/logging.hpp"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/error.h>
}

namespace aether {

class FFmpegDemuxer : public Demuxer {
public:
    FFmpegDemuxer() = default;
    ~FFmpegDemuxer() override { Close(); }

    Result<void> Open(Shared<MediaSource> source) override {
        source_ = source;

        if (auto result = source_->Open(); !result) {
            return result;
        }

        // Allocate format context
        format_ctx_ = avformat_alloc_context();
        if (!format_ctx_) {
            return Error::Make(ErrorCode::OutOfMemory, "Failed to allocate format context");
        }

        // Open input
        AVInputFormat* input_format = nullptr;
        int ret = avformat_open_input(&format_ctx_, source_->GetInfo().location.c_str(),
                                      input_format, nullptr);
        if (ret < 0) {
            char err_buf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, err_buf, sizeof(err_buf));
            return Error::Make(ErrorCode::DemuxerError, "Failed to open input: {}", err_buf);
        }

        // Read stream info
        ret = avformat_find_stream_info(format_ctx_, nullptr);
        if (ret < 0) {
            return Error::Make(ErrorCode::DemuxerError, "Failed to find stream info");
        }

        // Build format info
        BuildFormatInfo();

        GetLogger().Debug("FFmpeg demuxer opened: {} streams", format_ctx_->nb_streams);
        return {};
    }

    void Close() override {
        if (format_ctx_) {
            avformat_close_input(&format_ctx_);
            format_ctx_ = nullptr;
        }
        if (source_) {
            source_->Close();
            source_.reset();
        }
    }

    [[nodiscard]] FormatInfo GetFormatInfo() const override {
        return format_info_;
    }

    Result<PacketPtr> ReadPacket() override {
        if (!format_ctx_) {
            return Error::Make(ErrorCode::InvalidState, "Demuxer not open");
        }

        AVPacket* pkt = av_packet_alloc();
        if (!pkt) {
            return Error::Make(ErrorCode::OutOfMemory, "Failed to allocate packet");
        }

        int ret = av_read_frame(format_ctx_, pkt);
        if (ret < 0) {
            av_packet_free(&pkt);

            if (ret == AVERROR_EOF) {
                return Error::Make(ErrorCode::EndOfStream, "End of stream");
            }

            char err_buf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, err_buf, sizeof(err_buf));
            return Error::Make(ErrorCode::DemuxerError, "Read error: {}", err_buf);
        }

        auto packet = std::make_unique<Packet>();
        packet->type = static_cast<MediaType>(format_ctx_->streams[pkt->stream_index]->codecpar->codec_type);
        packet->codec_id = MapFFmpegCodec(pkt->stream_index);
        packet->pts = pkt->pts;
        packet->dts = pkt->dts;
        packet->duration = pkt->duration;
        packet->is_keyframe = !!(pkt->flags & AV_PKT_FLAG_KEY);
        packet->stream_index = pkt->stream_index;
        packet->data.assign(pkt->data, pkt->data + pkt->size);

        av_packet_free(&pkt);
        return packet;
    }

    Result<void> Seek(i64 pts, i32 stream_index) override {
        if (!format_ctx_) {
            return Error::Make(ErrorCode::InvalidState, "Demuxer not open");
        }

        int ret = av_seek_frame(format_ctx_, stream_index, pts, AVSEEK_FLAG_BACKWARD);
        if (ret < 0) {
            return Error::Make(ErrorCode::DemuxerError, "Seek failed");
        }

        return {};
    }

    [[nodiscard]] i64 GetDuration() const override {
        if (!format_ctx_ || format_ctx_->duration == AV_NOPTS_VALUE) {
            return 0;
        }
        return format_ctx_->duration / AV_TIME_BASE * 1000;  // ms
    }

    [[nodiscard]] i64 GetBitrate() const override {
        if (!format_ctx_) {
            return 0;
        }
        return format_ctx_->bit_rate;
    }

    [[nodiscard]] DemuxerInfo GetInfo() const override {
        DemuxerInfo info;
        info.name = "FFmpeg";
        info.long_name = "FFmpeg Demuxer";
        info.supports_seek = true;
        return info;
    }

private:
    void BuildFormatInfo() {
        if (format_ctx_->iformat) {
            format_info_.format_name = format_ctx_->iformat->name ? format_ctx_->iformat->name : "";
            format_info_.format_long_name = format_ctx_->iformat->long_name ? format_ctx_->iformat->long_name : "";
        }

        if (format_ctx_->duration != AV_NOPTS_VALUE) {
            format_info_.duration_ms = format_ctx_->duration / AV_TIME_BASE * 1000;
        }

        format_info_.bitrate = format_ctx_->bit_rate;

        // Parse streams
        for (u32 i = 0; i < format_ctx_->nb_streams; ++i) {
            AVStream* stream = format_ctx_->streams[i];
            AVCodecParameters* par = stream->codecpar;

            FormatInfo::StreamInfo stream_info;
            stream_info.index = static_cast<i32>(i);
            stream_info.codec_id = MapFFmpegCodec(par->codec_id);
            stream_info.bitrate = par->bit_rate;

            if (par->codec_type == AVMEDIA_TYPE_VIDEO) {
                stream_info.type = MediaType::Video;
                stream_info.resolution = SizeU{static_cast<u32>(par->width), static_cast<u32>(par->height)};
                if (stream->avg_frame_rate.den > 0) {
                    stream_info.fps = av_q2d(stream->avg_frame_rate);
                }
            } else if (par->codec_type == AVMEDIA_TYPE_AUDIO) {
                stream_info.type = MediaType::Audio;
                stream_info.sample_rate = par->sample_rate;
                stream_info.channels = par->ch_layout.nb_channels;
            } else if (par->codec_type == AVMEDIA_TYPE_SUBTITLE) {
                stream_info.type = MediaType::Subtitle;
            }

            format_info_.streams.push_back(stream_info);
        }

        // Parse metadata
        AVDictionaryEntry* tag = nullptr;
        while ((tag = av_dict_get(format_ctx_->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
            if (std::string(tag->key) == "title") format_info_.title = tag->value;
            else if (std::string(tag->key) == "artist") format_info_.artist = tag->value;
            else if (std::string(tag->key) == "album") format_info_.album = tag->value;
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
            case AV_CODEC_ID_AAC: return CodecID::AAC;
            case AV_CODEC_ID_MP3: return CodecID::MP3;
            case AV_CODEC_ID_AC3: return CodecID::AC3;
            case AV_CODEC_ID_FLAC: return CodecID::FLAC;
            case AV_CODEC_ID_OPUS: return CodecID::Opus;
            case AV_CODEC_ID_VORBIS: return CodecID::Vorbis;
            default: return CodecID::Unknown;
        }
    }

    CodecID MapFFmpegCodec(int stream_index) {
        if (!format_ctx_ || stream_index < 0 ||
            static_cast<usize>(stream_index) >= format_ctx_->nb_streams) {
            return CodecID::Unknown;
        }
        return MapFFmpegCodec(format_ctx_->streams[stream_index]->codecpar->codec_id);
    }

private:
    Shared<MediaSource> source_;
    AVFormatContext* format_ctx_ = nullptr;
    FormatInfo format_info_;
};

std::unique_ptr<Demuxer> CreateFFmpegDemuxer() {
    return std::make_unique<FFmpegDemuxer>();
}

std::unique_ptr<Demuxer> CreateDemuxer(ContainerFormat format) {
    (void)format;
    return std::make_unique<FFmpegDemuxer>();
}

std::unique_ptr<Demuxer> CreateDemuxerByName(const std::string& name) {
    (void)name;
    return std::make_unique<FFmpegDemuxer>();
}

} // namespace aether

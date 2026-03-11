// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/media/metadata.hpp
// DESCRIPTION: Media metadata handling
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_MEDIA_METADATA_HPP
#define AETHER_MEDIA_METADATA_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"

#include <string>
#include <unordered_map>
#include <variant>
#include <optional>

namespace aether {

/**
 * @struct Metadata
 * @brief Media metadata container
 */
struct AETHER_API Metadata {
    using Value = std::variant<std::string, i64, f64, std::vector<u8>>;

    // Standard tags
    std::optional<std::string> title;
    std::optional<std::string> artist;
    std::optional<std::string> album;
    std::optional<std::string> album_artist;
    std::optional<std::string> composer;
    std::optional<std::string> performer;
    std::optional<i64> track_number;
    std::optional<i64> disc_number;
    std::optional<std::string> year;
    std::optional<std::string> date;
    std::optional<std::string> genre;
    std::optional<std::string> comment;
    std::optional<std::string> copyright;
    std::optional<std::string> encoder;

    // Video specific
    std::optional<std::string> director;
    std::optional<std::string> studio;
    std::optional<std::string> series;
    std::optional<i64> season;
    std::optional<i64> episode;

    // Technical
    std::optional<i64> duration_ms;
    std::optional<i64> bitrate;
    std::optional<std::string> codec;

    // Cover art
    std::vector<u8> cover_art;
    std::string cover_art_mime;

    // Custom tags
    std::unordered_map<std::string, Value> custom;

    /**
     * @brief Get value by key
     */
    const Value* Get(const std::string& key) const;

    /**
     * @brief Set value by key
     */
    void Set(const std::string& key, Value value);

    /**
     * @brief Get all tags as map
     */
    std::unordered_map<std::string, Value> ToMap() const;

    /**
     * @brief Create from map
     */
    static Metadata FromMap(const std::unordered_map<std::string, Value>& map);
};

/**
 * @class MetadataParser
 * @brief Parse metadata from various formats
 */
class AETHER_API MetadataParser {
public:
    /**
     * @brief Parse ID3 tags (MP3)
     */
    static Result<Metadata> ParseID3(std::span<const u8> data);

    /**
     * @brief parse Vorbis comments (FLAC, OGG)
     */
    static Result<Metadata> ParseVorbisComments(std::span<const u8> data);

    /**
     * @brief Parse MP4 atoms
     */
    static Result<Metadata> ParseMP4(std::span<const u8> data);

    /**
     * @brief Parse Matroska tags
     */
    static Result<Metadata> ParseMatroska(std::span<const u8> data);

    /**
     * @brief Parse EXIF data (images)
     */
    static Result<Metadata> ParseEXIF(std::span<const u8> data);

    /**
     * @brief Parse XMP data
     */
    static Result<Metadata> ParseXMP(std::span<const u8> data);
};

} // namespace aether

#endif // AETHER_MEDIA_METADATA_HPP

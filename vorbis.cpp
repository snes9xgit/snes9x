#include "stb_vorbis.c"
#include "vorbis.h"
#include <string>
#include <stdexcept>

auto VorbisReader::load(std::vector<uint8_t>&& mem) -> bool {
    this->mem = mem;
    int error;
    this->vorbis = stb_vorbis_open_memory(this->mem.data(), this->mem.size(), &error, nullptr);
    if (this->vorbis) {
        auto info = stb_vorbis_get_info(this->vorbis);
        if (info.sample_rate != 44100) {
            std::printf("Wrong sample rate (not 44.1KHz) in .ogg");
            return false;
        }
        if (info.channels != 2) {
            std::printf("Wrong number of channels (not 2) in .ogg");
            return false;
        }
        auto comments = stb_vorbis_get_comment(this->vorbis);
        for (int i = 0; i < comments.comment_list_length; i++) {
            auto comment = std::string(comments.comment_list[i]);
            auto pos = comment.find('=');
            auto commentParts = std::array {
                comment.substr(0, pos),
                comment.substr(pos + 1)
            };
            if (commentParts.size() != 2) { continue; }
            if (commentParts[0] != "LOOPSTART") { continue; }
            this->loopPosition = 0;
            try {
                this->loopPosition = std::stoi(commentParts[1]);
            } catch (std::invalid_argument const& _e) {
                std::printf("Mis-formatted loop start in .ogg, defaulting to start");
            } catch (std::out_of_range const& _e) {
                std::printf("Out-of-range integer loop start in .ogg, defaulting to start");
            }
        }
        this->fillSamples();
        return true;
    } else {
        std::printf("Failed to open vorbis file, stb_vorbis error %d\n", error);
    }
    return false;
}

auto VorbisReader::fillSamples() -> void {
    this->bufPos = 0;
    this->buffer.resize(2048);
    auto count = stb_vorbis_get_samples_short_interleaved(
        this->vorbis, 2, this->buffer.data(), this->buffer.size()
    );
    if (size_t(count) * 2 < this->buffer.size()) {
        this->reachedEnd = true;
        this->buffer.resize(count * 2);
        return;
    }
    this->buffer.resize(count * 2);
}

auto VorbisReader::getSamples(bool loop) -> std::optional<std::array<int16_t, 2>> {
    if (this->vorbis == nullptr) {
        printf("unexpected vorbis nullptr in getSamples\n");
        return std::nullopt;
    }
    if (this->bufPos == this->buffer.size()) {
        if (this->reachedEnd) {
            this->reachedEnd = false;
            if (loop) {
                stb_vorbis_seek(this->vorbis, this->loopPosition);
            } else {
                stb_vorbis_seek(this->vorbis, 0);
                return std::nullopt;
            }
        }
        this->fillSamples();
    }
    auto val0 = this->buffer[this->bufPos];
    auto val1 = this->buffer[this->bufPos + 1];
    this->bufPos += 2;
    return std::array { val0, val1 };
}

auto VorbisReader::sampleNumber() -> size_t {
    if (this->vorbis == nullptr) {
        printf("unexpected vorbis nullptr in sampleNumber\n");
        return 0;
    }
    return stb_vorbis_get_sample_offset(this->vorbis);
}

auto VorbisReader::seek(size_t sampleNumber) -> void {
    if (this->vorbis == nullptr) {
        printf("unexpected vorbis nullptr in seek\n");
        return;
    }
    stb_vorbis_seek(this->vorbis, sampleNumber);
}

auto VorbisReader::close() -> void {
    if (this->vorbis != nullptr) {
        stb_vorbis_close(this->vorbis);
        this->vorbis = nullptr;
    }
    this->mem.clear();
    this->bufPos = 0;
    this->loopPosition = 0;
}

VorbisReader::VorbisReader() {}

VorbisReader::~VorbisReader() {
    this->close();
}

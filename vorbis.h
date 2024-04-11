#pragma once

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"
#include <cstdint>
#include <array>
#include <vector>
#include <optional>

class VorbisReader {
private:
	std::vector<uint8_t> mem;
	std::vector<int16_t> buffer;
	size_t bufPos;
	stb_vorbis* vorbis = nullptr;
	size_t loopPosition = 0;
	bool reachedEnd = false;

	auto fillSamples() -> void;
public:
	auto load(std::vector<uint8_t>&& mem) -> bool;
	auto getSamples(bool loop) -> std::optional<std::array<int16_t, 2>>;
	auto sampleNumber() -> size_t;
	auto seek(size_t sampleNumber) -> void;
	auto close() -> void;

	VorbisReader();
	~VorbisReader();
};

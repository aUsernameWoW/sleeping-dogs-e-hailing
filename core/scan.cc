#include "scan.hh"

#include <Windows.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "log.hh"

namespace scan
{
	struct Section
	{
		uint8_t* mBegin = nullptr;
		size_t mSize = 0;
	};

	static Section FindText()
	{
		// The main module, not whoever loaded us: the ASI loader runs inside dinput8.dll.
		auto* base = reinterpret_cast<uint8_t*>(GetModuleHandleW(nullptr));
		auto* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
		auto* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
		IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(nt);
		for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++section) {
			if (std::memcmp(section->Name, ".text", 6) == 0) {
				return { base + section->VirtualAddress, section->Misc.VirtualSize };
			}
		}
		return {};
	}

	static bool Parse(const char* pattern, std::vector<uint8_t>& bytes, std::vector<bool>& mask)
	{
		for (const char* p = pattern; *p;) {
			if (*p == ' ') {
				++p;
			}
			else if (*p == '?') {
				bytes.push_back(0);
				mask.push_back(false);
				while (*p == '?') {
					++p;
				}
			}
			else {
				char* end = nullptr;
				const unsigned long value = std::strtoul(p, &end, 16);
				if (end == p || value > 0xFF) {
					return false;
				}
				bytes.push_back(static_cast<uint8_t>(value));
				mask.push_back(true);
				p = end;
			}
		}
		return !bytes.empty() && mask.front();
	}

	static int Count(const Section& text, const std::vector<uint8_t>& bytes, const std::vector<bool>& mask, uint8_t*& found)
	{
		int count = 0;
		const size_t length = bytes.size();
		const uint8_t* last = text.mBegin + text.mSize - length;
		for (uint8_t* p = text.mBegin; p <= last; ++p) {
			p = static_cast<uint8_t*>(std::memchr(p, bytes[0], static_cast<size_t>(last - p) + 1));
			if (!p) {
				break;
			}
			size_t i = 1;
			while (i < length && (!mask[i] || p[i] == bytes[i])) {
				++i;
			}
			if (i == length) {
				found = p;
				++count;
			}
		}
		return count;
	}

	uint8_t* FindUnique(const char* name, const char* pattern)
	{
		static const Section text = FindText();
		std::vector<uint8_t> bytes;
		std::vector<bool> mask;
		if (!text.mBegin || !Parse(pattern, bytes, mask)) {
			LOG("scan: %s: bad pattern or no .text", name);
			return nullptr;
		}

		uint8_t* found = nullptr;
		int count = Count(text, bytes, mask, found);
		// Another mod may have hooked the function before we looked: SDEncore hooks five of ours (it loads first,
		// plugins load in name order), and MinHook writes a `jmp rel32` over the first 5 bytes. Take that jump in
		// their place; hooking the function again chains (MinHook copies the jump into our trampoline).
		const char* hooked = "";
		if (count == 0 && bytes.size() >= 12) {
			bytes[0] = 0xE9;
			std::fill(mask.begin() + 1, mask.begin() + 5, false);
			count = Count(text, bytes, mask, found);
			hooked = " (it starts with a jump: hooked by another mod)";
		}

		const auto* base = reinterpret_cast<uint8_t*>(GetModuleHandleW(nullptr));
		if (count != 1) {
			LOG("scan: %s: %d matches, not using it", name, count);
			return nullptr;
		}
		LOG("scan: %s at +0x%llX%s", name, static_cast<unsigned long long>(found - base), hooked);
		return found;
	}

	bool Matches(const uint8_t* at, const char* pattern)
	{
		std::vector<uint8_t> bytes;
		std::vector<bool> mask;
		if (!at || !Parse(pattern, bytes, mask)) {
			return false;
		}
		for (size_t i = 0; i < bytes.size(); ++i) {
			if (mask[i] && at[i] != bytes[i]) {
				return false;
			}
		}
		return true;
	}

	void* RipTarget(const uint8_t* disp, int trailing)
	{
		int32_t offset;
		std::memcpy(&offset, disp, sizeof(offset));
		return const_cast<uint8_t*>(disp + 4 + trailing + offset);
	}
}

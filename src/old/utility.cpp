#include "utility.h"

namespace KrisRenderer {
	uint32_t rgbaToHexArgb(float (&fg_color)[4]) {
		short r = fg_color[0] * 255;
		short g = fg_color[1] * 255;
		short b = fg_color[2] * 255;
		short a = fg_color[3] * 255;

		return (a << 24) | (r << 16) | (g << 8) | b;
	}

	uint32_t rgbaToHexRgba(float (&fg_color)[4]) {
		short r = fg_color[0] * 255;
		short g = fg_color[1] * 255;
		short b = fg_color[2] * 255;
		short a = fg_color[3] * 255;

		return (r << 24) | (g << 16) | (b << 8) | a;
	}

	glm::vec2 NDC_to_Screen(const glm::vec2& ndc, const int winWidth, const int winHeight) {
		assert (winWidth > 0 && winHeight > 0);
		// round to nearest pixel
		int x = static_cast<int>((ndc.x + 1) * winWidth / 2 + 0.5);
		int y = static_cast<int>((1 - ndc.y) * winHeight / 2 + 0.5);
		return glm::vec2(x, y);
	}
}

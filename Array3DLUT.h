#ifndef ARRAY3DLUT_H_
#define ARRAY3DLUT_H_

#include "FastLED.h"
#include "pixeltypes.h"

FASTLED_NAMESPACE_BEGIN

/// 3D array index
struct LUTCoord {
	union {
		struct {
			uint8_t redIndex;
			uint8_t greenIndex;
			uint8_t blueIndex;
		};
		uint8_t raw[3];
	};

	inline LUTCoord() __attribute__((always_inline))
    {
    }

    inline LUTCoord( uint8_t ir, uint8_t ig, uint8_t ib)  __attribute__((always_inline))
        : redIndex(ir), greenIndex(ig), blueIndex(ib)
    {
    }
};

/// 8 vertices of a cube
struct LUTSubcube {
	CRGB n000;
	CRGB n001;
	CRGB n010;
	CRGB n011;
	CRGB n100;
	CRGB n101;
	CRGB n110;
	CRGB n111;
};

/// This 3D LUT can be used provided that the space between entries is the same for all dimensions.
class Array3DLUT {
public:
	CRGB *entries;
	uint8_t size;
	float step;
	uint8_t maxSubcubeIndex;

	Array3DLUT(CRGB *entries, uint8_t size);
	virtual ~Array3DLUT();

	/// Retrieve an entry in the table.
	/// @param indexes - 3D index of entry
	/// @returns Entry value
	CRGB lookup(LUTCoord indexes);

	/// Returns the values of the 8 vertices of the target sub-cube.
	/// The given coordinates must be those of the lowest point.
	/// @param subcubeOriginCoord - lowest point coordinates of the desired subcube
	/// @returns Each (8) subcube values
	struct LUTSubcube lookup_subcube(LUTCoord subcubeOriginCoord);

	/// Similar to lookup but instead of a coordinate in the LUT,
	/// we pass a point and the result will be interpolated.
	/// @param input - RGB coordinates of a point in the LUT
	/// @returns Interpolated value in the LUT
	struct CRGB lookup3DTetrahedral(CRGB &input);
};

#endif /* ARRAY3DLUT_H_ */

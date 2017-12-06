
#include "Array3DLUT.h"

Array3DLUT::Array3DLUT(CRGB *entries, uint8_t size) {
	this->entries = entries;
	this->size = size;
	this->step = 255. / (size - 1);
	this->maxSubcubeIndex = size - 2;
}

Array3DLUT::~Array3DLUT() {
	// TODO Auto-generated destructor stub
}

CRGB Array3DLUT::lookup(LUTCoord indexes) {
	uint8_t offset =
			  (indexes.redIndex * this->size * this->size)
			+ (indexes.greenIndex * this->size)
			+ indexes.blueIndex;
	return this->entries[offset];
}


struct LUTSubcube Array3DLUT::lookup_subcube(LUTCoord subcubeOriginCoord) {
	struct LUTSubcube result;

	result.n000 = this->lookup(subcubeOriginCoord);
	result.n001 = this->lookup(LUTCoord(subcubeOriginCoord.redIndex, subcubeOriginCoord.greenIndex, subcubeOriginCoord.blueIndex+1));
	result.n010 = this->lookup(LUTCoord(subcubeOriginCoord.redIndex, subcubeOriginCoord.greenIndex+1, subcubeOriginCoord.blueIndex));
	result.n011 = this->lookup(LUTCoord(subcubeOriginCoord.redIndex, subcubeOriginCoord.greenIndex+1, subcubeOriginCoord.blueIndex+1));
	result.n100 = this->lookup(LUTCoord(subcubeOriginCoord.redIndex+1, subcubeOriginCoord.greenIndex, subcubeOriginCoord.blueIndex));
	result.n101 = this->lookup(LUTCoord(subcubeOriginCoord.redIndex+1, subcubeOriginCoord.greenIndex, subcubeOriginCoord.blueIndex+1));
	result.n110 = this->lookup(LUTCoord(subcubeOriginCoord.redIndex+1, subcubeOriginCoord.greenIndex+1, subcubeOriginCoord.blueIndex));
	result.n111 = this->lookup(LUTCoord(subcubeOriginCoord.redIndex+1, subcubeOriginCoord.greenIndex+1, subcubeOriginCoord.blueIndex+1));

	return result;
}

// Ref implem: https://github.com/ampas/CLF/blob/master/python/aces/clf/Array.py
struct CRGB Array3DLUT::lookup3DTetrahedral(CRGB &input) {
	LUTCoord subcubeOrigin; // Store the subcube's origin coordinates
	float dimensionalDistances[3]; // Distance for each dimensions

	for (int i = 0; i < 3; i++) { // for each rgb channel
		dimensionalDistances[i] = input.raw[i] / this->step; // Not yet valid. We should remove int part
		subcubeOrigin.raw[i] = uint8_t(dimensionalDistances[i]); // we put a guessed coordinate
		if (subcubeOrigin.raw[i] > maxSubcubeIndex) {
			// But, if we are on an upper border then we must take the
			// inner border or the subcube will not exist.
			subcubeOrigin.raw[i] = maxSubcubeIndex;
		}

		dimensionalDistances[i] -= subcubeOrigin.raw[i]; // Remove the int part
	}

	// Rebind for consistency with Truelight paper
	#define fx dimensionalDistances[0]
	#define fy dimensionalDistances[1]
	#define fz dimensionalDistances[2]

	struct CRGB result;
	struct LUTSubcube cube = this->lookup_subcube(subcubeOrigin);
	if (fx > fy) {
		if (fy > fz) {
			for (int i=0; i < 3; i++) {
				result.raw[i] = (
						(1-fx)  * cube.n000.raw[i] +
						(fx-fy) * cube.n100.raw[i] +
						(fy-fz) * cube.n110.raw[i] +
						(fz)    * cube.n111.raw[i] );
			}
		} else {
			if (fx > fz) {
				for (int i=0; i < 3; i++) {
					result.raw[i] = (
							(1-fx)  * cube.n000.raw[i] +
							(fx-fz) * cube.n100.raw[i] +
							(fz-fy) * cube.n101.raw[i] +
							(fy)    * cube.n111.raw[i] );
				}
			} else {
				for (int i=0; i < 3; i++) {
					result.raw[i] = (
							(1-fz)  * cube.n000.raw[i] +
							(fz-fx) * cube.n001.raw[i] +
							(fx-fy) * cube.n101.raw[i] +
							(fy)    * cube.n111.raw[i] );
				}
			}
		}
	} else {
		if (fz > fy) {
			for (int i=0; i < 3; i++) {
				result.raw[i] = (
						(1-fz)  * cube.n000.raw[i] +
						(fz-fy) * cube.n001.raw[i] +
						(fy-fx) * cube.n011.raw[i] +
						(fx)    * cube.n111.raw[i] );
			}
		} else {
			if (fz > fx) {
				for (int i=0; i < 3; i++) {
					result.raw[i] = (
							(1-fy)  * cube.n000.raw[i] +
							(fy-fz) * cube.n010.raw[i] +
							(fz-fx) * cube.n011.raw[i] +
							(fx)    * cube.n111.raw[i] );
				}
			} else {
				for (int i=0; i < 3; i++) {
					result.raw[i] = (
							(1-fy)  * cube.n000.raw[i] +
							(fy-fx) * cube.n010.raw[i] +
							(fx-fz) * cube.n011.raw[i] +
							(fz)    * cube.n111.raw[i] );
				}
			}
		}
	}

	return result;
}



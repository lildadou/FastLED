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


/// This 3D LUT can be used provided that the space between entries is the same for all dimensions.
template<uint8_t SIZE>
class Array3DLUT {
public:
	CRGB (*cube)[SIZE][SIZE];

	Array3DLUT(CRGB entries[SIZE][SIZE][SIZE]);
	virtual ~Array3DLUT();

	/// Similar to lookup but instead of a coordinate in the LUT,
	/// we pass a point and the result will be interpolated.
	/// @param input - RGB coordinates of a point in the LUT
	/// @returns Interpolated value in the LUT
	struct CRGB lookup3DTetrahedral(CRGB &input);
};




template<uint8_t SIZE>
Array3DLUT<SIZE>::Array3DLUT(CRGB (*entries)[SIZE][SIZE]) {
	this->cube = entries;
}

template<uint8_t SIZE>
Array3DLUT<SIZE>::~Array3DLUT() {
	// TODO Auto-generated destructor stub
}

// Ref implem: https://github.com/ampas/CLF/blob/master/python/aces/clf/Array.py
template<uint8_t SIZE>
struct CRGB Array3DLUT<SIZE>::lookup3DTetrahedral(CRGB &input) {
	LUTCoord subcubeDarkestVertice; // Store the subcube's origin coordinates
	uint8_t darkSpotDistances[3]; // Distance input <-> subcube's darkest vertice

	#define STEP (255/(SIZE-1))
	#define MAX_INDEX SIZE-2
	for (int i = 0; i < 3; i++) { // for each rgb channel
		subcubeDarkestVertice.raw[i] = input.raw[i] / STEP;
		darkSpotDistances[i] = input.raw[i] % STEP;

		if (subcubeDarkestVertice.raw[i] > MAX_INDEX) {
			// But, if we are on an upper border then we must take the
			// inner border or the subcube will not exist.
			subcubeDarkestVertice.raw[i] = MAX_INDEX;
			darkSpotDistances[i] = 255;
		}
	}

	// Rebind for consistency with Truelight paper
	#define fx darkSpotDistances[0]
	#define fy darkSpotDistances[1]
	#define fz darkSpotDistances[2]
	#define N000 this->cube[subcubeDarkestVertice.redIndex  ][subcubeDarkestVertice.greenIndex  ][subcubeDarkestVertice.blueIndex  ]
	#define N001 this->cube[subcubeDarkestVertice.redIndex  ][subcubeDarkestVertice.greenIndex  ][subcubeDarkestVertice.blueIndex+1]
	#define N010 this->cube[subcubeDarkestVertice.redIndex  ][subcubeDarkestVertice.greenIndex+1][subcubeDarkestVertice.blueIndex  ]
	#define N011 this->cube[subcubeDarkestVertice.redIndex  ][subcubeDarkestVertice.greenIndex+1][subcubeDarkestVertice.blueIndex+1]
	#define N100 this->cube[subcubeDarkestVertice.redIndex+1][subcubeDarkestVertice.greenIndex  ][subcubeDarkestVertice.blueIndex  ]
	#define N101 this->cube[subcubeDarkestVertice.redIndex+1][subcubeDarkestVertice.greenIndex  ][subcubeDarkestVertice.blueIndex+1]
	#define N110 this->cube[subcubeDarkestVertice.redIndex+1][subcubeDarkestVertice.greenIndex+1][subcubeDarkestVertice.blueIndex  ]
	#define N111 this->cube[subcubeDarkestVertice.redIndex+1][subcubeDarkestVertice.greenIndex+1][subcubeDarkestVertice.blueIndex+1]

	struct CRGB result;
	if (fx > fy) {
		if (fy > fz) {
			for (int i=0; i < 3; i++) {
				result.raw[i] = (
						(uint16_t)(255-fx) * N000.raw[i] +
						(uint16_t)(fx-fy)  * N100.raw[i] +
						(uint16_t)(fy-fz)  * N110.raw[i] +
						(uint16_t)(fz)     * N111.raw[i] ) >> 8;
			}
		} else {
			if (fx > fz) {
				for (int i=0; i < 3; i++) {
					result.raw[i] = (
							(uint16_t)(255-fx) * N000.raw[i] +
							(uint16_t)(fx-fz)  * N100.raw[i] +
							(uint16_t)(fz-fy)  * N101.raw[i] +
							(uint16_t)(fy)     * N111.raw[i] ) >> 8;
				}
			} else {
				for (int i=0; i < 3; i++) {
					result.raw[i] = (
							(uint16_t)(255-fz) * N000.raw[i] +
							(uint16_t)(fz-fx)  * N001.raw[i] +
							(uint16_t)(fx-fy)  * N101.raw[i] +
							(uint16_t)(fy)     * N111.raw[i] ) >> 8;
				}
			}
		}
	} else {
		if (fz > fy) {
			for (int i=0; i < 3; i++) {
				result.raw[i] = (
						(uint16_t)(255-fz) * N000.raw[i] +
						(uint16_t)(fz-fy)  * N001.raw[i] +
						(uint16_t)(fy-fx)  * N011.raw[i] +
						(uint16_t)(fx)     * N111.raw[i] ) >> 8;
			}
		} else {
			if (fz > fx) {
				for (int i=0; i < 3; i++) {
					result.raw[i] = (
							(uint16_t)(255-fy) * N000.raw[i] +
							(uint16_t)(fy-fz)  * N010.raw[i] +
							(uint16_t)(fz-fx)  * N011.raw[i] +
							(uint16_t)(fx)     * N111.raw[i] ) >> 8;
				}
			} else {
				for (int i=0; i < 3; i++) {
					result.raw[i] = (
							(uint16_t)(255-fy) * N000.raw[i] +
							(uint16_t)(fy-fx)  * N010.raw[i] +
							(uint16_t)(fx-fz)  * N011.raw[i] +
							(uint16_t)(fz)     * N111.raw[i] ) >> 8;
				}
			}
		}
	}

	return result;
}

#endif /* ARRAY3DLUT_H_ */

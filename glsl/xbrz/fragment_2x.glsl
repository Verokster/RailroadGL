/*
	xBRZ fragment shader
	based on libretro xBRZ shader
	https://github.com/libretro/glsl-shaders/tree/master/xbrz/shaders

	MIT License

	Copyright (c) 2019 Oleksiy Ryabchun

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

precision mediump float;

uniform sampler2D tex01;
uniform sampler2D tex02;
uniform vec2 texSize;

in vec2 fTexCoord;

out vec4 fragColor;

#define BLEND_NONE 0
#define BLEND_NORMAL 1
#define BLEND_DOMINANT 2
#define LUMINANCE_WEIGHT 1.0
#define EQUAL_COLOR_TOLERANCE 30.0/255.0
#define STEEP_DIRECTION_THRESHOLD 2.2
#define DOMINANT_DIRECTION_THRESHOLD 3.6
#define M_PI 3.1415926535897932384626433832795

const float M_PI_QUAD = 1.0 - M_PI / 4.0;
const float five_sixths = 5.0 / 6.0;

float reduce(const vec3 color) {
	return dot(color, vec3(65536.0, 256.0, 1.0));
}

float DistYCbCr(const vec3 pixA, const vec3 pixB) {
	const vec3 w = vec3(0.2627, 0.6780, 0.0593);
	const float scaleB = 0.5 / (1.0 - w.b);
	const float scaleR = 0.5 / (1.0 - w.r);
	vec3 diff = pixA - pixB;
	float Y = dot(diff, w);
	float Cb = scaleB * (diff.b - Y);
	float Cr = scaleR * (diff.r - Y);
	
	return sqrt( ((LUMINANCE_WEIGHT * Y) * (LUMINANCE_WEIGHT * Y)) + (Cb * Cb) + (Cr * Cr) );
}

bool IsPixEqual(const vec3 pixA, const vec3 pixB) {
	return (DistYCbCr(pixA, pixB) < EQUAL_COLOR_TOLERANCE);
}

bool IsBlendingNeeded(const ivec4 blend) {
	return any(notEqual(blend, ivec4(BLEND_NONE)));
}

void main() {
	if (texture(tex01, fTexCoord) == texture(tex02, fTexCoord))
		discard;

	vec2 texel = floor(fTexCoord * texSize) + 0.5;

	#define TEX(x, y) texture(tex01, (texel + vec2(x, y)) / texSize).rgb

	vec3 src[25];
	src[21] = TEX(-1.0, -2.0);
	src[22] = TEX( 0.0, -2.0);
	src[23] = TEX( 1.0, -2.0);
	src[ 6] = TEX(-1.0, -1.0);
	src[ 7] = TEX( 0.0, -1.0);
	src[ 8] = TEX( 1.0, -1.0);
	src[ 5] = TEX(-1.0,  0.0);
	src[ 0] = TEX( 0.0,  0.0);
	src[ 1] = TEX( 1.0,  0.0);
	src[ 4] = TEX(-1.0,  1.0);
	src[ 3] = TEX( 0.0,  1.0);
	src[ 2] = TEX( 1.0,  1.0);
	src[15] = TEX(-1.0,  2.0);
	src[14] = TEX( 0.0,  2.0);
	src[13] = TEX( 1.0,  2.0);
	src[19] = TEX(-2.0, -1.0);
	src[18] = TEX(-2.0,  0.0);
	src[17] = TEX(-2.0,  1.0);
	src[ 9] = TEX( 2.0, -1.0);
	src[10] = TEX( 2.0,  0.0);
	src[11] = TEX( 2.0,  1.0);
	
	float v[9];
	v[0] = reduce(src[0]);
	v[1] = reduce(src[1]);
	v[2] = reduce(src[2]);
	v[3] = reduce(src[3]);
	v[4] = reduce(src[4]);
	v[5] = reduce(src[5]);
	v[6] = reduce(src[6]);
	v[7] = reduce(src[7]);
	v[8] = reduce(src[8]);
	
	ivec4 blendResult = ivec4(BLEND_NONE);
	if (!((v[0] == v[1] && v[3] == v[2]) || (v[0] == v[3] && v[1] == v[2])))
	{
		float dist_03_01 = DistYCbCr(src[ 4], src[ 0]) + DistYCbCr(src[ 0], src[ 8]) + DistYCbCr(src[14], src[ 2]) + DistYCbCr(src[ 2], src[10]) + (4.0 * DistYCbCr(src[ 3], src[ 1]));
		float dist_00_02 = DistYCbCr(src[ 5], src[ 3]) + DistYCbCr(src[ 3], src[13]) + DistYCbCr(src[ 7], src[ 1]) + DistYCbCr(src[ 1], src[11]) + (4.0 * DistYCbCr(src[ 0], src[ 2]));
		bool dominantGradient = (DOMINANT_DIRECTION_THRESHOLD * dist_03_01) < dist_00_02;
		blendResult.z = ((dist_03_01 < dist_00_02) && (v[0] != v[1]) && (v[0] != v[3])) ? ((dominantGradient) ? BLEND_DOMINANT : BLEND_NORMAL) : BLEND_NONE;
	}

	if (!((v[5] == v[0] && v[4] == v[3]) || (v[5] == v[4] && v[0] == v[3])))
	{
		float dist_04_00 = DistYCbCr(src[17], src[ 5]) + DistYCbCr(src[ 5], src[ 7]) + DistYCbCr(src[15], src[ 3]) + DistYCbCr(src[ 3], src[ 1]) + (4.0 * DistYCbCr(src[ 4], src[ 0]));
		float dist_05_03 = DistYCbCr(src[18], src[ 4]) + DistYCbCr(src[ 4], src[14]) + DistYCbCr(src[ 6], src[ 0]) + DistYCbCr(src[ 0], src[ 2]) + (4.0 * DistYCbCr(src[ 5], src[ 3]));
		bool dominantGradient = (DOMINANT_DIRECTION_THRESHOLD * dist_05_03) < dist_04_00;
		blendResult.w = ((dist_04_00 > dist_05_03) && (v[0] != v[5]) && (v[0] != v[3])) ? ((dominantGradient) ? BLEND_DOMINANT : BLEND_NORMAL) : BLEND_NONE;
	}
	
	if (!((v[7] == v[8] && v[0] == v[1]) || (v[7] == v[0] && v[8] == v[1])))
	{
		float dist_00_08 = DistYCbCr(src[ 5], src[ 7]) + DistYCbCr(src[ 7], src[23]) + DistYCbCr(src[ 3], src[ 1]) + DistYCbCr(src[ 1], src[ 9]) + (4.0 * DistYCbCr(src[ 0], src[ 8]));
		float dist_07_01 = DistYCbCr(src[ 6], src[ 0]) + DistYCbCr(src[ 0], src[ 2]) + DistYCbCr(src[22], src[ 8]) + DistYCbCr(src[ 8], src[10]) + (4.0 * DistYCbCr(src[ 7], src[ 1]));
		bool dominantGradient = (DOMINANT_DIRECTION_THRESHOLD * dist_07_01) < dist_00_08;
		blendResult.y = ((dist_00_08 > dist_07_01) && (v[0] != v[7]) && (v[0] != v[1])) ? ((dominantGradient) ? BLEND_DOMINANT : BLEND_NORMAL) : BLEND_NONE;
	}
	
	if (!((v[6] == v[7] && v[5] == v[0]) || (v[6] == v[5] && v[7] == v[0])))
	{
		float dist_05_07 = DistYCbCr(src[18], src[ 6]) + DistYCbCr(src[ 6], src[22]) + DistYCbCr(src[ 4], src[ 0]) + DistYCbCr(src[ 0], src[ 8]) + (4.0 * DistYCbCr(src[ 5], src[ 7]));
		float dist_06_00 = DistYCbCr(src[19], src[ 5]) + DistYCbCr(src[ 5], src[ 3]) + DistYCbCr(src[21], src[ 7]) + DistYCbCr(src[ 7], src[ 1]) + (4.0 * DistYCbCr(src[ 6], src[ 0]));
		bool dominantGradient = (DOMINANT_DIRECTION_THRESHOLD * dist_05_07) < dist_06_00;
		blendResult.x = ((dist_05_07 < dist_06_00) && (v[0] != v[5]) && (v[0] != v[7])) ? ((dominantGradient) ? BLEND_DOMINANT : BLEND_NORMAL) : BLEND_NONE;
	}
	
	vec3 dst[4];
	dst[ 0] = src[0];
	dst[ 1] = src[0];
	dst[ 2] = src[0];
	dst[ 3] = src[0];
	
	if (IsBlendingNeeded(blendResult) == true)
	{
		float dist_01_04 = DistYCbCr(src[1], src[4]);
		float dist_03_08 = DistYCbCr(src[3], src[8]);
		bool haveShallowLine = (STEEP_DIRECTION_THRESHOLD * dist_01_04 <= dist_03_08) && (v[0] != v[4]) && (v[5] != v[4]);
		bool haveSteepLine   = (STEEP_DIRECTION_THRESHOLD * dist_03_08 <= dist_01_04) && (v[0] != v[8]) && (v[7] != v[8]);
		bool needBlend = (blendResult.z != BLEND_NONE);
		bool doLineBlend = (  blendResult.z >= BLEND_DOMINANT ||
						   ((blendResult.y != BLEND_NONE && !IsPixEqual(src[0], src[4])) ||
							 (blendResult.w != BLEND_NONE && !IsPixEqual(src[0], src[8])) ||
							 (IsPixEqual(src[4], src[3]) && IsPixEqual(src[3], src[2]) && IsPixEqual(src[2], src[1]) && IsPixEqual(src[1], src[8]) && IsPixEqual(src[0], src[2]) == false) ) == false );
		
		vec3 blendPix = ( DistYCbCr(src[0], src[1]) <= DistYCbCr(src[0], src[3]) ) ? src[1] : src[3];
		dst[1] = mix(dst[1], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 0.25 : 0.00);
		dst[2] = mix(dst[2], blendPix, (needBlend) ? ((doLineBlend) ? ((haveShallowLine) ? ((haveSteepLine) ? five_sixths : 0.75) : ((haveSteepLine) ? 0.75 : 0.50)) : M_PI_QUAD) : 0.00);
		dst[3] = mix(dst[3], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 0.25 : 0.00);
		
		dist_01_04 = DistYCbCr(src[7], src[2]);
		dist_03_08 = DistYCbCr(src[1], src[6]);
		haveShallowLine = (STEEP_DIRECTION_THRESHOLD * dist_01_04 <= dist_03_08) && (v[0] != v[2]) && (v[3] != v[2]);
		haveSteepLine   = (STEEP_DIRECTION_THRESHOLD * dist_03_08 <= dist_01_04) && (v[0] != v[6]) && (v[5] != v[6]);
		needBlend = (blendResult.y != BLEND_NONE);
		doLineBlend = (  blendResult.y >= BLEND_DOMINANT ||
					  !((blendResult.x != BLEND_NONE && !IsPixEqual(src[0], src[2])) ||
						(blendResult.z != BLEND_NONE && !IsPixEqual(src[0], src[6])) ||
						(IsPixEqual(src[2], src[1]) && IsPixEqual(src[1], src[8]) && IsPixEqual(src[8], src[7]) && IsPixEqual(src[7], src[6]) && !IsPixEqual(src[0], src[8])) ) );
		
		blendPix = ( DistYCbCr(src[0], src[7]) <= DistYCbCr(src[0], src[1]) ) ? src[7] : src[1];
		dst[0] = mix(dst[0], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 0.25 : 0.00);
		dst[1] = mix(dst[1], blendPix, (needBlend) ? ((doLineBlend) ? ((haveShallowLine) ? ((haveSteepLine) ? five_sixths : 0.75) : ((haveSteepLine) ? 0.75 : 0.50)) : M_PI_QUAD) : 0.00);
		dst[2] = mix(dst[2], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 0.25 : 0.00);
		
		dist_01_04 = DistYCbCr(src[5], src[8]);
		dist_03_08 = DistYCbCr(src[7], src[4]);
		haveShallowLine = (STEEP_DIRECTION_THRESHOLD * dist_01_04 <= dist_03_08) && (v[0] != v[8]) && (v[1] != v[8]);
		haveSteepLine   = (STEEP_DIRECTION_THRESHOLD * dist_03_08 <= dist_01_04) && (v[0] != v[4]) && (v[3] != v[4]);
		needBlend = (blendResult.x != BLEND_NONE);
		doLineBlend = (  blendResult.x >= BLEND_DOMINANT ||
					  !((blendResult.w != BLEND_NONE && !IsPixEqual(src[0], src[8])) ||
						(blendResult.y != BLEND_NONE && !IsPixEqual(src[0], src[4])) ||
						(IsPixEqual(src[8], src[7]) && IsPixEqual(src[7], src[6]) && IsPixEqual(src[6], src[5]) && IsPixEqual(src[5], src[4]) && !IsPixEqual(src[0], src[6])) ) );
		
		blendPix = ( DistYCbCr(src[0], src[5]) <= DistYCbCr(src[0], src[7]) ) ? src[5] : src[7];
		dst[3] = mix(dst[3], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 0.25 : 0.00);
		dst[0] = mix(dst[0], blendPix, (needBlend) ? ((doLineBlend) ? ((haveShallowLine) ? ((haveSteepLine) ? five_sixths : 0.75) : ((haveSteepLine) ? 0.75 : 0.50)) : M_PI_QUAD) : 0.00);
		dst[1] = mix(dst[1], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 0.25 : 0.00);
		
		dist_01_04 = DistYCbCr(src[3], src[6]);
		dist_03_08 = DistYCbCr(src[5], src[2]);
		haveShallowLine = (STEEP_DIRECTION_THRESHOLD * dist_01_04 <= dist_03_08) && (v[0] != v[6]) && (v[7] != v[6]);
		haveSteepLine   = (STEEP_DIRECTION_THRESHOLD * dist_03_08 <= dist_01_04) && (v[0] != v[2]) && (v[1] != v[2]);
		needBlend = (blendResult.w != BLEND_NONE);
		doLineBlend = (  blendResult.w >= BLEND_DOMINANT ||
					  !((blendResult.z != BLEND_NONE && !IsPixEqual(src[0], src[6])) ||
						(blendResult.x != BLEND_NONE && !IsPixEqual(src[0], src[2])) ||
						(IsPixEqual(src[6], src[5]) && IsPixEqual(src[5], src[4]) && IsPixEqual(src[4], src[3]) && IsPixEqual(src[3], src[2]) && !IsPixEqual(src[0], src[4])) ) );
		
		blendPix = ( DistYCbCr(src[0], src[3]) <= DistYCbCr(src[0], src[5]) ) ? src[3] : src[5];
		dst[2] = mix(dst[2], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 0.25 : 0.00);
		dst[3] = mix(dst[3], blendPix, (needBlend) ? ((doLineBlend) ? ((haveShallowLine) ? ((haveSteepLine) ? five_sixths : 0.75) : ((haveSteepLine) ? 0.75 : 0.50)) : M_PI_QUAD) : 0.00);
		dst[0] = mix(dst[0], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 0.25 : 0.00);
	}
	
	vec2 f = fract(fTexCoord * texSize);
	vec3 res = mix( mix(dst[0], dst[1], step(0.50, f.x)),
						mix(dst[3], dst[2], step(0.50, f.x)), step(0.50, f.y) );
								 
	fragColor = vec4(res, 1.0);
}
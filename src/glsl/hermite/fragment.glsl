/*
	Cubic fragment shader
	based on libretro Cubic shader
	https://github.com/libretro/glsl-shaders/tree/master/cubic/shaders

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

uniform sampler2D tex01;
uniform vec2 texSize;

#if __VERSION__ >= 130
	#define COMPAT_IN in
	#define COMPAT_TEXTURE texture
	out vec4 FRAG_COLOR;
#else
	#define COMPAT_IN varying 
	#define COMPAT_TEXTURE texture2D
	#define FRAG_COLOR gl_FragColor
#endif

COMPAT_IN vec2 fTex;

vec4 hermite(sampler2D tex, vec2 coord)
{
	vec2 uv = coord * texSize - 0.5;
	vec2 texel = floor(uv) + 0.5;
	vec2 t = fract(uv);

	uv = texel + t * t * t * (t * (t * 6.0 - 15.0) + 10.0);

	return COMPAT_TEXTURE(tex, uv / texSize);
}

void main() {
	FRAG_COLOR = hermite(tex01, fTex);
}
/*=====================================================================*
 *                   Copyright (C) 2012 Paul Mineiro                   *
 * All rights reserved.                                                *
 *                                                                     *
 * Redistribution and use in source and binary forms, with             *
 * or without modification, are permitted provided that the            *
 * following conditions are met:                                       *
 *                                                                     *
 *     * Redistributions of source code must retain the                *
 *     above copyright notice, this list of conditions and             *
 *     the following disclaimer.                                       *
 *                                                                     *
 *     * Redistributions in binary form must reproduce the             *
 *     above copyright notice, this list of conditions and             *
 *     the following disclaimer in the documentation and/or            *
 *     other materials provided with the distribution.                 *
 *                                                                     *
 *     * Neither the name of Paul Mineiro nor the names                *
 *     of other contributors may be used to endorse or promote         *
 *     products derived from this software without specific            *
 *     prior written permission.                                       *
 *                                                                     *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND              *
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,         *
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES               *
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE             *
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER               *
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,                 *
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES            *
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE           *
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR                *
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF          *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT           *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY              *
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE             *
 * POSSIBILITY OF SUCH DAMAGE.                                         *
 *                                                                     *
 * Contact: Paul Mineiro <paul@mineiro.com>                            *
 *=====================================================================*/

#ifndef FASTAPPROX_H_
#define FASTAPPROX_H_
#include <stdint.h>

// Underflow of exponential is common practice in numerical routines,
// so handle it here.

namespace fastapprox
{

static inline float fastpow2(float p)
{
    float offset = (p < 0) ? 1.0f : 0.0f;
    float clipp = (p < -126) ? -126.0f : p;
    auto w = static_cast<int>(clipp);
    float z = clipp - w + offset;
    union
    {
        uint32_t i;
        float f;
    } v = {static_cast<uint32_t>((1 << 23) * (clipp + 121.2740575f
                                              + 27.7280233f / (4.84252568f - z)
                                              - 1.49012907f * z))};

    return v.f;
}

static inline float fastexp(float p)
{
    return fastpow2(1.442695040f * p);
}

static inline float fasterpow2(float p)
{
    float clipp = (p < -126) ? -126.0f : p;
    union
    {
        uint32_t i;
        float f;
    } v = {static_cast<uint32_t>((1 << 23) * (clipp + 126.94269504f))};
    return v.f;
}

static inline float fasterexp(float p)
{
    return fasterpow2(1.442695040f * p);
}

static inline float fastlog2(float x)
{
    union
    {
        float f;
        uint32_t i;
    } vx = {x};
    union
    {
        uint32_t i;
        float f;
    } mx = {(vx.i & 0x007FFFFF) | 0x3f000000};
    float y = vx.i;
    y *= 1.1920928955078125e-7f;

    return y - 124.22551499f - 1.498030302f * mx.f
           - 1.72587999f / (0.3520887068f + mx.f);
}

static inline float fastlog(float x)
{
    return 0.69314718f * fastlog2(x);
}

static inline float fasterlog2(float x)
{
    union
    {
        float f;
        uint32_t i;
    } vx = {x};
    float y = vx.i;
    y *= 1.1920928955078125e-7f;
    return y - 126.94269504f;
}

static inline float fasterlog(float x)
{
    union
    {
        float f;
        uint32_t i;
    } vx = {x};
    float y = vx.i;
    y *= 8.2629582881927490e-8f;
    return y - 87.989971088f;
}

} // namespace fastapprox
#endif

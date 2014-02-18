/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h"

#include "vpvl2/mvd/LightKeyframe.h"

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct LightKeyframeChunk {
    LightKeyframeChunk() {}
    uint64_t timeIndex;
    float position[3];
    float color[3];
    uint8_t enabled;
};

#pragma pack(pop)

LightKeyframe::LightKeyframe(const Motion *motionRef)
    : VPVL2_KEYFRAME_INITIALIZE_FIELDS(),
      m_ptr(0),
      m_motionRef(motionRef),
      m_color(kZeroV3),
      m_direction(kZeroV3)
{
}

LightKeyframe::~LightKeyframe()
{
    VPVL2_KEYFRAME_DESTROY_FIELDS()
    delete m_ptr;
    m_ptr = 0;
    m_color.setZero();
    m_direction.setZero();
}

size_t LightKeyframe::size()
{
    static const LightKeyframeChunk keyframe;
    return sizeof(keyframe);
}

bool LightKeyframe::preparse(uint8_t *&ptr, size_t &rest, size_t reserved, Motion::DataInfo & /* info */)
{
    if (!internal::validateSize(ptr, size(), rest)) {
        VPVL2_LOG(WARNING, "Invalid size of MVD light keyframe detected: ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
        return false;
    }
    if (!internal::validateSize(ptr, reserved, rest)) {
        VPVL2_LOG(WARNING, "Invalid size of MVD reserved light keyframe detected: ptr=" << static_cast<const void *>(ptr) << " size=" << reserved << " rest=" << rest);
        return false;
    }
    return true;
}

void LightKeyframe::read(const uint8_t *data)
{
    LightKeyframeChunk chunk;
    internal::getData(data, chunk);
    internal::setPosition(chunk.position, m_direction);
    internal::setPositionRaw(chunk.color, m_color);
    setTimeIndex(TimeIndex(chunk.timeIndex));
    setEnable(chunk.enabled != 0);
}

void LightKeyframe::write(uint8_t *data) const
{
    LightKeyframeChunk chunk;
    internal::getPosition(direction(), chunk.position);
    internal::getPositionRaw(color(), chunk.color);
    chunk.timeIndex = uint64_t(timeIndex());
    chunk.enabled = isEnabled();
    internal::writeBytes(&chunk, sizeof(chunk), data);
}

size_t LightKeyframe::estimateSize() const
{
    return size();
}

ILightKeyframe *LightKeyframe::clone() const
{
    LightKeyframe *keyframe = m_ptr = new LightKeyframe(m_motionRef);
    keyframe->setTimeIndex(m_timeIndex);
    keyframe->setLayerIndex(m_layerIndex);
    keyframe->setColor(m_color);
    keyframe->setDirection(m_direction);
    keyframe->setEnable(m_enabled);
    m_ptr = 0;
    return keyframe;
}

Vector3 LightKeyframe::color() const
{
    return m_color;
}

Vector3 LightKeyframe::direction() const
{
    return m_direction;
}

bool LightKeyframe::isEnabled() const
{
    return m_enabled;
}

void LightKeyframe::setColor(const Vector3 &value)
{
    m_color = value;
}

void LightKeyframe::setDirection(const Vector3 &value)
{
    m_direction = value;
}

void LightKeyframe::setEnable(bool value)
{
    m_enabled = value;
}

void LightKeyframe::setName(const IString * /* value */)
{
}

IKeyframe::Type LightKeyframe::type() const
{
    return kLightKeyframe;
}

const Motion *LightKeyframe::parentMotionRef() const
{
    return m_motionRef;
}

} /* namespace mvd */
} /* namespace vpvl2 */

/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2013  hkrn                                    */
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

#include "vpvl2/vmd/MorphKeyframe.h"

namespace vpvl2
{
namespace vmd
{

#pragma pack(push, 1)

struct MorphKeyframeChunk
{
    uint8_t name[MorphKeyframe::kNameSize];
    int32_t timeIndex;
    float32_t weight;
};

#pragma pack(pop)

size_t MorphKeyframe::strideSize()
{
    return sizeof(MorphKeyframeChunk);
}

MorphKeyframe::MorphKeyframe(IEncoding *encoding)
    : VPVL2_KEYFRAME_INITIALIZE_FIELDS(),
      m_encodingRef(encoding),
      m_weight(0.0f)
{
}

MorphKeyframe::~MorphKeyframe()
{
    VPVL2_KEYFRAME_DESTROY_FIELDS()
}

void MorphKeyframe::read(const uint8_t *data)
{
    MorphKeyframeChunk chunk;
    internal::getData(data, chunk);
    internal::setStringDirect(m_encodingRef->toString(chunk.name, IString::kShiftJIS, sizeof(chunk.name)), m_namePtr);
    setTimeIndex(static_cast<const TimeIndex>(chunk.timeIndex));
    setWeight(chunk.weight);
}

void MorphKeyframe::write(uint8_t *data) const
{
    MorphKeyframeChunk chunk;
    uint8_t *name = m_encodingRef->toByteArray(m_namePtr, IString::kShiftJIS);
    internal::copyBytes(chunk.name, name, sizeof(chunk.name));
    m_encodingRef->disposeByteArray(name);
    chunk.timeIndex = static_cast<int>(m_timeIndex);
    chunk.weight = float(m_weight);
    internal::copyBytes(data, reinterpret_cast<const uint8_t *>(&chunk), sizeof(chunk));
}

size_t MorphKeyframe::estimateSize() const
{
    return strideSize();
}

IMorphKeyframe *MorphKeyframe::clone() const
{
    MorphKeyframe *keyframe = new MorphKeyframe(m_encodingRef);
    keyframe->setName(m_namePtr);
    keyframe->setTimeIndex(m_timeIndex);
    keyframe->setWeight(m_weight);
    return keyframe;
}

void MorphKeyframe::setName(const IString *value)
{
    internal::setString(value, m_namePtr);
}

void MorphKeyframe::setWeight(const IMorph::WeightPrecision &value)
{
    m_weight = value;
}

} /* namespace vmd */
} /* namespace vpvl2 */

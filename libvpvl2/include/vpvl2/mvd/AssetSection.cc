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

#include "vpvl2/mvd/AssetKeyframe.h"
#include "vpvl2/mvd/AssetSection.h"

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct AssetSectionHeader {
    int32_t reserved;
    int32_t sizeOfKeyframe;
    int32_t countOfKeyframes;
    int32_t reserved2;
};

#pragma pack(pop)

AssetSection::AssetSection(const Motion *motionRef)
    : BaseSection(motionRef)
{
}

AssetSection::~AssetSection()
{
}

bool AssetSection::preparse(uint8_t *&ptr, size_t &rest, Motion::DataInfo &info)
{
    AssetSectionHeader header;
    if (!internal::validateSize(ptr, sizeof(header), rest)) {
        return false;
    }
    internal::getData(ptr - sizeof(header), header);
    if (!internal::validateSize(ptr, header.reserved2, rest)) {
        return false;
    }
    const int32_t nkeyframes = header.countOfKeyframes;
    const size_t reserved = header.sizeOfKeyframe - AssetKeyframe::size();
    for (int32_t i = 0; i < nkeyframes; i++) {
        if (!AssetKeyframe::preparse(ptr, rest, reserved, info)) {
            return false;
        }
    }
    return true;
}

void AssetSection::read(const uint8_t * /* data */)
{
}

void AssetSection::seek(const IKeyframe::TimeIndex &timeIndex)
{
    saveCurrentTimeIndex(timeIndex);
}

void AssetSection::write(uint8_t * /* data */) const
{
}

size_t AssetSection::estimateSize() const
{
    return 0;
}

size_t AssetSection::countKeyframes() const
{
    return 0;
}

void AssetSection::addKeyframe(IKeyframe * /* keyframe */)
{
}

void AssetSection::deleteKeyframe(IKeyframe *&keyframe)
{
    delete keyframe;
    keyframe = 0;
}

void AssetSection::getKeyframes(const IKeyframe::TimeIndex & /* timeIndex */,
                                const IKeyframe::LayerIndex & /* layerIndex */,
                                Array<IKeyframe *> & /* keyframes */) const
{
}

void AssetSection::getAllKeyframes(Array<IKeyframe *> & /* value */) const
{
}

void AssetSection::setAllKeyframes(const Array<IKeyframe *> & /* value */)
{
}

} /* namespace mvd */
} /* namespace vpvl2 */

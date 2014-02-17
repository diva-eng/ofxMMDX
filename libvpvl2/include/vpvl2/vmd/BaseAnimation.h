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

#pragma once
#ifndef VPVL2_VMD_BASEANIMATION_H_
#define VPVL2_VMD_BASEANIMATION_H_

#include "vpvl2/Common.h"
#include "vpvl2/IKeyframe.h"

namespace vpvl2
{
namespace vmd
{

/**
 * @file
 * @author Nagoya Institute of Technology Department of Computer Science
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * The basic class inherited by BoneAnimation, CameraAnimation and FaceAnimation class.
 */

class VPVL2_API BaseAnimation
{
public:

    BaseAnimation();
    virtual ~BaseAnimation();

    virtual void read(const uint8_t *data, int size) = 0;
    virtual void seek(const IKeyframe::TimeIndex &timeIndexAt) = 0;
    void advance(const IKeyframe::TimeIndex &deltaTimeIndex);
    void rewind(const IKeyframe::TimeIndex &target, const IKeyframe::TimeIndex &deltaTimeIndex);
    void reset();
    void addKeyframe(IKeyframe *keyframe);
    void deleteKeyframe(IKeyframe *&keyframe);
    void getKeyframes(const IKeyframe::TimeIndex &timeIndex, Array<IKeyframe *> &keyframes) const;
    void getAllKeyframes(Array<IKeyframe *> &value) const;
    void setAllKeyframes(const Array<IKeyframe *> &value, IKeyframe::Type type);

    int countKeyframes() const { return m_keyframes.count(); }
    IKeyframe::TimeIndex previousTimeIndex() const { return m_previousTimeIndex; }
    IKeyframe::TimeIndex currentTimeIndex() const { return m_currentTimeIndex; }
    IKeyframe::TimeIndex duration() const { return m_durationTimeIndex; }

protected:
    template<typename T>
    static int findKeyframeIndex(const IKeyframe::TimeIndex &key, const Array<T *> &keyframes) {
        int min = 0, max = keyframes.count() - 1;
        while (min < max) {
            int mid = (min + max) / 2;
            const T *keyframe = keyframes[mid];
            if (keyframe->timeIndex() < key)
                min = mid + 1;
            else
                max = mid;
        }
        if (min == max) {
            const IKeyframe::TimeIndex &timeIndex = keyframes[min]->timeIndex();
            if (timeIndex == key)
                return min;
        }
        return -1;
    }

    PointerArray<IKeyframe> m_keyframes;
    int m_lastTimeIndex;
    IKeyframe::TimeIndex m_durationTimeIndex;
    IKeyframe::TimeIndex m_currentTimeIndex;
    IKeyframe::TimeIndex m_previousTimeIndex;

    VPVL2_DISABLE_COPY_AND_ASSIGN(BaseAnimation)
};

}
}

#endif


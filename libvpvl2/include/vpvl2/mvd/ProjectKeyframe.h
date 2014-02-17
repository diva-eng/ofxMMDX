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

#pragma once
#ifndef VPVL2_MVD_PROJECTKEYFRAME_H_
#define VPVL2_MVD_PROJECTKEYFRAME_H_

#include "vpvl2/IProjectKeyframe.h"
#include "vpvl2/mvd/Motion.h"
#include "vpvl2/internal/Keyframe.h"

namespace vpvl2
{
class IEncoding;

namespace mvd
{

class VPVL2_API ProjectKeyframe : public IProjectKeyframe
{
public:
    ProjectKeyframe(const Motion *motionRef);
    ~ProjectKeyframe();

    static size_t size();
    static bool preparse(uint8_t *&ptr, size_t &rest, size_t reserved, Motion::DataInfo &info);

    void read(const uint8_t *data);
    void write(uint8_t *data) const;
    size_t estimateSize() const;
    IProjectKeyframe *clone() const;
    void setName(const IString *value);
    Type type() const;
    const Motion *parentMotionRef() const;

    VPVL2_KEYFRAME_DEFINE_METHODS()
    float32_t gravityFactor() const;
    Vector3 gravityDirection() const;
    int shadowMode() const;
    float32_t shadowDistance() const;
    float32_t shadowDepth() const;
    void setGravityFactor(float value);
    void setGravityDirection(const Vector3 &value);
    void setShadowMode(int value);
    void setShadowDistance(float value);
    void setShadowDepth(float value);

private:
    VPVL2_KEYFRAME_DEFINE_FIELDS()
    mutable ProjectKeyframe *m_ptr;
    const Motion *m_motionRef;
    Vector3 m_gravityDirection;
    float32_t m_gravityFactor;
    float32_t m_shadowDistance;
    float32_t m_shadowDepth;
    int m_shadowMode;

    VPVL2_DISABLE_COPY_AND_ASSIGN(ProjectKeyframe)
};

} /* namespace mvd */
} /* namespace vpvl2 */

#endif


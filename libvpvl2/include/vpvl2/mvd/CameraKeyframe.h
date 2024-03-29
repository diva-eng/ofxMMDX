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
#ifndef VPVL2_MVD_CAMERAKEYFRAME_H_
#define VPVL2_MVD_CAMERAKEYFRAME_H_

#include "vpvl2/ICameraKeyframe.h"
#include "vpvl2/mvd/Motion.h"
#include "vpvl2/internal/Keyframe.h"

namespace vpvl2
{
class IEncoding;

namespace mvd
{

class VPVL2_API CameraKeyframe : public ICameraKeyframe
{
public:
    CameraKeyframe(const Motion *motionRef);
    ~CameraKeyframe();

    static size_t size();
    static bool preparse(uint8_t *&ptr, size_t &rest, size_t reserved, Motion::DataInfo &info);
    static int interpolationTableSize();

    void read(const uint8_t *data);
    void write(uint8_t *data) const;
    size_t estimateSize() const;
    ICameraKeyframe *clone() const;
    void setDefaultInterpolationParameter();
    void setInterpolationParameter(InterpolationType type, const QuadWord &value);
    void getInterpolationParameter(InterpolationType type, QuadWord &value) const;
    Vector3 lookAt() const;
    Vector3 angle() const;
    Scalar distance() const;
    Scalar fov() const;
    bool isPerspective() const;
    void setLookAt(const Vector3 &value);
    void setAngle(const Vector3 &value);
    void setDistance(const Scalar &value);
    void setFov(const Scalar &value);
    void setPerspective(bool value);
    void setName(const IString *value);
    Type type() const;

    VPVL2_KEYFRAME_DEFINE_METHODS()
    const Motion *parentMotionRef() const;
    const internal::InterpolationTable &tableForPosition() const;
    const internal::InterpolationTable &tableForRotation() const;
    const internal::InterpolationTable &tableForFov() const;
    const internal::InterpolationTable &tableForDistance() const;

private:
    VPVL2_KEYFRAME_DEFINE_FIELDS()
    mutable CameraKeyframe *m_ptr;
    const Motion *m_motionRef;
    Vector3 m_position;
    Vector3 m_angle;
    float m_distance;
    float m_fov;
    bool m_perspective;
    internal::InterpolationTable m_interpolationPosition;
    internal::InterpolationTable m_interpolationRotation;
    internal::InterpolationTable m_interpolationFov;
    internal::InterpolationTable m_interpolationDistance;

    VPVL2_DISABLE_COPY_AND_ASSIGN(CameraKeyframe)
};

} /* namespace mvd */
} /* namespace vpvl2 */

#endif


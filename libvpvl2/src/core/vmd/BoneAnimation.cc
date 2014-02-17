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
#include "vpvl2/internal/MotionHelper.h"

#include "vpvl2/IBoneKeyframe.h"
#include "vpvl2/vmd/BoneAnimation.h"
#include "vpvl2/vmd/BoneKeyframe.h"

namespace vpvl2
{
namespace vmd
{

struct BoneAnimation::PrivateContext {
    IBone *bone;
    Array<BoneKeyframe *> keyframes;
    Vector3 position;
    Quaternion rotation;
    int lastIndex;

    bool isNull() const {
        if (keyframes.count() == 1) {
            const IBoneKeyframe *keyframe = keyframes[0];
            return keyframe->localTranslation() == kZeroV3 &&
                    keyframe->localRotation() == Quaternion::getIdentity();
        }
        return false;
    }
};

IKeyframe::SmoothPrecision BoneAnimation::weightValue(const BoneKeyframe *keyframe,
                                                      const IKeyframe::SmoothPrecision &w,
                                                      int at)
{
    const uint16_t index = static_cast<int16_t>(w * BoneKeyframe::kTableSize);
    const IKeyframe::SmoothPrecision *v = keyframe->interpolationTable()[at];
    return v[index] + (v[index + 1] - v[index]) * (w * BoneKeyframe::kTableSize - index);
}

void BoneAnimation::lerpVector3(const BoneKeyframe *keyframe,
                                const Vector3 &from,
                                const Vector3 &to,
                                const IKeyframe::SmoothPrecision &w,
                                int at,
                                IKeyframe::SmoothPrecision &value)
{
    const IKeyframe::SmoothPrecision &valueFrom = from[at];
    const IKeyframe::SmoothPrecision &valueTo = to[at];
    if (keyframe->linear()[at]) {
        value = internal::MotionHelper::lerp(valueFrom, valueTo, w);
    }
    else {
        const IKeyframe::SmoothPrecision &w2 = weightValue(keyframe, w, at);
        value = internal::MotionHelper::lerp(valueFrom, valueTo, w2);
    }
}

BoneAnimation::BoneAnimation(IEncoding *encoding)
    : BaseAnimation(),
      m_encodingRef(encoding),
      m_modelRef(0),
      m_enableNullFrame(false)
{
}

BoneAnimation::~BoneAnimation()
{
    m_name2contexts.releaseAll();
    m_modelRef = 0;
}

void BoneAnimation::read(const uint8_t *data, int size)
{
    uint8_t *ptr = const_cast<uint8_t *>(data);
    m_keyframes.reserve(size);
    for (int i = 0; i < size; i++) {
        BoneKeyframe *keyframe = m_keyframes.append(new BoneKeyframe(m_encodingRef));
        keyframe->read(ptr);
        ptr += keyframe->estimateSize();
    }
}

void BoneAnimation::seek(const IKeyframe::TimeIndex &timeIndexAt)
{
    if (!m_modelRef) {
        return;
    }
    const int ncontexts = m_name2contexts.count();
    for (int i = 0; i < ncontexts; i++) {
        PrivateContext *keyframes = *m_name2contexts.value(i);
        if (m_enableNullFrame && keyframes->isNull()) {
            continue;
        }
        calculateKeyframes(timeIndexAt, keyframes);
        IBone *bone = keyframes->bone;
        bone->setLocalTranslation(keyframes->position);
        bone->setLocalRotation(keyframes->rotation);
    }
    m_previousTimeIndex = m_currentTimeIndex;
    m_currentTimeIndex = timeIndexAt;
}

void BoneAnimation::setParentModelRef(IModel *model)
{
    createPrivateContexts(model);
    m_modelRef = model;
}

BoneKeyframe *BoneAnimation::findKeyframeAt(int i) const
{
    return internal::checkBound(i, 0, m_keyframes.count()) ? reinterpret_cast<BoneKeyframe *>(m_keyframes[i]) : 0;
}

BoneKeyframe *BoneAnimation::findKeyframe(const IKeyframe::TimeIndex &timeIndex, const IString *name) const
{
    if (!name)
        return 0;
    const HashString &key = name->toHashString();
    PrivateContext *const *ptr = m_name2contexts.find(key);
    if (ptr) {
        const PrivateContext *context = *ptr;
        const Array<BoneKeyframe *> &keyframes = context->keyframes;
        int index = findKeyframeIndex(timeIndex, keyframes);
        return index != -1 ? keyframes[index] : 0;
    }
    return 0;
}

void BoneAnimation::createPrivateContexts(IModel *model)
{
    if (!model) {
        return;
    }
    const int nkeyframes = m_keyframes.count();
    m_name2contexts.releaseAll();
    // Build internal node to find by name, not frame index
    for (int i = 0; i < nkeyframes; i++) {
        BoneKeyframe *keyframe = reinterpret_cast<BoneKeyframe *>(m_keyframes.at(i));
        const IString *name = keyframe->name();
        const HashString &key = name->toHashString();
        PrivateContext **ptr = m_name2contexts[key], *context;
        if (ptr) {
            context = *ptr;
            context->keyframes.append(keyframe);
        }
        else if (IBone *bone = model->findBoneRef(name)) {
            PrivateContext *context = m_name2contexts.insert(key, new PrivateContext());
            context->keyframes.append(keyframe);
            context->bone = bone;
            context->lastIndex = 0;
            context->position.setZero();
            context->rotation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
        }
    }
    // Sort frames from each internal nodes by frame index ascend
    const int ncontexts = m_name2contexts.count();
    for (int i = 0; i < ncontexts; i++) {
        PrivateContext *context = *m_name2contexts.value(i);
        Array<BoneKeyframe *> &keyframes = context->keyframes;
        keyframes.sort(internal::MotionHelper::KeyframeTimeIndexPredication());
        btSetMax(m_durationTimeIndex, keyframes[keyframes.count() - 1]->timeIndex());
    }
}

void BoneAnimation::calculateKeyframes(const IKeyframe::TimeIndex &timeIndexAt, PrivateContext *context)
{
    Array<BoneKeyframe *> &keyframes = context->keyframes;
    int fromIndex, toIndex;
    internal::MotionHelper::findKeyframeIndices(timeIndexAt, m_currentTimeIndex, context->lastIndex, fromIndex, toIndex, keyframes);
    const BoneKeyframe *keyframeFrom = keyframes.at(fromIndex), *keyframeTo = keyframes.at(toIndex);
    const IKeyframe::TimeIndex &timeIndexFrom = keyframeFrom->timeIndex(), timeIndexTo = keyframeTo->timeIndex();
    BoneKeyframe *keyframeForInterpolation = const_cast<BoneKeyframe *>(keyframeTo);
    const Vector3 &positionFrom = keyframeFrom->localTranslation();
    const Quaternion &rotationFrom = keyframeFrom->localRotation();
    const Vector3 &positionTo = keyframeTo->localTranslation();
    const Quaternion &rotationTo = keyframeTo->localRotation();
    if (timeIndexFrom != timeIndexTo) {
        if (m_currentTimeIndex <= timeIndexFrom) {
            context->position = positionFrom;
            context->rotation = rotationFrom;
        }
        else if (m_currentTimeIndex >= timeIndexTo) {
            context->position = positionTo;
            context->rotation = rotationTo;
        }
        else {
            const IKeyframe::SmoothPrecision &w = (m_currentTimeIndex - timeIndexFrom) / (timeIndexTo - timeIndexFrom);
            IKeyframe::SmoothPrecision x = 0, y = 0, z = 0;
            lerpVector3(keyframeForInterpolation, positionFrom, positionTo, w, 0, x);
            lerpVector3(keyframeForInterpolation, positionFrom, positionTo, w, 1, y);
            lerpVector3(keyframeForInterpolation, positionFrom, positionTo, w, 2, z);
            context->position.setValue(Scalar(x), Scalar(y), Scalar(z));
            if (keyframeForInterpolation->linear()[3]) {
                context->rotation = rotationFrom.slerp(rotationTo, Scalar(w));
            }
            else {
                const IKeyframe::SmoothPrecision &w2 = weightValue(keyframeForInterpolation, w, 3);
                context->rotation = rotationFrom.slerp(rotationTo, Scalar(w2));
            }
        }
    }
    else {
        context->position = positionFrom;
        context->rotation = rotationFrom;
    }
}

void BoneAnimation::reset()
{
    BaseAnimation::reset();
    const int ncontexts = m_name2contexts.count();
    for (int i = 0; i < ncontexts; i++) {
        PrivateContext *context = *m_name2contexts.value(i);
        context->lastIndex = 0;
    }
}

} /* namespace vmd */
} /* namespace vpvl2 */

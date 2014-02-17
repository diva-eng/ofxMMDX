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
#include "vpvl2/pmd/Bone.h"

namespace vpvl2
{
namespace pmd
{

Bone::Bone(IModel *modelRef, vpvl::Bone *bone, IEncoding *encoding)
    : m_modelRef(modelRef),
      m_encodingRef(encoding),
      m_name(0),
      m_parentBone(0),
      m_effectorBoneRef(0),
      m_childBone(0),
      m_boneRef(bone),
      m_fixedAxis(kZeroV3),
      m_index(-1)
{
    m_name = m_encodingRef->toString(m_boneRef->name(), IString::kShiftJIS, vpvl::Bone::kNameSize);
}

Bone::~Bone()
{
    delete m_name;
    m_name = 0;
    delete m_parentBone;
    m_parentBone = 0;
    delete m_childBone;
    m_childBone = 0;
    m_modelRef = 0;
    m_effectorBoneRef = 0;
    m_encodingRef = 0;
    m_boneRef = 0;
    m_fixedAxis.setZero();
    m_index = -1;
}

const IString *Bone::name() const
{
    return m_name;
}

int Bone::index() const
{
    return m_index;
}

Transform Bone::worldTransform() const
{
    return m_boneRef->localTransform();
}

Vector3 Bone::origin() const
{
    return m_boneRef->originPosition();
}

Vector3 Bone::destinationOrigin() const
{
    return m_boneRef->child()->localTransform().getOrigin();
}

Vector3 Bone::localTranslation() const
{
    return m_boneRef->position();
}

void Bone::getEffectorBones(Array<IBone *> &value) const
{
    const int nlinks = m_IKLinkRefs.count();
    for (int i = 0; i < nlinks; i++) {
        IBone *bone = m_IKLinkRefs[i];
        value.append(bone);
    }
}

Quaternion Bone::localRotation() const
{
    return m_boneRef->rotation();
}

void Bone::setLocalTranslation(const Vector3 &value)
{
    m_boneRef->setPosition(value);
}

void Bone::setLocalRotation(const Quaternion &value)
{
    m_boneRef->setRotation(value);
}

bool Bone::isMovable() const
{
    return m_boneRef->isMovable();
}

bool Bone::isRotateable() const
{
    return m_boneRef->isRotateable();
}

bool Bone::isVisible() const
{
    return m_boneRef->isVisible();
}

bool Bone::isInteractive() const
{
    return m_boneRef->isVisible();
}

bool Bone::hasInverseKinematics() const
{
    return m_effectorBoneRef && m_IKLinkRefs.count() > 0;
}

bool Bone::hasFixedAxes() const
{
    return m_boneRef->type() == vpvl::Bone::kTwist;
}

bool Bone::hasLocalAxes() const
{
    bool finger = m_name->contains(m_encodingRef->stringConstant(IEncoding::kFinger));
    bool arm = m_name->endsWith(m_encodingRef->stringConstant(IEncoding::kArm));
    bool elbow = m_name->endsWith(m_encodingRef->stringConstant(IEncoding::kElbow));
    bool wrist = m_name->endsWith(m_encodingRef->stringConstant(IEncoding::kWrist));
    return finger || arm || elbow || wrist;
}

Vector3 Bone::fixedAxis() const
{
    return m_fixedAxis;
}

void Bone::getLocalAxes(Matrix3x3 &value) const
{
    if (hasLocalAxes()) {
        const Vector3 &axisX = (m_childBone->origin() - m_boneRef->originPosition()).normalized();
        Vector3 tmp1 = axisX;
        if (m_name->startsWith(m_encodingRef->stringConstant(IEncoding::kLeft)))
            tmp1.setY(-axisX.y());
        else
            tmp1.setX(-axisX.x());
        const Vector3 &axisZ = axisX.cross(tmp1).normalized();
        Vector3 tmp2 = axisX;
        tmp2.setZ(-axisZ.z());
        const Vector3 &axisY = tmp2.cross(-axisX).normalized();
        value[0] = axisX;
        value[1] = axisY;
        value[2] = axisZ;
    }
    else {
        value.setIdentity();
    }
}

Transform Bone::localTransform() const
{
    return m_localTransform;
}

void Bone::getLocalTransform(Transform &value) const
{
    m_boneRef->getSkinTransform(value);
}

void Bone::setLocalTransform(const Transform &value)
{
    Transform transform(value);
    m_boneRef->getSkinTransform(transform);
    m_boneRef->setLocalTransform(transform);
}

void Bone::setParentBone(vpvl::Bone *value)
{
    if (value) {
        m_parentBone = new Bone(m_modelRef, const_cast<vpvl::Bone *>(value->parent()), m_encodingRef);
    }
}

void Bone::setChildBone(vpvl::Bone *value)
{
    if (value) {
        const vpvl::Bone *child = value->child();
        m_childBone = new Bone(m_modelRef, const_cast<vpvl::Bone *>(child), m_encodingRef);
        if (hasFixedAxes()) {
            m_fixedAxis = (child->originPosition() - m_boneRef->originPosition()).normalized();
        }
    }
}

void Bone::setIK(vpvl::IK *ik, const Hash<HashPtr, Bone *> &b2b)
{
    vpvl::Bone *targetBone = ik->targetBone();
    if (Bone *const *valuePtr = b2b.find(targetBone)) {
        Bone *value = *valuePtr;
        m_effectorBoneRef = value;
    }
    const vpvl::BoneList &bones = ik->linkedBones();
    const int nbones = bones.count();
    for (int i = 0; i < nbones; i++) {
        vpvl::Bone *bone = bones[i];
        if (Bone *const *valuePtr = b2b.find(bone)) {
            Bone *value = *valuePtr;
            m_IKLinkRefs.append(value);
        }
    }
}

void Bone::setInverseKinematicsEnable(bool value)
{
    m_boneRef->setSimulated(!value);
}

bool Bone::isInverseKinematicsEnabled() const
{
    return !m_boneRef->isSimulated();
}

void Bone::updateLocalTransform()
{
    m_localTransform = m_boneRef->localTransform();
    m_boneRef->getSkinTransform(m_localTransform);
}

void Bone::setIndex(int value)
{
    m_index = value;
}

}
}

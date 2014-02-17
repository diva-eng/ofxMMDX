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
#ifndef VPVL2_INTERNAL_BASERIGIDBODY_H_
#define VPVL2_INTERNAL_BASERIGIDBODY_H_

#include "vpvl2/IRigidBody.h"
#include "LinearMath/btMotionState.h"

class btCollisionShape;
class btDiscreteDynamicsWorld;
class btRigidBody;

namespace vpvl2
{

class IBone;
class IEncoding;
class IString;

namespace internal
{

class VPVL2_API BaseRigidBody : public IRigidBody
{
public:
    class DefaultMotionState : public btMotionState {
    public:
        DefaultMotionState(const Transform &startTransform, const IBone *boneRef);
        ~DefaultMotionState();

        void getWorldTransform(btTransform &worldTransform) const;
        void setWorldTransform(const btTransform &worldTransform);
        void reset();
        const IBone *boneRef() const;
        void setBoneRef(const IBone *value);

    protected:
        const IBone *m_boneRef;
        Transform m_startTransform;
        Transform m_worldTransform;
    };

    class AlignedMotionState : public DefaultMotionState {
    public:
        AlignedMotionState(const Transform &startTransform, const IBone *boneRef);
        ~AlignedMotionState();

        void setWorldTransform(const btTransform &worldTransform);
    };

    class KinematicMotionState : public DefaultMotionState {
    public:
        KinematicMotionState(const Transform &startTransform, const IBone *boneRef);
        ~KinematicMotionState();

        void getWorldTransform(btTransform &worldTransform) const;
        void setWorldTransform(const btTransform & /* worldTransform */);
    };

    BaseRigidBody(IModel *parentModelRef, IEncoding *encodingRef);
    ~BaseRigidBody();

    void syncLocalTransform();
    void joinWorld(void *value);
    void leaveWorld(void *value);
    void setKinematic(bool value, const Vector3 &basePosition);

    virtual const Transform createTransform() const;
    virtual btCollisionShape *createShape() const;
    virtual btRigidBody *createRigidBody(btCollisionShape *shape);

    btRigidBody *body() const { return m_body; }
    void *bodyPtr() const { return m_body; }
    IModel *parentModelRef() const { return m_parentModelRef; }
    IBone *boneRef() const { return m_boneRef; }
    int boneIndex() const { return m_boneIndex; }
    const IString *name() const { return m_name; }
    const IString *englishName() const { return m_englishName; }
    Vector3 size() const { return m_size; }
    Vector3 position() const { return m_position; }
    Vector3 rotation() const { return m_rotation; }
    float32_t mass() const { return m_mass; }
    float32_t linearDamping() const { return m_linearDamping; }
    float32_t angularDamping() const { return m_angularDamping; }
    float32_t restitution() const { return m_restitution; }
    float32_t friction() const { return m_friction; }
    uint16_t groupID() const { return m_groupID; }
    uint16_t collisionGroupMask() const { return m_collisionGroupMask; }
    uint8_t collisionGroupID() const { return m_collisionGroupID; }
    int index() const { return m_index; }

    void setName(const IString *value);
    void setEnglishName(const IString *value);
    void setParentModelRef(IModel *value);
    void setBoneRef(IBone *value);
    void setAngularDamping(float32_t value);
    void setCollisionGroupID(uint8_t value);
    void setCollisionMask(uint16_t value);
    void setFriction(float32_t value);
    void setLinearDamping(float32_t value);
    void setMass(float32_t value);
    void setPosition(const Vector3 &value);
    void setRestitution(float32_t value);
    void setRotation(const Vector3 &value);
    void setShapeType(ShapeType value);
    void setSize(const Vector3 &value);
    void setType(ObjectType value);
    void setIndex(int value);

protected:
    void build(IBone *boneRef, int index);
    virtual DefaultMotionState *createKinematicMotionState() const;
    virtual DefaultMotionState *createDefaultMotionState() const;
    virtual DefaultMotionState *createAlignedMotionState() const;

    btRigidBody *m_body;
    btRigidBody *m_ptr;
    btCollisionShape *m_shape;
    DefaultMotionState *m_motionState;
    DefaultMotionState *m_kinematicMotionState;
    Transform m_worldTransform;
    Transform m_world2LocalTransform;
    IModel *m_parentModelRef;
    IEncoding *m_encodingRef;
    IBone *m_boneRef;
    IString *m_name;
    IString *m_englishName;
    int m_boneIndex;
    Vector3 m_size;
    Vector3 m_position;
    Vector3 m_rotation;
    float m_mass;
    float m_linearDamping;
    float m_angularDamping;
    float m_restitution;
    float m_friction;
    int m_index;
    uint16_t m_groupID;
    uint16_t m_collisionGroupMask;
    uint8_t m_collisionGroupID;
    ShapeType m_shapeType;
    ObjectType m_type;

    VPVL2_DISABLE_COPY_AND_ASSIGN(BaseRigidBody)
};

} /* namespace internal */
} /* namespace vpvl2 */

#endif


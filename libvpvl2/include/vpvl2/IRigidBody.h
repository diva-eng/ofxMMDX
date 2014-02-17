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
#ifndef VPVL2_IRIGIDBODY_H_
#define VPVL2_IRIGIDBODY_H_

#include "vpvl2/Common.h"

namespace vpvl2
{

class IBone;
class IModel;
class IString;

/**
 * モデルの剛体をあらわすインターフェースです。
 *
 */
class VPVL2_API IRigidBody
{
public:
    enum ShapeType {
        kUnknownShape = -1,
        kSphereShape,
        kBoxShape,
        kCapsureShape,
        kMaxShapeType
    };
    enum ObjectType {
        kStaticObject,
        kDynamicObject,
        kAlignedObject,
        kMaxObjectType
    };

    virtual ~IRigidBody() {}

    virtual void syncLocalTransform() = 0;
    virtual void joinWorld(void *value) = 0;
    virtual void leaveWorld(void *value) = 0;
    virtual void setKinematic(bool value, const Vector3 &basePosition) = 0;
    virtual const Transform createTransform() const = 0;

    virtual void *bodyPtr() const = 0;
    virtual IModel *parentModelRef() const = 0;
    virtual IBone *boneRef() const = 0;
    virtual const IString *name() const = 0;
    virtual const IString *englishName() const = 0;
    virtual Vector3 size() const = 0;
    virtual Vector3 position() const = 0;
    virtual Vector3 rotation() const = 0;
    virtual float32_t mass() const = 0;
    virtual float32_t linearDamping() const = 0;
    virtual float32_t angularDamping() const = 0;
    virtual float32_t restitution() const = 0;
    virtual float32_t friction() const = 0;
    virtual uint16_t groupID() const = 0;
    virtual uint16_t collisionGroupMask() const = 0;
    virtual uint8_t collisionGroupID() const = 0;
    virtual int index() const = 0;

    virtual void setName(const IString *value) = 0;
    virtual void setEnglishName(const IString *value) = 0;
    virtual void setBoneRef(IBone *value) = 0;
    virtual void setAngularDamping(float32_t value) = 0;
    virtual void setCollisionGroupID(uint8_t value) = 0;
    virtual void setCollisionMask(uint16_t value) = 0;
    virtual void setFriction(float32_t value) = 0;
    virtual void setLinearDamping(float32_t value) = 0;
    virtual void setMass(float32_t value) = 0;
    virtual void setPosition(const Vector3 &value) = 0;
    virtual void setRestitution(float32_t value) = 0;
    virtual void setRotation(const Vector3 &value) = 0;
    virtual void setShapeType(ShapeType value) = 0;
    virtual void setSize(const Vector3 &value) = 0;
    virtual void setType(ObjectType value) = 0;
    virtual void setIndex(int value) = 0;
};

} /* namespace vpvl2 */

#endif

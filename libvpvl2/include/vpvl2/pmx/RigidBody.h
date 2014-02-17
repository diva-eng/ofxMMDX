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
#ifndef VPVL2_PMX_RIGIDBODY_H_
#define VPVL2_PMX_RIGIDBODY_H_

#include "vpvl2/internal/BaseRigidBody.h"
#include "vpvl2/pmx/Model.h"
#include "vpvl2/pmx/Morph.h"

class btCollisionShape;
class btRigidBody;
class btMotionState;

namespace vpvl2
{
namespace pmx
{

/**
 * @file
 * @author Nagoya Institute of Technology Department of Computer Science
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * RigidBody class represents a rigid body of a Polygon Model Data object.
 */

class VPVL2_API RigidBody : public internal::BaseRigidBody
{
public:
    RigidBody(IModel *modelRef, IEncoding *encodingRef);
    ~RigidBody();

    static bool preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info);
    static bool loadRigidBodies(const Array<RigidBody *> &rigidBodies, const Array<Bone *> &bones);
    static void writeRigidBodies(const Array<RigidBody *> &rigidBodies, const Model::DataInfo &info, uint8_t *&data);
    static size_t estimateTotalSize(const Array<RigidBody *> &rigidBodies, const Model::DataInfo &info);

    void read(const uint8_t *data, const Model::DataInfo &info, size_t &size);
    void write(uint8_t *&data, const Model::DataInfo &info) const;
    size_t estimateSize(const Model::DataInfo &info) const;
    void reset();
    void mergeMorph(const Morph::Impulse *morph, const IMorph::WeightPrecision &weight);

private:
    VPVL2_DISABLE_COPY_AND_ASSIGN(RigidBody)
};

} /* namespace pmx */
} /* namespace vpvl2 */

#endif


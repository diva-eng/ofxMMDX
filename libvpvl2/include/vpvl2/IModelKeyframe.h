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
#ifndef VPVL2_IMODELKEYFRAME_H_
#define VPVL2_IMODELKEYFRAME_H_

#include "vpvl2/IKeyframe.h"
#include "vpvl2/IVertex.h"

namespace vpvl2
{

/**
 * モデルのキーフレームをあらわすインターフェースです。
 *
 */
class VPVL2_API IModelKeyframe : public IKeyframe
{
public:
    virtual ~IModelKeyframe() {}

    /**
     * IModelKeyframe のインスタンスの完全なコピーを返します.
     *
     * @return IBoneKeyframe
     */
    virtual IModelKeyframe *clone() const = 0;

    virtual bool isVisible() const = 0;
    virtual bool isShadowEnabled() const = 0;
    virtual bool isAddBlendEnabled() const = 0;
    virtual bool isPhysicsEnabled() const = 0;
    virtual bool isInverseKinematicsEnabld(const IBone *value) const = 0;
    virtual uint8_t physicsStillMode() const = 0;
    virtual IVertex::EdgeSizePrecision edgeWidth() const = 0;
    virtual Color edgeColor() const = 0;
    virtual void setVisible(bool value) = 0;
    virtual void setShadowEnable(bool value) = 0;
    virtual void setAddBlendEnable(bool value) = 0;
    virtual void setPhysicsEnable(bool value) = 0;
    virtual void setPhysicsStillMode(uint8_t value) = 0;
    virtual void setEdgeWidth(const IVertex::EdgeSizePrecision &value) = 0;
    virtual void setEdgeColor(const Color &value) = 0;
    virtual void setInverseKinematicsEnable(IBone *bone, bool value) = 0;
};

} /* namespace vpvl2 */

#endif

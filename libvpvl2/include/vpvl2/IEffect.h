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
#ifndef VPVL2_IEFFECT_H_
#define VPVL2_IEFFECT_H_

#include "vpvl2/Common.h"

namespace vpvl2
{

class IString;
class ITexture;

namespace extensions
{
namespace gl
{
class FrameBufferObject;
}
}

class IEffect
{
public:
    class IAnnotation;
    class IPass;
    class ISamplerState;
    class IParameter {
    public:
        enum Type {
            kUnknownParameterType,
            kBoolean,
            kFloat,
            kFloat3,
            kFloat4,
            kFloat4x4,
            kInteger,
            kTexture,
            kSampler2D,
            kSampler3D,
            kSamplerCube
        };
        enum VariableType {
            kUnknownVariableType,
            kConstant,
            kUniform,
            kVarying
        };
        virtual ~IParameter() {}
        virtual IEffect *parentEffectRef() const = 0;
        virtual const IAnnotation *annotationRef(const char *name) const = 0;
        virtual const char *name() const = 0;
        virtual const char *semantic() const = 0;
        virtual Type type() const = 0;
        virtual VariableType variableType() const = 0;
        virtual void connect(IParameter *destinationParameter) = 0;
        virtual void reset() = 0;
        virtual void getValue(int &value) const = 0;
        virtual void getValue(float &value) const = 0;
        virtual void getValue(Vector3 &value) const = 0;
        virtual void getValue(Vector4 &value) const = 0;
        virtual void getMatrix(float *value) const = 0;
        virtual void getArrayDimension(int &value) const = 0;
        virtual void getArrayTotalSize(int &value) const = 0;
        virtual void getTextureRef(intptr_t &value) const = 0;
        virtual void getSamplerStateRefs(Array<ISamplerState *> &value) const = 0;
        virtual void setValue(bool value) = 0;
        virtual void setValue(int value) = 0;
        virtual void setValue(float value) = 0;
        virtual void setValue(const Vector3 &value) = 0;
        virtual void setValue(const Vector4 &value) = 0;
        virtual void setValue(const Vector4 *value) = 0;
        virtual void setMatrix(const float *value) = 0;
        virtual void setSampler(const ITexture *value) = 0;
        virtual void setTexture(const ITexture *value) = 0;
        virtual void setTexture(intptr_t value) = 0;
        virtual void setPointer(const void *ptr, size_t size, size_t stride, Type type) = 0;
    };
    class ITechnique {
    public:
        virtual ~ITechnique() {}
        virtual IEffect *parentEffectRef() const = 0;
        virtual IPass *findPass(const char *name) const = 0;
        virtual const IAnnotation *annotationRef(const char *name) const = 0;
        virtual const char *name() const = 0;
        virtual void getPasses(Array<IPass *> &passes) const = 0;
    };
    class IPass {
    public:
        virtual ~IPass() {}
        virtual ITechnique *parentTechniqueRef() const = 0;
        virtual const IAnnotation *annotationRef(const char *name) const = 0;
        virtual const char *name() const = 0;
        virtual void setState() = 0;
        virtual void resetState() = 0;
    };
    class ISamplerState {
    public:
        virtual ~ISamplerState() {}
        virtual const char *name() const = 0;
        virtual IParameter::Type type() const = 0;
        virtual IParameter *parameterRef() const = 0;
        virtual void getValue(int &value) const = 0;
    };
    class IAnnotation {
    public:
        virtual ~IAnnotation() {}
        virtual bool booleanValue() const = 0;
        virtual int integerValue() const = 0;
        virtual const int *integerValues(int *size) const = 0;
        virtual float floatValue() const = 0;
        virtual const float *floatValues(int *size) const = 0;
        virtual const char *stringValue() const = 0;
    };

    enum ScriptOrderType {
        kPreProcess,
        kStandard,
        kStandardOffscreen,
        kPostProcess,
        kAutoDetection,
        kMaxScriptOrderType
    };
    struct OffscreenRenderTarget {
        ITexture *textureRef;
        IEffect::IParameter *textureParameterRef;
        IEffect::IParameter *samplerParameterRef;
    };
    virtual ~IEffect() {}

    /**
     * CGcontext のインスタンスを返します.
     *
     * void* で返すため、static_cast で CGcontext にキャストする必要があります。
     *
     * @brief internalContext
     * @return
     */
    virtual void *internalContext() const = 0;

    /**
     * CGeffect のインスタンスを返します.
     *
     * void* で返すため、static_cast で CGeffect にキャストする必要があります。
     *
     * @brief internalPointer
     * @return
     */
    virtual void *internalPointer() const = 0;

    /**
     * オフスクリーンターゲットの配列を取得します.
     *
     * @brief getOffscreenRenderTargets
     * @param value
     */
    virtual void getOffscreenRenderTargets(Array<OffscreenRenderTarget> &value) const = 0;

    /**
     * UIWidget セマンティクスがあるパラメータの配列を返します.
     *
     * void* で返すため、個々に static_cast で CGparameter にキャストする必要があります。
     * 現在実装していないため機能していません。
     *
     * @brief getInteractiveParameters
     * @param value
     */
    virtual void getInteractiveParameters(Array<IEffect::IParameter *> &value) const = 0;

    /**
     * 親の IEffect インスタンスのポインタ参照を返します.
     *
     * @brief parentEffect
     * @return
     */
    virtual IEffect *parentEffectRef() const = 0;

    /**
     * 親の IEffect インスタンスのポインタ参照を設定します.
     *
     * @brief parentEffect
     * @param value
     */
    virtual void setParentEffectRef(IEffect *value) = 0;

    /**
     * 親のフレームバッファの参照を返します.
     *
     * エフェクト作成後必ず createFrameBufferObject を呼び出す必要があります。
     * 呼び出されていない場合は NULL を返します。
     *
     * @brief parentFrameBufferObject
     * @return
     */
    virtual extensions::gl::FrameBufferObject *parentFrameBufferObject() const = 0;

    /**
     * フレームバッファを作成します.
     *
     * 別スレッドで処理する際に正しくエフェクトを作成できるようにするため、
     *　フレームバッファの作成は呼び出し側で行う必要があります。
     *
     * @brief createFrameBufferObject
     */
    virtual void createFrameBufferObject() = 0;

    /**
     * スクリプトの描画タイミングを返します.
     *
     * @brief scriptOrderType
     * @return
     */
    virtual ScriptOrderType scriptOrderType() const = 0;

    /**
     * オフスクリーンレンダーターゲットを追加します.
     *
     * @brief addOffscreenRenderTarget
     * @param textureRef
     * @param textureParameterRef
     * @param samplerParameterRef
     */
    virtual void addOffscreenRenderTarget(ITexture *textureRef, IParameter *textureParameterRef, IParameter *samplerParameterRef) = 0;

    /**
     * 接頭子に UI とつくユーザが操作可能なパラメータを追加します.
     *
     * @brief addInteractiveParameter
     * @param value
     */
    virtual void addInteractiveParameter(IEffect::IParameter *value) = 0;

    /**
     * レンダーターゲットを追加します.
     *
     * @brief addRenderColorTargetIndex
     * @param targetIndex
     */
    virtual void addRenderColorTargetIndex(int targetIndex) = 0;

    /**
     * レンダーターゲットを削除します.
     *
     * @brief removeRenderColorTargetIndex
     * @param targetIndex
     */
    virtual void removeRenderColorTargetIndex(int targetIndex) = 0;

    /**
     * 現在設定されているレンダーターゲットを全て削除します.
     *
     * @brief clearRenderColorTargetIndices
     */
    virtual void clearRenderColorTargetIndices() = 0;

    /**
     * スクリプト順序を設定します.
     *
     * @brief setScriptOrderType
     * @param value
     */
    virtual void setScriptOrderType(ScriptOrderType value) = 0;

    /**
     * 指定されたレンダーターゲットの番号が存在するかを返します.
     *
     * @brief hasRenderColorTargetIndex
     * @param targetIndex
     * @return
     */
    virtual bool hasRenderColorTargetIndex(int targetIndex) const = 0;

    /**
     * パラメータ名から OpenGL でいう out (attribute) のパラメータを返します.
     *
     * 存在する場合は非 NULL の IEffect::IParameter のインスタンスを返します。
     * 現在このメソッドは実装されておらず、常に NULL を返します。
     *
     * @brief findVaryingParameter
     * @param name
     * @return
     */
    virtual IEffect::IParameter *findVaryingParameter(const char *name) const = 0;

    /**
     * パラメータ名から OpenGL でいう uniform のパラメータを返します.
     *
     * 存在する場合は非 NULL の IEffect::IParameter のインスタンスを返します.
     *
     * @brief findUniformParameter
     * @param name
     * @return
     */
    virtual IEffect::IParameter *findUniformParameter(const char *name) const = 0;

    /**
     * テクニック名からテクニックのインスタンスを返します.
     *
     * 存在する場合は非 NULL の IEffect::ITechnique のインスタンスを返します.
     *
     * @brief findTechnique
     * @param name
     * @return
     */
    virtual IEffect::ITechnique *findTechnique(const char *name) const = 0;

    /**
     * エフェクトに設定されている全てのパラメータを引数に設定します.
     *
     * @brief getParameterRefs
     * @param parameters
     */
    virtual void getParameterRefs(Array<IParameter *> &parameters) const = 0;

    /**
     * エフェクトに設定されている全てのテクニックを引数に設定します.
     *
     * @brief getTechniqueRefs
     * @param techniques
     */
    virtual void getTechniqueRefs(Array<ITechnique *> &techniques) const = 0;
};

} /* namespace vpvl2 */

#endif

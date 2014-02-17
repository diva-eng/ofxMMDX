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
#ifndef VPVL2_IMOTION_H_
#define VPVL2_IMOTION_H_

#include "vpvl2/Common.h"
#include "vpvl2/IKeyframe.h"

namespace vpvl2
{

class IBoneKeyframe;
class ICameraKeyframe;
class IEffectKeyframe;
class ILightKeyframe;
class IModel;
class IModelKeyframe;
class IMorphKeyframe;
class IProjectKeyframe;
class IString;
class Scene;

/**
 * モーションをあらわすインターフェースです。
 *
 */
class VPVL2_API IMotion
{
public:
    enum Type {
        kUnknownMotion,
        kVMDMotion,
        kMVDMotion,
        kMaxMotionType
    };

    virtual ~IMotion() {}

    /**
     * オンメモリ上にある data とその長さ size に基づいてモーションを構築します.
     *
     * モデルの読み込みに成功した場合は true を、失敗した場合は false を返し、
     * IMotion::error() が kNoError 以外の値を返すようになります。
     *
     * load は複数回呼んでも IMotion では正しく処理されますが、IMotion に依存する
     * クラスが正しく処理されなくなるので、あまり推奨しません。
     *
     * @param data
     * @param size
     * @return bool
     * @sa save
     * @sa estimateSize
     */
    virtual bool load(const uint8_t *data, size_t size) = 0;

    /**
     * オンメモリ上にある data に IMotion のインスタンスに基づいてデータを書き込みます.
     *
     * data の長さは IMotion::estimateSize() が返す値を利用してください。
     *
     * @param data
     * @sa load
     * @sa estimateSize
     */
    virtual void save(uint8_t *data) const = 0;

    /**
     * IMotion::save(data) に必要なデータの長さを返します.
     *
     * これは save を呼ぶ前に save に渡すデータをメモリ上に確保する量を求めるときに使います。
     * save と併せて使用する必要があります。
     *
     * @return size_t
     * @sa load
     * @sa save
     */
    virtual size_t estimateSize() const = 0;

    /**
     * モーションが依存するモデルのポインタを返します.
     *
     * ボーンとモーフのモーションは原則非 null を返します。
     * カメラと照明のモーションはモデルに依存しないため、null を返します。
     *
     * @return IModel
     * @sa setParentModel
     */
    virtual IModel *parentModelRef() const = 0;

    /**
     * モーションが依存するモデルを設定します.
     *
     * null の設定も可能です。
     *
     * @param IModel
     * @sa parentModel
     */
    virtual void setParentModelRef(IModel *model) = 0;

    /**
     * モーションを指定されたフレームの位置に移動します.
     *
     * 内部的にはフレームの位置が保存されています。
     *
     * @param timeIndex
     * @sa advance
     */
    virtual void seek(const IKeyframe::TimeIndex &timeIndex) = 0;

    /**
     * モーションを指定されたフレームの位置に移動した上で場面を更新します.
     *
     * 場面オブジェクトの更新が行われること以外 seek() と同じです。
     *
     * @param timeIndex
     * @param Scene
     */
    virtual void seekScene(const IKeyframe::TimeIndex &timeIndex, Scene *scene) = 0;

    /**
     * モーションを指定されたフレームの位置分進めます.
     *
     * 前回 advance または seek を呼んだ位置から引数の値を加算してフレームを進めます。
     * 内部的には IMotion::seek() を呼び出しています。
     *
     * @param delta
     * @sa seek
     */
    virtual void advance(const IKeyframe::TimeIndex &deltaTimeIndex) = 0;

    /**
     * モーションを指定されたフレームの位置分進めた上で上で場面を更新します.
     *
     * 場面オブジェクトの更新が行われること以外 advance() と同じです。
     *
     * @param delta
     * @param Scene
     */
    virtual void advanceScene(const IKeyframe::TimeIndex &deltaTimeIndex, Scene *scene) = 0;

    /**
     * モーションを最初の位置にリセットします.
     *
     */
    virtual void reset() = 0;

    /**
     * モーションの継続時間（モーションの終端位置となる timeIndex）を返します.
     *
     * @return float
     * @sa isReachedTo
     */
    virtual IKeyframe::TimeIndex duration() const = 0;

    /**
     * モーションが指定されたフレームの位置まで進んでいるかを返します.
     *
     * @return bool
     * @sa maxFrameIndex
     */
    virtual bool isReachedTo(const IKeyframe::TimeIndex &timeIndex) const = 0;

    /**
     * キーフレームを追加します.
     *
     * キーフレームを追加したら必ず update を行う必要があります。そうしないと find* 系が正しい値を返さなくなってしまいます。
     * また、メモリの所有権が IMotion 側に移動するため、メモリを解放しないようにする必要があります。
     *
     * @param IKeyframe
     * @sa replaceKeyframe
     * @sa update
     */
    virtual void addKeyframe(IKeyframe *value) = 0;

    /**
     * 指定されたキーフレームの型にあるキーフレームの数を返します.
     *
     * @param IKeyframe::Type
     * @return int
     */
    virtual int countKeyframes(IKeyframe::Type value) const = 0;

    /**
     * キーフレームの名前と型からレイヤー数を返します.
     *
     * @param name
     * @param type
     * @return int
     */
    virtual IKeyframe::LayerIndex countLayers(const IString *name,
                                              IKeyframe::Type type) const = 0;

    /**
     * 時間のインデックスとレイヤーインデックスとキーフレーム型から存在するキーフレームをすべて返します.
     *
     * 見つかったキーフレームは keyframes 引数に渡され、値が更新されます。
     * 値は全てポインタの参照として渡されるため、delete でメモリを解放してはいけません。
     *
     * @brief getKeyframes
     * @param timeIndex
     * @param layerIndex
     * @param type
     * @param keyframes
     */
    virtual void getKeyframeRefs(const IKeyframe::TimeIndex &timeIndex,
                              const IKeyframe::LayerIndex &layerIndex,
                              IKeyframe::Type type,
                              Array<IKeyframe *> &keyframes) = 0;

    /**
     * キーフレームの位置と名前からボーンのキーフレームを返します.
     *
     * 見つかった場合は IBoneKeyframe インスタンスを、見つからない場合は 0 を返します。
     * ポインタの参照を返すため、delete でメモリを解放してはいけません。
     *
     * @param timeIndex
     * @param name
     * @param layerIndex
     * @return IBoneKeyframe
     * @sa findBoneKeyframeAt
     */
    virtual IBoneKeyframe *findBoneKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                            const IString *name,
                                            const IKeyframe::LayerIndex &layerIndex) const = 0;

    /**
     * キーフレームの配列の添字からボーン該当するキーフレームの全てを返します.
     *
     * 見つかった場合は IBoneKeyframe インスタンスを、見つからない場合は 0 を返します。
     * ポインタの参照を返すため、delete でメモリを解放してはいけません。
     *
     * @param index
     * @return IBoneKeyframe
     * @sa findBoneKeyframe
     */
    virtual IBoneKeyframe *findBoneKeyframeRefAt(int index) const = 0;

    /**
     * キーフレームの位置からカメラのキーフレームを返します.
     *
     * 見つかった場合は ICameraKeyframe インスタンスを、見つからない場合は 0 を返します。
     * ポインタの参照を返すため、delete でメモリを解放してはいけません。
     *
     * @param timeIndex
     * @param layerIndex
     * @return IBoneKeyframe
     * @sa findCameraKeyframeAt
     */
    virtual ICameraKeyframe *findCameraKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                                const IKeyframe::LayerIndex &layerIndex) const = 0;

    /**
     * キーフレームの配列の添字からカメラのキーフレームを返します.
     *
     * 見つかった場合は ICameraKeyframe インスタンスを、見つからない場合は 0 を返します。
     * ポインタの参照を返すため、delete でメモリを解放してはいけません。
     *
     * @param index
     * @return ICameraKeyframe
     * @sa findCameraKeyframe
     */
    virtual ICameraKeyframe *findCameraKeyframeRefAt(int index) const = 0;

    virtual IEffectKeyframe *findEffectKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                                const IString *name,
                                                const IKeyframe::LayerIndex &layerIndex) const = 0;

    virtual IEffectKeyframe *findEffectKeyframeRefAt(int index) const = 0;

    /**
     * キーフレームの位置から照明のキーフレームを返します.
     *
     * 見つかった場合は ILightKeyframe インスタンスを、見つからない場合は 0 を返します。
     * ポインタの参照を返すため、delete でメモリを解放してはいけません。
     *
     * @param timeIndex
     * @param layerIndex
     * @return ILightKeyframe
     * @sa findLightKeyframeAt
     */
    virtual ILightKeyframe *findLightKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                              const IKeyframe::LayerIndex &layerIndex) const = 0;

    /**
     * キーフレームの配列の位置から照明のキーフレームを返します.
     *
     * 見つかった場合は ILightKeyframe インスタンスを、見つからない場合は 0 を返します。
     * ポインタの参照を返すため、delete でメモリを解放してはいけません。
     *
     * @param index
     * @return ILightKeyframe
     * @sa findLightKeyframe
     */
    virtual ILightKeyframe *findLightKeyframeRefAt(int index) const = 0;

    virtual IModelKeyframe *findModelKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                              const IKeyframe::LayerIndex &layerIndex) const = 0;

    virtual IModelKeyframe *findModelKeyframeRefAt(int index) const = 0;

    /**
     * キーフレームの位置と名前からモーフのキーフレームを返します.
     *
     * 見つかった場合は IMorphKeyframe インスタンスを、見つからない場合は 0 を返します。
     * ポインタの参照を返すため、delete でメモリを解放してはいけません。
     *
     * @param timeIndex
     * @param name
     * @param layerIndex
     * @return IMorphKeyframe
     * @sa findMorphKeyframeAt
     */
    virtual IMorphKeyframe *findMorphKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                              const IString *name,
                                              const IKeyframe::LayerIndex &layerIndex) const = 0;

    /**
     * キーフレームの配列の添字からモーフのキーフレームを返します.
     *
     * 見つかった場合は IMorphKeyframe インスタンスを、見つからない場合は 0 を返します。
     * ポインタの参照を返すため、delete でメモリを解放してはいけません。
     *
     * @param index
     * @return IMorphKeyframe
     * @sa findMorphKeyframe
     */
    virtual IMorphKeyframe *findMorphKeyframeRefAt(int index) const = 0;

    virtual IProjectKeyframe *findProjectKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                                  const IKeyframe::LayerIndex &layerIndex) const = 0;

    virtual IProjectKeyframe *findProjectKeyframeRefAt(int index) const = 0;

    /**
     * キーフレームを置換します.
     *
     * キーフレームは内部的に削除してから追加されるため、addKeyframe 同様 update を呼び出す必要があります。
     * 同様にメモリの所有権が IMotion 側に移動するため、メモリを解放しないようにする必要があります。
     *
     * @param IKeyframe
     * @sa addKeyframe
     * @sa update
     */
    virtual void replaceKeyframe(IKeyframe *value) = 0;

    /**
     * 指定されたキーフレームを削除します.
     *
     * キーフレームのインスタンスを delete を使って物理的に削除するため、呼び出し後引数の値は無効になり、0 にリセットされます。
     *
     * @param IKeyframe
     */
    virtual void deleteKeyframe(IKeyframe *&value) = 0;

    /**
     * 指定されたキーフレームの型の情報を更新します.
     *
     * @param IKeyframe::Type
     */
    virtual void update(IKeyframe::Type type) = 0;

    virtual void getAllKeyframeRefs(Array<IKeyframe *> &value, IKeyframe::Type type) = 0;

    virtual void setAllKeyframes(const Array<IKeyframe *> &value, IKeyframe::Type type) = 0;

    /**
     * モーションのコピーを作成します.
     *
     * @return IMotion
     */
    virtual IMotion *clone() const = 0;

    /**
     * キーフレーム数が1つしかない場合でも反映させるかを指定します.
     *
     * @return bool
     * @sa setNullFrameEnable
     */
    virtual bool isNullFrameEnabled() const = 0;

    /**
     * キーフレーム数が1つしかない場合でも反映させるかを設定します.
     *
     * @param bool
     * @sa isNullFrameEnabled
     */
    virtual void setNullFrameEnable(bool value) = 0;

    /**
     * 親の場面インスタンスの参照を返します.
     *
     * @brief parentSceneRef
     * @return
     */
    virtual Scene *parentSceneRef() const = 0;

    /**
     * モーション名を返します.
     *
     * @return IString
     */
    virtual const IString *name() const = 0;

    /**
     * モーションの型を返します.
     *
     * @return Type
     */
    virtual Type type() const = 0;
};

} /* namespace vpvl2 */

#endif


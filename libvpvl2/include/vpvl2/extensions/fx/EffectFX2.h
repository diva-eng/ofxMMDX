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
#ifndef VPVL2_EXTENSIONS_FX_EFFECTFX2_H_
#define VPVL2_EXTENSIONS_FX_EFFECTFX2_H_

#include <vpvl2/IEffect.h>

namespace vpvl2
{

class IEncoding;
class IString;

namespace extensions
{
namespace fx
{

class EffectFX2 {
public:
    EffectFX2(IEncoding *encoding);
    ~EffectFX2();

    bool parse(const uint8_t *data, size_t size);

private:
    struct ParseData {
        ParseData(uint8_t *ptr, size_t size, size_t rest)
            : base(ptr),
              size(size),
              ptr(ptr),
              nshaders(0),
              rest(rest)
        {
        }
        const uint8_t *base;
        const size_t size;
        uint8_t *ptr;
        size_t nshaders;
        size_t rest;
    };
    struct Annotateable;
    struct Annotation;
    struct Parameter;
    struct Technique;
    struct Pass;
    struct State;
    struct Texture;
    struct Shader;

    static bool lookup(const ParseData &data, size_t offset, uint32_t &value);
    static uint32_t paddingSize(uint32_t size);
    bool parseString(const ParseData &data, size_t offset, IString *&string);
    bool parseRawString(const ParseData &data, const uint8_t *ptr, size_t size, IString *&string);
    bool parseAnnotationIndices(ParseData &data, Annotateable *annotate, const int nannotations);
    bool parseParameters(ParseData &data, const int nparameters);
    bool parseTechniques(ParseData &data, const int ntechniques);
    bool parsePasses(ParseData &data, Technique *technique, const int npasses);
    bool parseStates(ParseData &data, const int nstates);
    bool parseAnnotations(ParseData &data, const int nannotations);
    bool parseShaders(ParseData &data, const int nshaders);
    bool parseTextures(ParseData &data, const int ntextures);

    typedef Hash<HashString, Parameter *> String2ParameterRefHash;
    typedef Hash<HashString, Technique *> String2TechniqueRefHash;
    typedef Hash<HashString, Pass *> String2PassRefHash;
    typedef Hash<HashString, Annotation *> String2AnnotationRefHash;
    typedef Hash<HashString, Texture *> String2TextureRefHash;
    IEncoding *m_encoding;
    PointerArray<Parameter> m_parameters;
    String2ParameterRefHash m_name2ParameterRef;
    String2ParameterRefHash m_semantic2ParameterRef;
    PointerArray<Technique> m_techniques;
    String2TechniqueRefHash m_name2TechniqueRef;
    PointerArray<Pass> m_passes;
    String2PassRefHash m_name2PassRef;
    PointerArray<State> m_states;
    PointerHash<HashInt, Annotation> m_annotations;
    String2AnnotationRefHash m_name2AnnotationRef;
    PointerArray<Texture> m_textures;
    String2TextureRefHash m_name2TextureRef;
    PointerArray<Shader> m_shaders;

    VPVL2_DISABLE_COPY_AND_ASSIGN(EffectFX2)
};

} /* namespace fx */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif

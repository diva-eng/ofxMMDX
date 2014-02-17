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

#ifndef VPVL2_CG2_PMDRENDERENGINE_H_
#define VPVL2_CG2_PMDRENDERENGINE_H_

#include "vpvl2/IRenderEngine.h"
#include "vpvl2/cg/EffectEngine.h"
#include "vpvl2/pmd/Model.h"

namespace vpvl2
{

class Scene;

namespace cl {
class PMDAccelerator;
}
namespace pmd {
class Model;
}

namespace cg
{

class VPVL2_API PMDRenderEngine : public vpvl2::IRenderEngine
        #ifdef VPVL2_LINK_QT
        , protected QGLFunctions
        #endif
{
public:
    class PrivateContext;

    PMDRenderEngine(IRenderContext *delegate,
                    const Scene *scene,
                    CGcontext effectContext,
                    cl::PMDAccelerator *accelerator,
                    pmd::Model *parentModelRef);
    virtual ~PMDRenderEngine();

    IModel *parentModelRef() const;
    bool upload(const IString *dir);
    void update();
    void renderModel();
    void renderEdge();
    void renderShadow();
    void renderZPlot();
    bool hasPreProcess() const;
    bool hasPostProcess() const;
    void preparePostProcess();
    void performPreProcess();
    void performPostProcess();
    IEffect *effectRef(IEffect::ScriptOrderType type) const;
    void setEffect(IEffect::ScriptOrderType type, IEffect *effectRef, const IString *dir);

protected:
    void log0(void *context, IRenderContext::LogLevel level, const char *format ...);

    IRenderContext *m_delegateRef;

private:
    bool releaseContext0(void *context);
    void release();

    enum VertexBufferObjectType {
        kModelVertices,
        kEdgeIndices,
        kModelIndices,
        kVertexBufferObjectMax
    };
    struct MaterialContext {
        MaterialContext()
            : mainTextureID(0),
              subTextureID(0)
        {
        }
        GLuint mainTextureID;
        GLuint subTextureID;
    };

    const Scene *m_sceneRef;
    EffectEngine *m_currentRef;
    cl::PMDAccelerator *m_accelerator;
    pmd::Model *m_modelRef;
    CGcontext m_contextRef;
    MaterialContext *m_materialContexts;
    pmd::Model::SkinningMeshes m_mesh;
    Color m_toonTextureColors[vpvl::PMDModel::kCustomTextureMax];
    GLuint m_vertexBufferObjects[kVertexBufferObjectMax];
    Hash<btHashInt, EffectEngine *> m_effects;
    Array<EffectEngine *> m_oseffects;
    bool m_cullFaceState;
    bool m_isVertexShaderSkinning;

    VPVL2_DISABLE_COPY_AND_ASSIGN(PMDRenderEngine)
};

} /* namespace gl2 */
} /* namespace vpvl2 */

#endif

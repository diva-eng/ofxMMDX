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

#include "EngineCommon.h"
#include "vpvl2/extensions/gl/VertexBundle.h"
#include "vpvl2/extensions/gl/VertexBundleLayout.h"
#include "vpvl2/internal/util.h" /* internal::snprintf */
#include "vpvl2/gl2/PMXRenderEngine.h"
#ifdef VPVL2_ENABLE_OPENCL
#include "vpvl2/cl/Context.h"
#include "vpvl2/cl/PMXAccelerator.h"
#endif

using namespace vpvl2;
using namespace vpvl2::gl2;
using namespace vpvl2::extensions::gl;

namespace {

enum VertexBufferObjectType
{
    kModelDynamicVertexBufferEven,
    kModelDynamicVertexBufferOdd,
    kModelStaticVertexBuffer,
    kModelIndexBuffer,
    kMaxVertexBufferObjectType
};

enum VertexArrayObjectType
{
    kVertexArrayObjectEven,
    kVertexArrayObjectOdd,
    kEdgeVertexArrayObjectEven,
    kEdgeVertexArrayObjectOdd,
    kMaxVertexArrayObjectType
};

struct MaterialTextureRefs
{
    MaterialTextureRefs()
        : mainTextureRef(0),
          sphereTextureRef(0),
          toonTextureRef(0)
    {
    }
    ~MaterialTextureRefs() {
        mainTextureRef = 0;
        sphereTextureRef = 0;
        toonTextureRef = 0;
    }
    ITexture *mainTextureRef;
    ITexture *sphereTextureRef;
    ITexture *toonTextureRef;
};

class ExtendedZPlotProgram : public ZPlotProgram
{
public:
    ExtendedZPlotProgram()
        : ZPlotProgram(),
          m_boneMatricesUniformLocation(-1)
    {
    }
    ~ExtendedZPlotProgram() {
        m_boneMatricesUniformLocation = -1;
    }

    void setBoneMatrices(const Scalar *value, size_t size) {
        glUniformMatrix4fv(m_boneMatricesUniformLocation, size, GL_FALSE, value);
    }

protected:
    virtual void bindAttributeLocations() {
        ZPlotProgram::bindAttributeLocations();
        glBindAttribLocation(m_program, IModel::Buffer::kBoneIndexStride, "inBoneIndices");
        glBindAttribLocation(m_program, IModel::Buffer::kBoneWeightStride, "inBoneWeights");
    }
    virtual void getUniformLocations() {
        ZPlotProgram::getUniformLocations();
        m_boneMatricesUniformLocation = glGetUniformLocation(m_program, "boneMatrices");
    }

private:
    GLint m_boneMatricesUniformLocation;
};

class EdgeProgram : public BaseShaderProgram
{
public:
    EdgeProgram()
        : BaseShaderProgram(),
          m_colorUniformLocation(-1),
          m_edgeSizeUniformLocation(-1),
          m_opacityUniformLocation(-1),
          m_boneMatricesUniformLocation(-1)
    {
    }
    ~EdgeProgram() {
        m_colorUniformLocation = -1;
        m_edgeSizeUniformLocation = -1;
        m_opacityUniformLocation = -1;
        m_boneMatricesUniformLocation = -1;
    }

    void setColor(const Color &value) {
        glUniform4fv(m_colorUniformLocation, 1, value);
    }
    void setSize(const Scalar &value) {
        glUniform1f(m_edgeSizeUniformLocation, value);
    }
    void setOpacity(const Scalar &value) {
        glUniform1f(m_opacityUniformLocation, value);
    }
    void setBoneMatrices(const Scalar *value, size_t size) {
        glUniformMatrix4fv(m_boneMatricesUniformLocation, size, GL_FALSE, value);
    }

protected:
    virtual void bindAttributeLocations() {
        BaseShaderProgram::bindAttributeLocations();
        glBindAttribLocation(m_program, IModel::Buffer::kEdgeSizeStride, "inEdgeSize");
        glBindAttribLocation(m_program, IModel::Buffer::kBoneIndexStride, "inBoneIndices");
        glBindAttribLocation(m_program, IModel::Buffer::kBoneWeightStride, "inBoneWeights");
    }
    virtual void getUniformLocations() {
        BaseShaderProgram::getUniformLocations();
        m_colorUniformLocation = glGetUniformLocation(m_program, "color");
        m_edgeSizeUniformLocation = glGetUniformLocation(m_program, "edgeSize");
        m_opacityUniformLocation = glGetUniformLocation(m_program, "opacity");
        m_boneMatricesUniformLocation = glGetUniformLocation(m_program, "boneMatrices");
    }

private:
    GLint m_colorUniformLocation;
    GLint m_edgeSizeUniformLocation;
    GLint m_opacityUniformLocation;
    GLint m_boneMatricesUniformLocation;
};

class ShadowProgram : public ObjectProgram
{
public:
    ShadowProgram()
        : ObjectProgram(),
          m_shadowMatrixUniformLocation(-1),
          m_boneMatricesUniformLocation(-1)
    {
    }
    ~ShadowProgram() {
        m_shadowMatrixUniformLocation = -1;
        m_boneMatricesUniformLocation = -1;
    }

    void setShadowMatrix(const float value[16]) {
        glUniformMatrix4fv(m_shadowMatrixUniformLocation, 1, GL_FALSE, value);
    }
    void setBoneMatrices(const Scalar *value, size_t size) {
        glUniformMatrix4fv(m_boneMatricesUniformLocation, size, GL_FALSE, value);
    }

protected:
    virtual void bindAttributeLocations() {
        glBindAttribLocation(m_program, IModel::Buffer::kBoneIndexStride, "inBoneIndices");
        glBindAttribLocation(m_program, IModel::Buffer::kBoneWeightStride, "inBoneWeights");
    }
    virtual void getUniformLocations() {
        ObjectProgram::getUniformLocations();
        m_shadowMatrixUniformLocation = glGetUniformLocation(m_program, "shadowMatrix");
        m_boneMatricesUniformLocation = glGetUniformLocation(m_program, "boneMatrices");
    }

private:
    GLint m_shadowMatrixUniformLocation;
    GLint m_boneMatricesUniformLocation;
};

class ModelProgram : public ObjectProgram
{
public:
    ModelProgram()
        : ObjectProgram(),
          m_cameraPositionUniformLocation(-1),
          m_materialColorUniformLocation(-1),
          m_materialSpecularUniformLocation(-1),
          m_materialShininessUniformLocation(-1),
          m_mainTextureBlendUniformLocation(-1),
          m_sphereTextureBlendUniformLocation(-1),
          m_toonTextureBlendUniformLocation(-1),
          m_sphereTextureUniformLocation(-1),
          m_hasSphereTextureUniformLocation(-1),
          m_isSPHTextureUniformLocation(-1),
          m_isSPATextureUniformLocation(-1),
          m_isSubTextureUniformLocation(-1),
          m_toonTextureUniformLocation(-1),
          m_hasToonTextureUniformLocation(-1),
          m_useToonUniformLocation(-1),
          m_boneMatricesUniformLocation(-1)
    {
    }
    ~ModelProgram() {
        m_cameraPositionUniformLocation = -1;
        m_materialColorUniformLocation = -1;
        m_materialSpecularUniformLocation = -1;
        m_materialShininessUniformLocation = -1;
        m_mainTextureBlendUniformLocation = -1;
        m_sphereTextureBlendUniformLocation = -1;
        m_toonTextureBlendUniformLocation = -1;
        m_sphereTextureUniformLocation = -1;
        m_hasSphereTextureUniformLocation = -1;
        m_isSPHTextureUniformLocation = -1;
        m_isSPATextureUniformLocation = -1;
        m_isSubTextureUniformLocation = -1;
        m_toonTextureUniformLocation = -1;
        m_hasToonTextureUniformLocation = -1;
        m_useToonUniformLocation = -1;
        m_boneMatricesUniformLocation = -1;
    }

    void setCameraPosition(const Vector3 &value) {
        glUniform3fv(m_cameraPositionUniformLocation, 1, value);
    }
    void setMaterialColor(const Color &value) {
        glUniform4fv(m_materialColorUniformLocation, 1, value);
    }
    void setMaterialSpecular(const Color &value) {
        glUniform3fv(m_materialSpecularUniformLocation, 1, value);
    }
    void setMaterialShininess(const Scalar &value) {
        glUniform1f(m_materialShininessUniformLocation, value);
    }
    void setMainTextureBlend(const Color &value) {
        glUniform4fv(m_mainTextureBlendUniformLocation, 1, value);
    }
    void setSphereTextureBlend(const Color &value) {
        glUniform4fv(m_sphereTextureBlendUniformLocation, 1, value);
    }
    void setToonTextureBlend(const Color &value) {
        glUniform4fv(m_toonTextureBlendUniformLocation, 1, value);
    }
    void setToonEnable(bool value) {
        glUniform1i(m_useToonUniformLocation, value ? 1 : 0);
    }
    void setSphereTexture(const ITexture *value, IMaterial::SphereTextureRenderMode mode) {
        if (value) {
            switch (mode) {
            case IMaterial::kNone:
            default:
                glUniform1i(m_hasSphereTextureUniformLocation, 0);
                glUniform1i(m_isSPHTextureUniformLocation, 0);
                glUniform1i(m_isSPATextureUniformLocation, 0);
                glUniform1i(m_isSubTextureUniformLocation, 0);
                break;
            case IMaterial::kMultTexture:
                enableSphereTexture(value);
                glUniform1i(m_isSPHTextureUniformLocation, 1);
                glUniform1i(m_isSPATextureUniformLocation, 0);
                glUniform1i(m_isSubTextureUniformLocation, 0);
                break;
            case IMaterial::kAddTexture:
                enableSphereTexture(value);
                glUniform1i(m_isSPHTextureUniformLocation, 0);
                glUniform1i(m_isSPATextureUniformLocation, 1);
                glUniform1i(m_isSubTextureUniformLocation, 0);
                break;
            case IMaterial::kSubTexture:
                enableSphereTexture(value);
                glUniform1i(m_isSPHTextureUniformLocation, 0);
                glUniform1i(m_isSPATextureUniformLocation, 0);
                glUniform1i(m_isSubTextureUniformLocation, 1);
                break;
            }
        }
        else {
            glUniform1i(m_hasSphereTextureUniformLocation, 0);
        }
    }
    void setToonTexture(const ITexture *value) {
        if (value) {
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(value->data()));
            glUniform1i(m_toonTextureUniformLocation, 2);
            glUniform1i(m_hasToonTextureUniformLocation, 1);
        }
        else {
            glUniform1i(m_hasToonTextureUniformLocation, 0);
        }
    }
    void setBoneMatrices(const Scalar *value, size_t size) {
        glUniformMatrix4fv(m_boneMatricesUniformLocation, size, GL_FALSE, value);
    }

protected:
    virtual void bindAttributeLocations() {
        ObjectProgram::bindAttributeLocations();
        glBindAttribLocation(m_program, IModel::Buffer::kUVA0Stride, "inUVA0");
        glBindAttribLocation(m_program, IModel::Buffer::kUVA1Stride, "inUVA1");
        glBindAttribLocation(m_program, IModel::Buffer::kBoneIndexStride, "inBoneIndices");
        glBindAttribLocation(m_program, IModel::Buffer::kBoneWeightStride, "inBoneWeights");
    }
    virtual void getUniformLocations() {
        ObjectProgram::getUniformLocations();
        m_cameraPositionUniformLocation = glGetUniformLocation(m_program, "cameraPosition");
        m_materialColorUniformLocation = glGetUniformLocation(m_program, "materialColor");
        m_materialSpecularUniformLocation = glGetUniformLocation(m_program, "materialSpecular");
        m_materialShininessUniformLocation = glGetUniformLocation(m_program, "materialShininess");
        m_mainTextureBlendUniformLocation = glGetUniformLocation(m_program, "mainTextureBlend");
        m_sphereTextureBlendUniformLocation = glGetUniformLocation(m_program, "sphereTextureBlend");
        m_toonTextureBlendUniformLocation = glGetUniformLocation(m_program, "toonTextureBlend");
        m_sphereTextureUniformLocation = glGetUniformLocation(m_program, "sphereTexture");
        m_hasSphereTextureUniformLocation = glGetUniformLocation(m_program, "hasSphereTexture");
        m_isSPHTextureUniformLocation = glGetUniformLocation(m_program, "isSPHTexture");
        m_isSPATextureUniformLocation = glGetUniformLocation(m_program, "isSPATexture");
        m_isSubTextureUniformLocation = glGetUniformLocation(m_program, "isSubTexture");
        m_toonTextureUniformLocation = glGetUniformLocation(m_program, "toonTexture");
        m_hasToonTextureUniformLocation = glGetUniformLocation(m_program, "hasToonTexture");
        m_useToonUniformLocation = glGetUniformLocation(m_program, "useToon");
        m_boneMatricesUniformLocation = glGetUniformLocation(m_program, "boneMatrices");
    }

private:
    void enableSphereTexture(const ITexture *value) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(value->data()));
        glUniform1i(m_sphereTextureUniformLocation, 1);
        glUniform1i(m_hasSphereTextureUniformLocation, 1);
    }

    GLint m_cameraPositionUniformLocation;
    GLint m_materialColorUniformLocation;
    GLint m_materialSpecularUniformLocation;
    GLint m_materialShininessUniformLocation;
    GLint m_mainTextureBlendUniformLocation;
    GLint m_sphereTextureBlendUniformLocation;
    GLint m_toonTextureBlendUniformLocation;
    GLint m_sphereTextureUniformLocation;
    GLint m_hasSphereTextureUniformLocation;
    GLint m_isSPHTextureUniformLocation;
    GLint m_isSPATextureUniformLocation;
    GLint m_isSubTextureUniformLocation;
    GLint m_toonTextureUniformLocation;
    GLint m_hasToonTextureUniformLocation;
    GLint m_useToonUniformLocation;
    GLint m_boneMatricesUniformLocation;
};

}

namespace vpvl2
{
namespace gl2
{

class PMXRenderEngine::PrivateContext
{
public:
    PrivateContext(const IModel *model, bool isVertexShaderSkinning)
        : modelRef(model),
          indexBuffer(0),
          staticBuffer(0),
          dynamicBuffer(0),
          matrixBuffer(0),
          edgeProgram(0),
          modelProgram(0),
          shadowProgram(0),
          zplotProgram(0),
          aabbMin(SIMD_INFINITY, SIMD_INFINITY, SIMD_INFINITY),
          aabbMax(-SIMD_INFINITY, -SIMD_INFINITY, -SIMD_INFINITY),
          cullFaceState(true),
          isVertexShaderSkinning(isVertexShaderSkinning),
          updateEven(true)
    {
        model->getIndexBuffer(indexBuffer);
        model->getStaticVertexBuffer(staticBuffer);
        model->getDynamicVertexBuffer(dynamicBuffer, indexBuffer);
        if (isVertexShaderSkinning)
            model->getMatrixBuffer(matrixBuffer, dynamicBuffer, indexBuffer);
        switch (indexBuffer->type()) {
        case IModel::IndexBuffer::kIndex32:
            indexType = GL_UNSIGNED_INT;
            break;
        case IModel::IndexBuffer::kIndex16:
            indexType = GL_UNSIGNED_SHORT;
            break;
        case IModel::IndexBuffer::kIndex8:
            indexType = GL_UNSIGNED_BYTE;
            break;
        case IModel::IndexBuffer::kMaxIndexType:
        default:
            indexType = GL_UNSIGNED_INT;
            break;
        }
    }
    virtual ~PrivateContext() {
        allocatedTextures.releaseAll();
        delete indexBuffer;
        indexBuffer = 0;
        delete dynamicBuffer;
        dynamicBuffer = 0;
        delete staticBuffer;
        staticBuffer = 0;
        delete edgeProgram;
        edgeProgram = 0;
        delete modelProgram;
        modelProgram = 0;
        delete shadowProgram;
        shadowProgram = 0;
        delete zplotProgram;
        zplotProgram = 0;
        aabbMin.setZero();
        aabbMax.setZero();
        cullFaceState = false;
        isVertexShaderSkinning = false;
    }

    void getVertexBundleType(VertexArrayObjectType &vao, VertexBufferObjectType &vbo) {
        if (updateEven) {
            vao = kVertexArrayObjectOdd;
            vbo = kModelDynamicVertexBufferOdd;
        }
        else {
            vao = kVertexArrayObjectEven;
            vbo = kModelDynamicVertexBufferEven;
        }
    }
    void getEdgeBundleType(VertexArrayObjectType &vao, VertexBufferObjectType &vbo) {
        if (updateEven) {
            vao = kEdgeVertexArrayObjectOdd;
            vbo = kModelDynamicVertexBufferOdd;
        }
        else {
            vao = kEdgeVertexArrayObjectEven;
            vbo = kModelDynamicVertexBufferEven;
        }
    }

    const IModel *modelRef;
    IModel::IndexBuffer *indexBuffer;
    IModel::StaticVertexBuffer *staticBuffer;
    IModel::DynamicVertexBuffer *dynamicBuffer;
    IModel::MatrixBuffer *matrixBuffer;
    EdgeProgram *edgeProgram;
    ModelProgram *modelProgram;
    ShadowProgram *shadowProgram;
    ExtendedZPlotProgram *zplotProgram;
    VertexBundle buffer;
    VertexBundleLayout bundles[kMaxVertexArrayObjectType];
    GLenum indexType;
    PointerHash<HashPtr, ITexture> allocatedTextures;
    Array<MaterialTextureRefs> materialTextureRefs;
    Vector3 aabbMin;
    Vector3 aabbMax;
#ifdef VPVL2_ENABLE_OPENCL
    cl::PMXAccelerator::Buffers buffers;
#endif
    bool cullFaceState;
    bool isVertexShaderSkinning;
    bool updateEven;
};

PMXRenderEngine::PMXRenderEngine(IRenderContext *renderContext,
                                 Scene *scene,
                                 cl::PMXAccelerator *accelerator,
                                 IModel *modelRef)
    : m_accelerator(accelerator),
      m_renderContextRef(renderContext),
      m_sceneRef(scene),
      m_modelRef(modelRef),
      m_context(0)
{
    bool vss = m_sceneRef->accelerationType() == Scene::kVertexShaderAccelerationType1;
    m_context = new PrivateContext(modelRef, vss);
#ifdef VPVL2_ENABLE_OPENCL
    if (vss || (m_accelerator && m_accelerator->isAvailable()))
        m_context->dynamicBuffer->setSkinningEnable(false);
#endif
}

PMXRenderEngine::~PMXRenderEngine()
{
#ifdef VPVL2_ENABLE_OPENCL
    if (m_context) {
        m_accelerator->release(m_context->buffers);
    }
    delete m_accelerator;
#endif
    if (m_context) {
        delete m_context;
        m_context = 0;
    }
    m_renderContextRef = 0;
    m_sceneRef = 0;
    m_modelRef = 0;
    m_accelerator = 0;
}

IModel *PMXRenderEngine::parentModelRef() const
{
    return m_modelRef && m_modelRef->parentSceneRef() ? m_modelRef : 0;
}

bool PMXRenderEngine::upload(const IString *dir)
{
    bool ret = true, vss = false;
    void *userData = 0;
    if (!m_context) {
        vss = m_sceneRef->accelerationType() == Scene::kVertexShaderAccelerationType1;
        m_context = new PrivateContext(m_modelRef, vss);
        m_context->dynamicBuffer->setSkinningEnable(false);
    }
    vss = m_context->isVertexShaderSkinning;
    m_renderContextRef->allocateUserData(m_modelRef, userData);
    m_renderContextRef->startProfileSession(IRenderContext::kProfileUploadModelProcess, m_modelRef);
    EdgeProgram *edgeProgram = m_context->edgeProgram = new EdgeProgram();
    ModelProgram *modelProgram = m_context->modelProgram = new ModelProgram();
    ShadowProgram *shadowProgram = m_context->shadowProgram = new ShadowProgram();
    ExtendedZPlotProgram *zplotProgram = m_context->zplotProgram = new ExtendedZPlotProgram();
    if (!createProgram(edgeProgram, dir,
                       IRenderContext::kEdgeVertexShader,
                       IRenderContext::kEdgeWithSkinningVertexShader,
                       IRenderContext::kEdgeFragmentShader,
                       userData)) {
        return releaseUserData0(userData);
    }
    if (!createProgram(modelProgram, dir,
                       IRenderContext::kModelVertexShader,
                       IRenderContext::kModelWithSkinningVertexShader,
                       IRenderContext::kModelFragmentShader,
                       userData)) {
        return releaseUserData0(userData);
    }
    if (!createProgram(shadowProgram, dir,
                       IRenderContext::kShadowVertexShader,
                       IRenderContext::kShadowWithSkinningVertexShader,
                       IRenderContext::kShadowFragmentShader,
                       userData)) {
        return releaseUserData0(userData);
    }
    if (!createProgram(zplotProgram, dir,
                       IRenderContext::kZPlotVertexShader,
                       IRenderContext::kZPlotWithSkinningVertexShader,
                       IRenderContext::kZPlotFragmentShader,
                       userData)) {
        return releaseUserData0(userData);
    }
    if (!uploadMaterials(dir, userData)) {
        return releaseUserData0(userData);
    }
    VertexBundle &buffer = m_context->buffer;
    buffer.create(VertexBundle::kVertexBuffer, kModelDynamicVertexBufferEven, GL_DYNAMIC_DRAW, 0, m_context->dynamicBuffer->size());
    buffer.create(VertexBundle::kVertexBuffer, kModelDynamicVertexBufferOdd, GL_DYNAMIC_DRAW, 0, m_context->dynamicBuffer->size());
    VPVL2_VLOG(2, "Binding model dynamic vertex buffer to the vertex buffer object: size=" << m_context->dynamicBuffer->size());
    const IModel::StaticVertexBuffer *staticBuffer = m_context->staticBuffer;
    buffer.create(VertexBundle::kVertexBuffer, kModelStaticVertexBuffer, GL_STATIC_DRAW, 0, staticBuffer->size());
    buffer.bind(VertexBundle::kVertexBuffer, kModelStaticVertexBuffer);
    void *address = buffer.map(VertexBundle::kVertexBuffer, 0, staticBuffer->size());
    staticBuffer->update(address);
    VPVL2_VLOG(2, "Binding model static vertex buffer to the vertex buffer object: ptr=" << address << " size=" << staticBuffer->size());
    buffer.unmap(VertexBundle::kVertexBuffer, address);
    buffer.unbind(VertexBundle::kVertexBuffer);
    const IModel::IndexBuffer *indexBuffer = m_context->indexBuffer;
    buffer.create(VertexBundle::kIndexBuffer, kModelIndexBuffer, GL_STATIC_DRAW, indexBuffer->bytes(), indexBuffer->size());
    VPVL2_VLOG(2, "Binding indices to the vertex buffer object: ptr=" << indexBuffer->bytes() << " size=" << indexBuffer->size());
    VertexBundleLayout &bundleME = m_context->bundles[kVertexArrayObjectEven];
    if (bundleME.create() && bundleME.bind()) {
        VPVL2_VLOG(2, "Binding an vertex array object for even frame: " << bundleME.name());
        createVertexBundle(kModelDynamicVertexBufferEven);
    }
    VertexBundleLayout &bundleMO = m_context->bundles[kVertexArrayObjectOdd];
    if (bundleMO.create() && bundleMO.bind()) {
        VPVL2_VLOG(2, "Binding an vertex array object for odd frame: " << bundleMO.name());
        createVertexBundle(kModelDynamicVertexBufferOdd);
    }
    VertexBundleLayout &bundleEE = m_context->bundles[kEdgeVertexArrayObjectEven];
    if (bundleEE.create() && bundleEE.bind()) {
        VPVL2_VLOG(2, "Binding an edge vertex array object for even frame: " << bundleEE.name());
        createEdgeBundle(kModelDynamicVertexBufferEven);
    }
    VertexBundleLayout &bundleEO = m_context->bundles[kEdgeVertexArrayObjectOdd];
    if (bundleEO.create() && bundleEO.bind()) {
        VPVL2_VLOG(2, "Binding an edge vertex array object for odd frame: " << bundleEO.name());
        createEdgeBundle(kModelDynamicVertexBufferOdd);
    }
    buffer.unbind(VertexBundle::kVertexBuffer);
    buffer.unbind(VertexBundle::kIndexBuffer);
    VertexBundleLayout::unbindVertexArrayObject();
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable()) {
        const VertexBundle &buffer = m_context->buffer;
        cl::PMXAccelerator::Buffers &buffers = m_context->buffers;
        m_accelerator->release(buffers);
        buffers.append(cl::PMXAccelerator::Buffer(buffer.findName(kModelDynamicVertexBufferEven)));
        buffers.append(cl::PMXAccelerator::Buffer(buffer.findName(kModelDynamicVertexBufferOdd)));
        m_accelerator->upload(buffers, m_context->indexBuffer);
    }
#endif
    m_modelRef->setVisible(true);
    update(); // for updating even frame
    update(); // for updating odd frame
    VPVL2_VLOG(2, "Created the model: " << internal::cstr(m_modelRef->name(), 0));
    m_renderContextRef->stopProfileSession(IRenderContext::kProfileUploadModelProcess, m_modelRef);
    m_renderContextRef->releaseUserData(m_modelRef, userData);
    return ret;
}

void PMXRenderEngine::update()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_context)
        return;
    m_renderContextRef->startProfileSession(IRenderContext::kProfileUpdateModelProcess, m_modelRef);
    VertexBufferObjectType vbo = m_context->updateEven
            ? kModelDynamicVertexBufferEven : kModelDynamicVertexBufferOdd;
    IModel::DynamicVertexBuffer *dynamicBuffer = m_context->dynamicBuffer;
    m_context->buffer.bind(VertexBundle::kVertexBuffer, vbo);
    if (void *address = m_context->buffer.map(VertexBundle::kVertexBuffer, 0, dynamicBuffer->size())) {
        if (m_context->isVertexShaderSkinning) {
            m_context->matrixBuffer->update(address);
        }
        else {
            const ICamera *camera = m_sceneRef->camera();
            dynamicBuffer->update(address, camera->position(), m_context->aabbMin, m_context->aabbMax);
        }
        m_context->buffer.unmap(VertexBundle::kVertexBuffer, address);
    }
    m_context->buffer.unbind(VertexBundle::kVertexBuffer);
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable()) {
        const cl::PMXAccelerator::Buffer &buffer = m_context->buffers[m_context->updateEven ? 0 : 1];
        m_accelerator->update(dynamicBuffer, m_sceneRef, buffer, m_context->aabbMin, m_context->aabbMax);
    }
#endif
    m_modelRef->setAabb(m_context->aabbMin, m_context->aabbMax);
    m_context->updateEven = m_context->updateEven ? false :true;
    m_renderContextRef->stopProfileSession(IRenderContext::kProfileUpdateModelProcess, m_modelRef);
}

void PMXRenderEngine::setUpdateOptions(int options)
{
    if (m_context) {
        IModel::DynamicVertexBuffer *dynamicBuffer = m_context->dynamicBuffer;
        dynamicBuffer->setParallelUpdateEnable(internal::hasFlagBits(options, kParallelUpdate));
    }
}

void PMXRenderEngine::renderModel()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_context)
        return;
    m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderModelProcess, m_modelRef);
    ModelProgram *modelProgram = m_context->modelProgram;
    modelProgram->bind();
    float matrix4x4[16];
    m_renderContextRef->getMatrix(matrix4x4, m_modelRef,
                                  IRenderContext::kWorldMatrix
                                  | IRenderContext::kViewMatrix
                                  | IRenderContext::kProjectionMatrix
                                  | IRenderContext::kCameraMatrix);
    modelProgram->setModelViewProjectionMatrix(matrix4x4);
    m_renderContextRef->getMatrix(matrix4x4, m_modelRef,
                                  IRenderContext::kWorldMatrix
                                  | IRenderContext::kViewMatrix
                                  | IRenderContext::kCameraMatrix);
    modelProgram->setNormalMatrix(matrix4x4);
    m_renderContextRef->getMatrix(matrix4x4, m_modelRef,
                                  IRenderContext::kWorldMatrix
                                  | IRenderContext::kViewMatrix
                                  | IRenderContext::kProjectionMatrix
                                  | IRenderContext::kLightMatrix);
    modelProgram->setLightViewProjectionMatrix(matrix4x4);
    const ILight *light = m_sceneRef->light();
    GLuint textureID = 0;
    if (const IShadowMap *shadowMapRef = m_sceneRef->shadowMapRef()) {
        const void *texture = shadowMapRef->textureRef();
        textureID = texture ? *static_cast<const GLuint *>(texture) : 0;
        modelProgram->setDepthTextureSize(shadowMapRef->size());
    }
    modelProgram->setLightColor(light->color());
    modelProgram->setLightDirection(light->direction());
    modelProgram->setToonEnable(light->isToonEnabled());
    modelProgram->setCameraPosition(m_sceneRef->camera()->lookAt());
    const Scalar &opacity = m_modelRef->opacity();
    modelProgram->setOpacity(opacity);
    Array<IMaterial *> materials;
    m_modelRef->getMaterialRefs(materials);
    const int nmaterials = materials.count();
    const bool hasModelTransparent = !btFuzzyZero(opacity - 1.0f),
            isVertexShaderSkinning = m_context->isVertexShaderSkinning;
    const Vector3 &lc = light->color();
    bool &cullFaceState = m_context->cullFaceState;
    Color diffuse, specular;
    size_t offset = 0, size = m_context->indexBuffer->strideSize();
    bindVertexBundle();
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        const MaterialTextureRefs &materialPrivate = m_context->materialTextureRefs[i];
        const Color &ma = material->ambient(), &md = material->diffuse(), &ms = material->specular();
        diffuse.setValue(ma.x() + md.x() * lc.x(), ma.y() + md.y() * lc.y(), ma.z() + md.z() * lc.z(), md.w());
        specular.setValue(ms.x() * lc.x(), ms.y() * lc.y(), ms.z() * lc.z(), 1.0);
        modelProgram->setMaterialColor(diffuse);
        modelProgram->setMaterialSpecular(specular);
        modelProgram->setMaterialShininess(material->shininess());
        modelProgram->setMainTextureBlend(material->mainTextureBlend());
        modelProgram->setSphereTextureBlend(material->sphereTextureBlend());
        modelProgram->setToonTextureBlend(material->toonTextureBlend());
        modelProgram->setMainTexture(materialPrivate.mainTextureRef);
        modelProgram->setSphereTexture(materialPrivate.sphereTextureRef, material->sphereTextureRenderMode());
        modelProgram->setToonTexture(materialPrivate.toonTextureRef);
        if (textureID && material->isSelfShadowEnabled())
            modelProgram->setDepthTexture(textureID);
        else
            modelProgram->setDepthTexture(0);
        if (isVertexShaderSkinning) {
            IModel::MatrixBuffer *matrixBuffer = m_context->matrixBuffer;
            modelProgram->setBoneMatrices(matrixBuffer->bytes(i), matrixBuffer->size(i));
        }
        if (!hasModelTransparent && cullFaceState && material->isCullingDisabled()) {
            glDisable(GL_CULL_FACE);
            cullFaceState = false;
        }
        else if (!cullFaceState && !material->isCullingDisabled()) {
            glEnable(GL_CULL_FACE);
            cullFaceState = true;
        }
        const int nindices = material->indexRange().count;
        m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderModelMaterialDrawCall, material);
        glDrawElements(GL_TRIANGLES, nindices, m_context->indexType, reinterpret_cast<const GLvoid *>(offset));
        m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderModelMaterialDrawCall, material);
        offset += nindices * size;
    }
    unbindVertexBundle();
    modelProgram->unbind();
    if (!cullFaceState) {
        glEnable(GL_CULL_FACE);
        cullFaceState = true;
    }
    m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderModelProcess, m_modelRef);
}

void PMXRenderEngine::renderShadow()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_context)
        return;
    m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderShadowProcess, m_modelRef);
    ShadowProgram *shadowProgram = m_context->shadowProgram;
    shadowProgram->bind();
    float matrix4x4[16];
    m_renderContextRef->getMatrix(matrix4x4, m_modelRef,
                                  IRenderContext::kWorldMatrix
                                  | IRenderContext::kViewMatrix
                                  | IRenderContext::kProjectionMatrix
                                  | IRenderContext::kShadowMatrix);
    shadowProgram->setModelViewProjectionMatrix(matrix4x4);
    const ILight *light = m_sceneRef->light();
    shadowProgram->setLightColor(light->color());
    shadowProgram->setLightDirection(light->direction());
    Array<IMaterial *> materials;
    m_modelRef->getMaterialRefs(materials);
    const int nmaterials = materials.count();
    const bool isVertexShaderSkinning = m_context->isVertexShaderSkinning;
    size_t offset = 0, size = m_context->indexBuffer->strideSize();
    bindVertexBundle();
    glDisable(GL_CULL_FACE);
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        const int nindices = material->indexRange().count;
        if (material->hasShadow()) {
            if (isVertexShaderSkinning) {
                IModel::MatrixBuffer *matrixBuffer = m_context->matrixBuffer;
                shadowProgram->setBoneMatrices(matrixBuffer->bytes(i), matrixBuffer->size(i));
            }
            m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderShadowMaterialDrawCall, material);
            glDrawElements(GL_TRIANGLES, nindices, m_context->indexType, reinterpret_cast<const GLvoid *>(offset));
            m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderShadowMaterialDrawCall, material);
        }
        offset += nindices * size;
    }
    unbindVertexBundle();
    glEnable(GL_CULL_FACE);
    shadowProgram->unbind();
    m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderShadowProcess, m_modelRef);
}

void PMXRenderEngine::renderEdge()
{
    if (!m_modelRef || !m_modelRef->isVisible() || btFuzzyZero(Scalar(m_modelRef->edgeWidth())) || !m_context)
        return;
    m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderEdgeProcess, m_modelRef);
    EdgeProgram *edgeProgram = m_context->edgeProgram;
    edgeProgram->bind();
    float matrix4x4[16];
    const Scalar &opacity = m_modelRef->opacity();
    m_renderContextRef->getMatrix(matrix4x4, m_modelRef,
                                  IRenderContext::kWorldMatrix
                                  | IRenderContext::kViewMatrix
                                  | IRenderContext::kProjectionMatrix
                                  | IRenderContext::kCameraMatrix);
    edgeProgram->setModelViewProjectionMatrix(matrix4x4);
    edgeProgram->setOpacity(opacity);
    Array<IMaterial *> materials;
    m_modelRef->getMaterialRefs(materials);
    const int nmaterials = materials.count();
    const bool isVertexShaderSkinning = m_context->isVertexShaderSkinning;
    IVertex::EdgeSizePrecision edgeScaleFactor = 0;
    if (isVertexShaderSkinning) {
        const ICamera *camera = m_sceneRef->camera();
        edgeScaleFactor = m_modelRef->edgeScaleFactor(camera->position());
    }
    size_t offset = 0, size = m_context->indexBuffer->strideSize();
    bool isOpaque = btFuzzyZero(opacity - 1);
    if (isOpaque)
        glDisable(GL_BLEND);
    glCullFace(GL_FRONT);
    bindEdgeBundle();
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        const int nindices = material->indexRange().count;
        edgeProgram->setColor(material->edgeColor());
        if (material->isEdgeEnabled()) {
            if (isVertexShaderSkinning) {
                IModel::MatrixBuffer *matrixBuffer = m_context->matrixBuffer;
                edgeProgram->setBoneMatrices(matrixBuffer->bytes(i), matrixBuffer->size(i));
                edgeProgram->setSize(Scalar(material->edgeSize() * edgeScaleFactor));
            }
            m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderEdgeMateiralDrawCall, material);
            glDrawElements(GL_TRIANGLES, nindices, m_context->indexType, reinterpret_cast<const GLvoid *>(offset));
            m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderEdgeMateiralDrawCall, material);
        }
        offset += nindices * size;
    }
    unbindVertexBundle();
    glCullFace(GL_BACK);
    if (isOpaque)
        glEnable(GL_BLEND);
    edgeProgram->unbind();
    m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderEdgeProcess, m_modelRef);
}

void PMXRenderEngine::renderZPlot()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_context)
        return;
    m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderZPlotProcess, m_modelRef);
    ExtendedZPlotProgram *zplotProgram = m_context->zplotProgram;
    zplotProgram->bind();
    float matrix4x4[16];
    m_renderContextRef->getMatrix(matrix4x4, m_modelRef,
                                  IRenderContext::kWorldMatrix
                                  | IRenderContext::kViewMatrix
                                  | IRenderContext::kProjectionMatrix
                                  | IRenderContext::kLightMatrix);
    zplotProgram->setModelViewProjectionMatrix(matrix4x4);
    Array<IMaterial *> materials;
    m_modelRef->getMaterialRefs(materials);
    const int nmaterials = materials.count();
    const bool isVertexShaderSkinning = m_context->isVertexShaderSkinning;
    size_t offset = 0, size = m_context->indexBuffer->strideSize();
    bindVertexBundle();
    glDisable(GL_CULL_FACE);
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        const int nindices = material->indexRange().count;
        if (material->hasShadowMap()) {
            if (isVertexShaderSkinning) {
                IModel::MatrixBuffer *matrixBuffer = m_context->matrixBuffer;
                zplotProgram->setBoneMatrices(matrixBuffer->bytes(i), matrixBuffer->size(i));
            }
            m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderZPlotMaterialDrawCall, material);
            glDrawElements(GL_TRIANGLES, nindices, m_context->indexType, reinterpret_cast<const GLvoid *>(offset));
            m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderZPlotMaterialDrawCall, material);
        }
        offset += nindices * size;
    }
    unbindVertexBundle();
    glEnable(GL_CULL_FACE);
    zplotProgram->unbind();
    m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderZPlotProcess, m_modelRef);
}

bool PMXRenderEngine::hasPreProcess() const
{
    return false;
}

bool PMXRenderEngine::hasPostProcess() const
{
    return false;
}

void PMXRenderEngine::preparePostProcess()
{
    /* do nothing */
}

void PMXRenderEngine::performPreProcess()
{
    /* do nothing */
}

void PMXRenderEngine::performPostProcess(IEffect * /* nextPostEffect */)
{
    /* do nothing */
}

IEffect *PMXRenderEngine::effectRef(IEffect::ScriptOrderType /* type */) const
{
    return 0;
}

void PMXRenderEngine::setEffect(IEffect::ScriptOrderType /* type */, IEffect * /* effect */, const IString * /* dir */)
{
    /* do nothing */
}

bool PMXRenderEngine::createProgram(BaseShaderProgram *program,
                                    const IString *dir,
                                    IRenderContext::ShaderType vertexShaderType,
                                    IRenderContext::ShaderType vertexSkinningShaderType,
                                    IRenderContext::ShaderType fragmentShaderType,
                                    void *userData)
{
    IString *vertexShaderSource = 0;
    IString *fragmentShaderSource = 0;
    if (m_context->isVertexShaderSkinning) {
        vertexShaderSource = m_renderContextRef->loadShaderSource(vertexSkinningShaderType, m_modelRef, dir, userData);
    }
    else {
        vertexShaderSource = m_renderContextRef->loadShaderSource(vertexShaderType, m_modelRef, dir, userData);
    }
    fragmentShaderSource = m_renderContextRef->loadShaderSource(fragmentShaderType, m_modelRef, dir, userData);
    program->addShaderSource(vertexShaderSource, GL_VERTEX_SHADER);
    program->addShaderSource(fragmentShaderSource, GL_FRAGMENT_SHADER);
    bool ok = program->linkProgram();
    delete vertexShaderSource;
    delete fragmentShaderSource;
    return ok;
}

bool PMXRenderEngine::uploadMaterials(const IString *dir, void *userData)
{
    Array<IMaterial *> materials;
    m_modelRef->getMaterialRefs(materials);
    const int nmaterials = materials.count();
    IRenderContext::Texture texture(IRenderContext::kTexture2D);
    m_context->materialTextureRefs.resize(nmaterials);
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        const IString *name = material->name(); (void) name;
        const int materialIndex = material->index(); (void) materialIndex;
        MaterialTextureRefs &materialPrivate = m_context->materialTextureRefs[i];
        texture.toon = false;
        if (const IString *mainTexturePath = material->mainTexture()) {
            if (m_renderContextRef->uploadTexture(mainTexturePath, dir, texture, userData)) {
                ITexture *textureRef = texture.texturePtrRef;
                materialPrivate.mainTextureRef = m_context->allocatedTextures.insert(textureRef, textureRef);
                VPVL2_VLOG(2, "Binding the texture as a main texture (material=" << internal::cstr(name, "(null)") << " index=" << materialIndex << " ID=" << texture.texturePtrRef << ")");
            }
            else {
                VPVL2_LOG(WARNING, "Cannot bind a main texture (material=" << internal::cstr(name, "(null)") << " index=" << materialIndex << ")");
                return false;
            }
        }
        if (const IString *sphereTexturePath = material->sphereTexture()) {
            if (m_renderContextRef->uploadTexture(sphereTexturePath, dir, texture, userData)) {
                ITexture *textureRef = texture.texturePtrRef;
                materialPrivate.sphereTextureRef = m_context->allocatedTextures.insert(textureRef, textureRef);
                VPVL2_VLOG(2, "Binding the texture as a sphere texture: material=" << internal::cstr(name, "(null)") << " index=" << materialIndex << " ID=" << texture.texturePtrRef);
            }
            else {
                VPVL2_LOG(WARNING, "Cannot bind a sphere texture: material=" << internal::cstr(name, "(null)") << " index=" << materialIndex);
                return false;
            }
        }
        texture.toon = true;
        if (material->isSharedToonTextureUsed()) {
            char buf[16];
            internal::snprintf(buf, sizeof(buf), "toon%02d.bmp", material->toonTextureIndex() + 1);
            IString *s = m_renderContextRef->toUnicode(reinterpret_cast<const uint8_t *>(buf));
            bool ret = m_renderContextRef->uploadTexture(s, 0, texture, userData);
            delete s;
            if (ret) {
                ITexture *textureRef = texture.texturePtrRef;
                materialPrivate.toonTextureRef = m_context->allocatedTextures.insert(textureRef, textureRef);
                VPVL2_VLOG(2, "Binding the texture as a shared toon texture: material=" << internal::cstr(name, "(null)") << " index=" << materialIndex << " ID=" << texture.texturePtrRef);
            }
            else {
                VPVL2_LOG(WARNING, "Cannot bind a shared toon texture: material=" << internal::cstr(name, "(null)") << " index=" << materialIndex);
                return false;
            }
        }
        else if (const IString *toonTexturePath = material->toonTexture()) {
            if (m_renderContextRef->uploadTexture(toonTexturePath, dir, texture, userData)) {
                ITexture *textureRef = texture.texturePtrRef;
                materialPrivate.toonTextureRef = m_context->allocatedTextures.insert(textureRef, textureRef);
                VPVL2_VLOG(2, "Binding the texture as a toon texture: material=" << internal::cstr(name, "(null)") << " index=" << materialIndex << " ID=" << texture.texturePtrRef);
            }
            else {
                VPVL2_LOG(WARNING, "Cannot bind a toon texture: material=" << internal::cstr(name, "(null)") << " index=" << materialIndex);
                return false;
            }
        }
    }
    return true;
}

bool PMXRenderEngine::releaseUserData0(void *userData)
{
    if (m_context) {
        delete m_context;
        m_context = 0;
    }
    m_renderContextRef->releaseUserData(m_modelRef, userData);
    return false;
}

void PMXRenderEngine::createVertexBundle(GLuint dvbo)
{
    VertexBundle &buffer = m_context->buffer;
    buffer.bind(VertexBundle::kVertexBuffer, dvbo);
    bindDynamicVertexAttributePointers();
    buffer.bind(VertexBundle::kVertexBuffer, kModelStaticVertexBuffer);
    bindStaticVertexAttributePointers();
    buffer.bind(VertexBundle::kIndexBuffer, kModelIndexBuffer);
    glEnableVertexAttribArray(IModel::Buffer::kVertexStride);
    glEnableVertexAttribArray(IModel::Buffer::kNormalStride);
    glEnableVertexAttribArray(IModel::Buffer::kTextureCoordStride);
    glEnableVertexAttribArray(IModel::Buffer::kUVA0Stride);
    glEnableVertexAttribArray(IModel::Buffer::kUVA1Stride);
    VertexBundleLayout::unbindVertexArrayObject();
}

void PMXRenderEngine::createEdgeBundle(GLuint dvbo)
{
    VertexBundle &buffer = m_context->buffer;
    buffer.bind(VertexBundle::kVertexBuffer, dvbo);
    bindEdgeVertexAttributePointers();
    buffer.bind(VertexBundle::kVertexBuffer, kModelStaticVertexBuffer);
    bindStaticVertexAttributePointers();
    buffer.bind(VertexBundle::kIndexBuffer, kModelIndexBuffer);
    glEnableVertexAttribArray(IModel::Buffer::kVertexStride);
    VertexBundleLayout::unbindVertexArrayObject();
}

void PMXRenderEngine::bindVertexBundle()
{
    VertexArrayObjectType vao;
    VertexBufferObjectType vbo;
    m_context->getVertexBundleType(vao, vbo);
    if (!m_context->bundles[vao].bind()) {
        VertexBundle &buffer = m_context->buffer;
        buffer.bind(VertexBundle::kVertexBuffer, vbo);
        bindDynamicVertexAttributePointers();
        buffer.bind(VertexBundle::kVertexBuffer, kModelStaticVertexBuffer);
        bindStaticVertexAttributePointers();
        buffer.bind(VertexBundle::kIndexBuffer, kModelIndexBuffer);
    }
}

void PMXRenderEngine::bindEdgeBundle()
{
    VertexArrayObjectType vao;
    VertexBufferObjectType vbo;
    m_context->getEdgeBundleType(vao, vbo);
    if (!m_context->bundles[vao].bind()) {
        VertexBundle &buffer = m_context->buffer;
        buffer.bind(VertexBundle::kVertexBuffer, vbo);
        bindEdgeVertexAttributePointers();
        buffer.bind(VertexBundle::kVertexBuffer, kModelStaticVertexBuffer);
        bindStaticVertexAttributePointers();
        buffer.bind(VertexBundle::kIndexBuffer, kModelIndexBuffer);
    }
}

void PMXRenderEngine::unbindVertexBundle()
{
    if (!VertexBundleLayout::unbindVertexArrayObject()) {
        VertexBundle &buffer = m_context->buffer;
        buffer.unbind(VertexBundle::kVertexBuffer);
        buffer.unbind(VertexBundle::kIndexBuffer);
    }
}

void PMXRenderEngine::bindDynamicVertexAttributePointers()
{
    const IModel::DynamicVertexBuffer *dynamicBuffer = m_context->dynamicBuffer;
    size_t offset, size;
    offset = dynamicBuffer->strideOffset(IModel::DynamicVertexBuffer::kVertexStride);
    size   = dynamicBuffer->strideSize();
    glVertexAttribPointer(IModel::Buffer::kVertexStride, 3, GL_FLOAT, GL_FALSE,
                          size, reinterpret_cast<const GLvoid *>(offset));
    offset = dynamicBuffer->strideOffset(IModel::DynamicVertexBuffer::kNormalStride);
    glVertexAttribPointer(IModel::Buffer::kNormalStride, 3, GL_FLOAT, GL_FALSE,
                          size, reinterpret_cast<const GLvoid *>(offset));
    offset = dynamicBuffer->strideOffset(IModel::DynamicVertexBuffer::kUVA0Stride);
    glVertexAttribPointer(IModel::Buffer::kUVA0Stride, 4, GL_FLOAT, GL_FALSE,
                          size, reinterpret_cast<const GLvoid *>(offset));
    offset = dynamicBuffer->strideOffset(IModel::DynamicVertexBuffer::kUVA1Stride);
    glVertexAttribPointer(IModel::Buffer::kUVA1Stride, 4, GL_FLOAT, GL_FALSE,
                          size, reinterpret_cast<const GLvoid *>(offset));
}

void PMXRenderEngine::bindEdgeVertexAttributePointers()
{
    const IModel::DynamicVertexBuffer *dynamicBuffer = m_context->dynamicBuffer;
    size_t offset, size;
    offset = dynamicBuffer->strideOffset(IModel::DynamicVertexBuffer::kEdgeVertexStride);
    size   = dynamicBuffer->strideSize();
    glVertexAttribPointer(IModel::Buffer::kVertexStride, 3, GL_FLOAT, GL_FALSE,
                          size, reinterpret_cast<const GLvoid *>(offset));
    if (m_context->isVertexShaderSkinning) {
        offset = dynamicBuffer->strideOffset(IModel::DynamicVertexBuffer::kNormalStride);
        glVertexAttribPointer(IModel::Buffer::kNormalStride, 3, GL_FLOAT, GL_FALSE,
                              size, reinterpret_cast<const GLvoid *>(offset));
        glVertexAttribPointer(IModel::Buffer::kEdgeSizeStride, 4, GL_FLOAT, GL_FALSE,
                              size, reinterpret_cast<const GLvoid *>(offset));
    }
}

void PMXRenderEngine::bindStaticVertexAttributePointers()
{
    const IModel::StaticVertexBuffer *staticBuffer = m_context->staticBuffer;
    size_t offset, size;
    offset = staticBuffer->strideOffset(IModel::StaticVertexBuffer::kTextureCoordStride);
    size   = staticBuffer->strideSize();
    glVertexAttribPointer(IModel::Buffer::kTextureCoordStride, 2, GL_FLOAT, GL_FALSE,
                          size, reinterpret_cast<const GLvoid *>(offset));
    if (m_context->isVertexShaderSkinning) {
        offset = staticBuffer->strideOffset(IModel::StaticVertexBuffer::kBoneIndexStride);
        glVertexAttribPointer(IModel::Buffer::kBoneIndexStride, 4, GL_FLOAT, GL_FALSE,
                              size, reinterpret_cast<const GLvoid *>(offset));
        offset = staticBuffer->strideOffset(IModel::StaticVertexBuffer::kBoneWeightStride);
        glVertexAttribPointer(IModel::Buffer::kBoneWeightStride, 4, GL_FLOAT, GL_FALSE,
                              size, reinterpret_cast<const GLvoid *>(offset));
    }
}

} /* namespace gl2 */
} /* namespace vpvl2 */

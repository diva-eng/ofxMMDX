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

#include <vpvl2/extensions/BaseRenderContext.h>

/* libvpvl2 */
#include <vpvl2/vpvl2.h>
#include <vpvl2/internal/util.h>
#include <vpvl2/extensions/gl/FrameBufferObject.h>
#include <vpvl2/extensions/gl/SimpleShadowMap.h>
#include <vpvl2/extensions/gl/Texture2D.h>
#include <vpvl2/extensions/icu4c/StringMap.h>

#ifdef VPVL2_ENABLE_EXTENSION_ARCHIVE
#include <vpvl2/extensions/Archive.h>
#endif

/* STL */
#include <fstream>
#include <iostream>
#include <sstream>
#include <set>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif

/* stb_image.c as a header */
#if defined(VPVL2_LINK_NVTT) && !defined(BUILD_SHARED_LIBS)
#define STBI_HEADER_FILE_ONLY
#endif
#define STBI_NO_STDIO
#include "stb_image.c"

/* GLI */
#include <gli/gli.hpp>

#ifdef __clang__
#pragma clang diagnostic pop
#endif

/* GLM */
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/* NVTT */
#ifdef VPVL2_LINK_NVTT
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif
#include <nvcore/Stream.h>
#include <nvcore/Timer.h>
#include <nvimage/DirectDrawSurface.h>
#include <nvimage/Image.h>
#include <nvimage/ImageIO.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif

/* Cg and ICU */
#ifdef VPVL2_ENABLE_NVIDIA_CG
#include <vpvl2/extensions/cg/Util.h>
#include <unicode/regex.h>
#endif

namespace {

static const char *DebugMessageSourceToString(GLenum value)
{
    switch (value) {
    case GL_DEBUG_SOURCE_API:
        return "OpenGL";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        return "Window";
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        return "ShaderCompiler";
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        return "ThirdParty";
    case GL_DEBUG_SOURCE_APPLICATION:
        return "Application";
    case GL_DEBUG_SOURCE_OTHER:
        return "Other";
    default:
        return "Unknown";
    }
}

static const char *DebugMessageTypeToString(GLenum value)
{
    switch (value) {
    case GL_DEBUG_TYPE_ERROR:
        return "Error";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        return "DeprecatedBehavior";
    case GL_DEBUG_TYPE_PORTABILITY:
        return "Portability";
    case GL_DEBUG_TYPE_PERFORMANCE:
        return "Performance";
    case GL_DEBUG_TYPE_OTHER:
        return "Other";
    default:
        return "Unknown";
    }
}

}

namespace vpvl2
{
namespace extensions
{
using namespace gl;
using namespace icu4c;

BaseRenderContext::ModelContext::ModelContext(BaseRenderContext *renderContextRef)
    : m_renderContextRef(renderContextRef)
{
}

BaseRenderContext::ModelContext::~ModelContext()
{
    m_renderContextRef = 0;
}

void BaseRenderContext::ModelContext::addTextureCache(const UnicodeString &path, ITexture *cache)
{
    m_textureRefCache.insert(std::make_pair(path, cache));
}

bool BaseRenderContext::ModelContext::findTextureCache(const UnicodeString &path, Texture &texture) const
{
    TextureCacheMap::const_iterator it = m_textureRefCache.find(path);
    if (it != m_textureRefCache.end()) {
        texture.texturePtrRef = it->second;
        texture.ok = true;
        return true;
    }
    return false;
}

bool BaseRenderContext::ModelContext::cacheTexture(ITexture *textureRef, Texture &texture, const UnicodeString &path)
{
    bool ok = texture.ok = textureRef != 0;
    if (textureRef) {
        glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(textureRef->data()));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        if (texture.toon) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        texture.texturePtrRef = textureRef;
        addTextureCache(path, textureRef);
    }
    return ok;
}

int BaseRenderContext::ModelContext::countCachedTextures() const
{
    return m_textureRefCache.size();
}

ITexture *BaseRenderContext::ModelContext::createTexture(const void *ptr,
                                                         const BaseSurface::Format &format,
                                                         const Vector3 &size,
                                                         bool mipmap,
                                                         bool canOptimize) const
{
    glBindTexture(format.target, 0);
    Texture2D *texture = new (std::nothrow) Texture2D(format, size, 0);
    if (texture) {
        texture->create();
        texture->bind();
        if (GLEW_ARB_texture_storage) {
            glTexStorage2D(format.target, 1, format.internal, GLsizei(size.x()), GLsizei(size.y()));
            glTexSubImage2D(format.target, 0, 0, 0, GLsizei(size.x()), GLsizei(size.y()), format.external, format.type, ptr);
        }
        else {
#if defined(GL_APPLE_client_storage) && defined(GL_APPLE_texture_range)
            if (canOptimize) {
                glTexParameteri(format.target, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_CACHED_APPLE);
                glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
            }
#endif
            glTexImage2D(format.target, 0, format.internal, GLsizei(size.x()), GLsizei(size.y()), 0, format.external, format.type, ptr);
        }
        if (mipmap) {
            generateMipmap(format.target);
        }
        texture->unbind();
    }
    return texture;
}

ITexture *BaseRenderContext::ModelContext::createTexture(const uint8_t *data, size_t size, bool mipmap)
{
    Vector3 textureSize;
    ITexture *texturePtr = 0;
    int x = 0, y = 0, ncomponents = 0;
    /* Loading major image format (BMP/JPG/PNG/TGA) texture with stb_image.c */
    if (stbi_uc *ptr = stbi_load_from_memory(data, size, &x, &y, &ncomponents, 4)) {
        textureSize.setValue(Scalar(x), Scalar(y), 1);
        BaseSurface::Format format(GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE, GL_TEXTURE_2D);
        texturePtr = createTexture(ptr, format, textureSize, mipmap, false);
        stbi_image_free(ptr);
    }
    return texturePtr;
}

void BaseRenderContext::ModelContext::generateMipmap(GLenum target) const
{
#ifdef VPVL2_LINK_GLEW
    if (GLEW_ARB_framebuffer_object) {
        glGenerateMipmap(target);
    }
#elif !defined(VPVL2_ENABLE_GLES2)
    const void *procs[] = { "glGenerateMipmap", "glGenerateMipmapEXT", 0 };
    typedef void (*glGenerateMipmapProcPtr)(GLuint);
    if (glGenerateMipmapProcPtr glGenerateMipmapProcPtrRef = reinterpret_cast<glGenerateMipmapProcPtr>(m_renderContextRef->findProcedureAddress(procs)))
        glGenerateMipmapProcPtrRef(target);
#endif /* VPVL2_LINK_GLEW */
}

bool BaseRenderContext::ModelContext::uploadTextureFile(const UnicodeString &path, Texture &texture)
{
    if (path[path.length() - 1] == '/' || findTextureCache(path, texture)) {
        VPVL2_VLOG(2, String::toStdString(path) << " is already cached, skipped.");
        return true;
    }
    ITexture *texturePtr = 0;
    Vector3 size;
    MapBuffer buffer(m_renderContextRef);
    /* Loading DDS texture with GLI */
    if (path.endsWith(".dds")) {
        gli::texture2D tex(gli::loadStorageDDS(String::toStdString(path).c_str()));
        if (!tex.empty()) {
            const gli::texture2D::format_type &fmt = tex.format();
            const gli::texture2D::dimensions_type &dim = tex.dimensions();
            BaseSurface::Format format(gli::external_format(fmt), gli::internal_format(fmt), gli::type_format(fmt), GL_TEXTURE_2D);
            size.setValue(Scalar(dim.x), Scalar(dim.y), 1);
            if (gli::is_compressed(fmt)) {
                Texture2D *texturePtr2 = new (std::nothrow) Texture2D(format, size, 0);
                texturePtr2->create();
                texturePtr2->bind();
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glCompressedTexImage2D(GL_TEXTURE_2D, 0, format.internal,
                                       GLsizei(size.x()), GLsizei(size.y()), 0, tex[0].size(), tex[0].data());
                texturePtr2->unbind();
                texturePtr = texturePtr2;
            }
            else {
                const void *ptr = tex[0].data();
                texturePtr = createTexture(ptr, format, size, texture.mipmap, false);
                if (!texturePtr) {
                    VPVL2_LOG(WARNING, "Cannot load texture from " << String::toStdString(path) << ": " << stbi_failure_reason());
                    return false;
                }
            }
        }
    }
    /* Loading major image format (BMP/JPG/PNG/TGA) texture with stb_image.c */
    else if (m_renderContextRef->mapFile(path, &buffer)) {
        texturePtr = createTexture(buffer.address, buffer.size, texture.mipmap);
        if (!texturePtr) {
            VPVL2_LOG(WARNING, "Cannot load texture from " << String::toStdString(path) << ": " << stbi_failure_reason());
            return false;
        }
    }
    return cacheTexture(texturePtr, texture, path);
}

bool BaseRenderContext::ModelContext::uploadTextureData(const uint8_t *data, size_t size, const UnicodeString &key, Texture &texture)
{
    if (findTextureCache(key, texture)) {
        VPVL2_VLOG(2, String::toStdString(key) << " is already cached, skipped.");
        return true;
    }
    ITexture *texturePtr = createTexture(data, size, texture.mipmap);
    if (!texturePtr) {
        VPVL2_LOG(WARNING, "Cannot load texture with key " << String::toStdString(key) << ": " << stbi_failure_reason());
        return false;
    }
    return cacheTexture(texturePtr, texture, key);
}

BaseRenderContext::BaseRenderContext(Scene *sceneRef, IEncoding *encodingRef, const StringMap *configRef)
    : m_configRef(configRef),
      m_sceneRef(sceneRef),
      m_encodingRef(encodingRef),
      m_archive(0),
      m_renderColorFormat(GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE, GL_TEXTURE_2D),
      m_lightWorldMatrix(1),
      m_lightViewMatrix(1),
      m_lightProjectionMatrix(1),
      m_cameraWorldMatrix(1),
      m_cameraViewMatrix(1),
      m_cameraProjectionMatrix(1),
      m_textureSampler(0),
      m_toonTextureSampler(0)
    #ifdef VPVL2_ENABLE_NVIDIA_CG
    ,
      m_effectPathPtr(0),
      m_msaaSamples(0)
    #endif /* VPVL2_ENABLE_NVIDIA_CG */
{
}

void BaseRenderContext::initialize(bool enableDebug)
{
    std::istringstream in(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)));
    std::string extension;
    while (std::getline(in, extension, ' ')) {
        m_extensions.insert(extension);
    }
    // const GLubyte *shaderVersionString = glGetString(GL_SHADING_LANGUAGE_VERSION);
    if (GLEW_ARB_debug_output && enableDebug) {
        glDebugMessageCallbackARB(reinterpret_cast<GLDEBUGPROC>(&BaseRenderContext::debugMessageCallback), this);
    }
    if (GLEW_ARB_sampler_objects) {
        glGenSamplers(1, &m_textureSampler);
        glGenSamplers(1, &m_toonTextureSampler);
        glSamplerParameteri(m_textureSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(m_textureSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glSamplerParameteri(m_toonTextureSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(m_toonTextureSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glSamplerParameteri(m_toonTextureSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glSamplerParameteri(m_toonTextureSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
#ifdef VPVL2_ENABLE_NVIDIA_CG
    glGetIntegerv(GL_MAX_SAMPLES, &m_msaaSamples);
#endif /* VPVL2_ENABLE_NVIDIA_CG */
}

BaseRenderContext::~BaseRenderContext()
{
    release();
    m_encodingRef = 0;
#ifdef VPVL2_ENABLE_NVIDIA_CG
    /* m_msaaSamples must not set zero at #release(), it causes multiple post effect will be lost */
    m_msaaSamples = 0;
#endif
}

void BaseRenderContext::allocateUserData(const IModel *model, void *&context)
{
    (void) model;
    ModelContext *ctx = new ModelContext(this);
    VPVL2_VLOG(2, "This model has " << model->count(IModel::kTextures) << " textures.");
    context = ctx;
}

void BaseRenderContext::releaseUserData(const IModel * /* model */, void *&context)
{
    ModelContext *ctx = static_cast<ModelContext *>(context);
    VPVL2_VLOG(2, ctx->countCachedTextures() << " textures is loaded.");
    delete ctx;
    context = 0;
}

bool BaseRenderContext::uploadTexture(const IString *name, const IString *dir, Texture &texture, void *context)
{
    bool ret = false;
    texture.texturePtrRef = 0;
    texture.system = false;
    if (texture.toon) {
        if (dir) {
            const UnicodeString &path = createPath(dir, name);
            VPVL2_VLOG(2, "Loading a model toon texture: " << String::toStdString(path).c_str());
            ret = uploadTextureInternal(path, texture, context);
        }
        else {
            // force loading system toon texture
            texture.ok = false;
        }
        if (!texture.ok) {
            String s(toonDirectory());
            const UnicodeString &path = createPath(&s, name);
            VPVL2_VLOG(2, "Loading a system toon texture: " << String::toStdString(path).c_str());
            texture.system = true;
            ret = uploadTextureInternal(path, texture, context);
        }
    }
    else {
        const UnicodeString &path = createPath(dir, name);
        VPVL2_VLOG(2, "Loading a model texture: " << String::toStdString(path).c_str());
        ret = uploadTextureInternal(path, texture, context);
    }
    return ret;
}

void BaseRenderContext::getMatrix(float value[], const IModel *model, int flags) const
{
    glm::mat4x4 m(1);
    if (internal::hasFlagBits(flags, IRenderContext::kShadowMatrix)) {
        if (internal::hasFlagBits(flags, IRenderContext::kProjectionMatrix)) {
            m *= m_cameraProjectionMatrix;
        }
        if (internal::hasFlagBits(flags, IRenderContext::kViewMatrix)) {
            m *= m_cameraViewMatrix;
        }
        if (model && internal::hasFlagBits(flags, IRenderContext::kWorldMatrix)) {
            static const Vector3 plane(0.0f, 1.0f, 0.0f);
            const ILight *light = m_sceneRef->light();
            const Vector3 &direction = light->direction();
            const Scalar dot = plane.dot(-direction);
            float matrix[16];
            for (int i = 0; i < 4; i++) {
                int offset = i * 4;
                for (int j = 0; j < 4; j++) {
                    int index = offset + j;
                    matrix[index] = plane[i] * direction[j];
                    if (i == j)
                        matrix[index] += dot;
                }
            }
            m *= glm::make_mat4x4(matrix);
            m *= m_cameraWorldMatrix;
            m = glm::scale(m, glm::vec3(model->scaleFactor()));
        }
    }
    else if (internal::hasFlagBits(flags, IRenderContext::kCameraMatrix)) {
        if (internal::hasFlagBits(flags, IRenderContext::kProjectionMatrix)) {
            m *= m_cameraProjectionMatrix;
        }
        if (internal::hasFlagBits(flags, IRenderContext::kViewMatrix)) {
            m *= m_cameraViewMatrix;
        }
        if (model && internal::hasFlagBits(flags, IRenderContext::kWorldMatrix)) {
            const IBone *bone = model->parentBoneRef();
            Transform transform;
            transform.setOrigin(model->worldPosition());
            transform.setRotation(model->worldRotation());
            Scalar matrix[16];
            transform.getOpenGLMatrix(matrix);
            m *= glm::make_mat4x4(matrix);
            if (bone) {
                transform = bone->worldTransform();
                transform.getOpenGLMatrix(matrix);
                m *= glm::make_mat4x4(matrix);
            }
            m *= m_cameraWorldMatrix;
            m = glm::scale(m, glm::vec3(model->scaleFactor()));
        }
    }
    else if (internal::hasFlagBits(flags, IRenderContext::kLightMatrix)) {
        if (internal::hasFlagBits(flags, IRenderContext::kWorldMatrix)) {
            m *= m_lightWorldMatrix;
            m = glm::scale(m, glm::vec3(model->scaleFactor()));
        }
        if (internal::hasFlagBits(flags, IRenderContext::kProjectionMatrix)) {
            m *= m_lightProjectionMatrix;
        }
        if (internal::hasFlagBits(flags, IRenderContext::kViewMatrix)) {
            m *= m_lightViewMatrix;
        }
    }
    if (internal::hasFlagBits(flags, IRenderContext::kInverseMatrix)) {
        m = glm::inverse(m);
    }
    if (internal::hasFlagBits(flags, IRenderContext::kTransposeMatrix)) {
        m = glm::transpose(m);
    }
    memcpy(value, glm::value_ptr(m), sizeof(float) * 16);
}

IString *BaseRenderContext::loadShaderSource(ShaderType type, const IModel *model, const IString *dir, void * /* context */)
{
    std::string file;
#ifdef VPVL2_ENABLE_NVIDIA_CG
    if (type == kModelEffectTechniques) {
        const IString *path = effectFilePath(model, dir);
        return loadShaderSource(type, path);
    }
#else
    (void) dir;
#endif /* VPVL2_ENABLE_NVIDIA_CG */
    switch (model->type()) {
    case IModel::kAssetModel:
        file += "/asset/";
        break;
    case IModel::kPMDModel:
    case IModel::kPMXModel:
        file += "/pmx/";
        break;
    default:
        break;
    }
    switch (type) {
    case kEdgeVertexShader:
        file += "edge.vsh";
        break;
    case kEdgeFragmentShader:
        file += "edge.fsh";
        break;
    case kModelVertexShader:
        file += "model.vsh";
        break;
    case kModelFragmentShader:
        file += "model.fsh";
        break;
    case kShadowVertexShader:
        file += "shadow.vsh";
        break;
    case kShadowFragmentShader:
        file += "shadow.fsh";
        break;
    case kZPlotVertexShader:
        file += "zplot.vsh";
        break;
    case kZPlotFragmentShader:
        file += "zplot.fsh";
        break;
    case kEdgeWithSkinningVertexShader:
        file += "skinning/edge.vsh";
        break;
    case kModelWithSkinningVertexShader:
        file += "skinning/model.vsh";
        break;
    case kShadowWithSkinningVertexShader:
        file += "skinning/shadow.vsh";
        break;
    case kZPlotWithSkinningVertexShader:
        file += "skinning/zplot.vsh";
        break;
    case kModelEffectTechniques:
    case kMaxShaderType:
    default:
        break;
    }
    const UnicodeString &path = shaderDirectory() + UnicodeString::fromUTF8(file.c_str());
    MapBuffer buffer(this);
    if (mapFile(path, &buffer)) {
        std::string bytes(buffer.address, buffer.address + buffer.size);
        return new(std::nothrow) String(UnicodeString::fromUTF8("#version 120\n" + bytes));
    }
    else {
        return 0;
    }
}

IString *BaseRenderContext::loadShaderSource(ShaderType type, const IString *path)
{
#ifdef VPVL2_ENABLE_NVIDIA_CG
    if (type == kModelEffectTechniques) {
        std::string bytes;
        MapBuffer buffer(this);
        if (path && mapFile(static_cast<const String *>(path)->value(), &buffer)) {
            uint8_t *address = buffer.address;
            bytes.assign(address, address + buffer.size);
        }
        else {
            UnicodeString defaultEffectPath = effectDirectory();
            defaultEffectPath.append("/base.cgfx");
            if (mapFile(defaultEffectPath, &buffer)) {
                uint8_t *address = buffer.address;
                bytes.assign(address, address + buffer.size);
            }
        }
        return bytes.empty() ? 0 : new (std::nothrow) String(UnicodeString::fromUTF8(bytes));
    }
#else
    (void) type;
    (void) path;
#endif /* VPVL2_ENABLE_NVIDIA_CG */
    return 0;
}

IString *BaseRenderContext::loadKernelSource(KernelType type, void * /* context */)
{
    std::string file;
    switch (type) {
    case kModelSkinningKernel:
        file += "skinning.cl";
        break;
    default:
        break;
    }
    UnicodeString path = kernelDirectory();
    path.append("/");
    path.append(UnicodeString::fromUTF8(file));
    MapBuffer buffer(this);
    if (mapFile(path, &buffer)) {
        std::string bytes(buffer.address, buffer.address + buffer.size);
        return new(std::nothrow) String(UnicodeString::fromUTF8(bytes));
    }
    else {
        return 0;
    }
}

IString *BaseRenderContext::toUnicode(const uint8_t *str) const
{
    if (const char *s = reinterpret_cast<const char *>(str)) {
        return m_encodingRef->toString(str, strlen(s), IString::kShiftJIS);
    }
    return 0;
}

bool BaseRenderContext::hasExtension(const void *namePtr) const
{
    return m_extensions.find(static_cast<const char *>(namePtr)) != m_extensions.end();
}

void BaseRenderContext::startProfileSession(ProfileType type, const void * /* arg */)
{
#ifdef VPVL2_LINK_NVTT
    nv::Timer *timer = getProfileTimer(type);
    timer->start();
#else
    (void) type;
#endif /* VPVL2_LINK_NVTT */
}

void BaseRenderContext::stopProfileSession(ProfileType type, const void * /* arg */)
{
#ifdef VPVL2_LINK_NVTT
    nv::Timer *timer = getProfileTimer(type);
    timer->stop();
#else
    (void) type;
#endif /* VPVL2_LINK_NVTT */
}

#ifdef VPVL2_ENABLE_NVIDIA_CG

void BaseRenderContext::getViewport(Vector3 &value) const
{
    value.setValue(m_viewport.x, m_viewport.y, 1);
}

void BaseRenderContext::getMousePosition(Vector4 &value, MousePositionType type) const {
    switch (type) {
    case kMouseLeftPressPosition:
        value.setValue(m_mouseLeftPressPosition.x,
                       m_mouseLeftPressPosition.y,
                       m_mouseLeftPressPosition.z,
                       m_mouseLeftPressPosition.w);
        break;
    case kMouseMiddlePressPosition:
        value.setValue(m_mouseMiddlePressPosition.x,
                       m_mouseMiddlePressPosition.y,
                       m_mouseMiddlePressPosition.z,
                       m_mouseMiddlePressPosition.w);
        break;
    case kMouseRightPressPosition:
        value.setValue(m_mouseRightPressPosition.x,
                       m_mouseRightPressPosition.y,
                       m_mouseRightPressPosition.z,
                       m_mouseRightPressPosition.w);
        break;
    case kMouseCursorPosition:
        value.setValue(m_mouseCursorPosition.x,
                       m_mouseCursorPosition.y,
                       m_mouseCursorPosition.z,
                       m_mouseCursorPosition.w);
        break;
    default:
        break;
    }
}

IModel *BaseRenderContext::findModel(const IString *name) const
{
    IModel *model = m_sceneRef->findModel(name);
    if (!model) {
        if (IModel *const *value = m_basename2modelRefs.find(name->toHashString())) {
            model = *value;
        }
    }
    return model;
}

IModel *BaseRenderContext::effectOwner(const IEffect *effect) const
{
    if (IModel *const *value = m_effectRef2modelRefs.find(effect)) {
        return *value;
    }
    return 0;
}

void BaseRenderContext::setEffectOwner(const IEffect *effectRef, IModel *model)
{
    const IString *name = model->name();
    m_effectRef2owners.insert(effectRef, static_cast<const String *>(name)->value());
    m_effectRef2modelRefs.insert(effectRef, model);
}

void BaseRenderContext::addModelPath(IModel *model, const UnicodeString &path)
{
    if (model) {
        UErrorCode status = U_ZERO_ERROR;
        RegexMatcher filenameMatcher(".+/((.+)\\.\\w+)$", 0, status);
        filenameMatcher.reset(path);
        if (filenameMatcher.find()) {
            const UnicodeString &basename = filenameMatcher.group(1, status);
            if (!model->name()) {
                String s(filenameMatcher.group(2, status));
                model->setName(&s);
            }
            m_basename2modelRefs.insert(String::toStdString(basename).c_str(), model);
            m_modelRef2Basenames.insert(model, basename);
        }
        else {
            if (!model->name()) {
                String s(path);
                model->setName(&s);
            }
            m_basename2modelRefs.insert(String::toStdString(path).c_str(), model);
        }
        m_modelRef2Paths.insert(model, path);
    }
}

UnicodeString BaseRenderContext::effectOwnerName(const IEffect *effect) const
{
    if (const UnicodeString *value = m_effectRef2owners.find(effect)) {
        return *value;
    }
    return UnicodeString();
}

FrameBufferObject *BaseRenderContext::createFrameBufferObject()
{
    return new FrameBufferObject(m_renderColorFormat, m_msaaSamples);
}

void BaseRenderContext::getEffectCompilerArguments(Array<IString *> &arguments) const
{
    arguments.clear();
}

const IString *BaseRenderContext::effectFilePath(const IModel *model, const IString *dir) const
{
    const UnicodeString &path = findModelPath(model);
    if (!path.isEmpty()) {
        UErrorCode status = U_ZERO_ERROR;
        RegexMatcher filenameMatcher("^.+/(.+)\\.\\w+$", 0, status);
        filenameMatcher.reset(path);
        const UnicodeString &s = filenameMatcher.find() ? filenameMatcher.group(1, status) : path;
        RegexMatcher extractMatcher("^.+\\[(.+)(?:\\.(?:cg)?fx)?\\]$", 0, status);
        extractMatcher.reset(s);
        const UnicodeString &cgfx = extractMatcher.find()
                ? extractMatcher.replaceAll("$1.cgfx", status) : s + ".cgfx";
        const UnicodeString &newEffectPath = createPath(dir, cgfx);
        m_effectPathPtr.reset(existsFile(newEffectPath) ? new String(newEffectPath) : 0);
    }
    if (!m_effectPathPtr.get()) {
        m_effectPathPtr.reset(new String(createPath(dir, UnicodeString::fromUTF8("default.cgfx"))));
    }
    return m_effectPathPtr.get();
}

void BaseRenderContext::addSharedTextureParameter(const char *name, const SharedTextureParameter &parameter)
{
    SharedTextureParameterKey key(parameter.parameterRef, name);
    m_sharedParameters.insert(std::make_pair(key, parameter));
}

bool BaseRenderContext::tryGetSharedTextureParameter(const char *name, SharedTextureParameter &parameter) const
{
    SharedTextureParameterKey key(parameter.parameterRef, name);
    SharedTextureParameterMap::const_iterator it = m_sharedParameters.find(key);
    if (it != m_sharedParameters.end()) {
        parameter = it->second;
        return true;
    }
    return false;
}

void BaseRenderContext::setMousePosition(const glm::vec2 &value, bool pressed, MousePositionType type)
{
    switch (type) {
    case kMouseLeftPressPosition:
        m_mouseLeftPressPosition = glm::vec4(value.x, value.y, pressed, 0);
        break;
    case kMouseMiddlePressPosition:
        m_mouseMiddlePressPosition = glm::vec4(value.x, value.y, pressed, 0);
        break;
    case kMouseRightPressPosition:
        m_mouseRightPressPosition = glm::vec4(value.x, value.y, pressed, 0);
        break;
    case kMouseCursorPosition:
        m_mouseCursorPosition = glm::vec4(value.x, value.y, 0, 0);
        break;
    default:
        break;
    }
}

UnicodeString BaseRenderContext::findModelPath(const IModel *model) const
{
    if (const UnicodeString *value = m_modelRef2Paths.find(model)) {
        return *value;
    }
    return UnicodeString();
}

UnicodeString BaseRenderContext::findModelBasename(const IModel *model) const
{
    if (const UnicodeString *value = m_modelRef2Basenames.find(model)) {
        return *value;
    }
    return UnicodeString();
}

FrameBufferObject *BaseRenderContext::findFrameBufferObjectByRenderTarget(const IEffect::OffscreenRenderTarget &rt, bool enableAA)
{
    FrameBufferObject *buffer = 0;
    if (const ITexture *textureRef = rt.textureRef) {
        if (FrameBufferObject *const *value = m_renderTargets.find(textureRef)) {
            buffer = *value;
        }
        else {
            int nsamples = enableAA ? m_msaaSamples : 0;
            buffer = m_renderTargets.insert(textureRef, new FrameBufferObject(m_renderColorFormat, nsamples));
        }
    }
    return buffer;
}

void BaseRenderContext::bindOffscreenRenderTarget(OffscreenTexture *texture, bool enableAA)
{
    const IEffect::OffscreenRenderTarget &rt = texture->renderTarget;
    if (FrameBufferObject *buffer = findFrameBufferObjectByRenderTarget(rt, enableAA)) {
        buffer->bindTexture(texture->colorTextureRef, 0);
        buffer->bindDepthStencilBuffer(&texture->depthStencilBuffer);
    }
    static const GLuint buffers[] = { GL_COLOR_ATTACHMENT0 };
    static const int nbuffers = sizeof(buffers) / sizeof(buffers[0]);
    cg::Util::setRenderColorTargets(buffers, nbuffers);
}

void BaseRenderContext::releaseOffscreenRenderTarget(const OffscreenTexture *texture, bool enableAA)
{
    const IEffect::OffscreenRenderTarget &rt = texture->renderTarget;
    if (FrameBufferObject *buffer = findFrameBufferObjectByRenderTarget(rt, enableAA)) {
        buffer->readMSAABuffer(0);
        buffer->unbind();
    }
}

void BaseRenderContext::parseOffscreenSemantic(IEffect *effect, const IString *dir)
{
    if (effect) {
        EffectAttachmentRuleList attachmentRules;
        std::string line;
        UErrorCode status = U_ZERO_ERROR;
        Vector3 size;
        RegexMatcher extensionMatcher("\\.(cg)?fx(sub)?$", 0, status),
                pairMatcher("\\s*=\\s*", 0, status);
        Array<IEffect::OffscreenRenderTarget> offscreenRenderTargets;
        effect->getOffscreenRenderTargets(offscreenRenderTargets);
        const int nOffscreenRenderTargets = offscreenRenderTargets.count();
        /* オフスクリーンレンダーターゲットの設定 */
        for (int i = 0; i < nOffscreenRenderTargets; i++) {
            const IEffect::OffscreenRenderTarget &renderTarget = offscreenRenderTargets[i];
            const IEffect::IParameter *parameter = renderTarget.textureParameterRef;
            const IEffect::IAnnotation *annotation = parameter->annotationRef("DefaultEffect");
            std::istringstream stream(annotation ? annotation->stringValue() : std::string());
            std::vector<UnicodeString> tokens(2);
            attachmentRules.clear();
            /* スクリプトを解析 */
            while (std::getline(stream, line, ';')) {
                int32_t size = static_cast<int32_t>(tokens.size());
                if (pairMatcher.split(UnicodeString::fromUTF8(line), &tokens[0], size, status) == size) {
                    const UnicodeString &key = tokens[0].trim();
                    const UnicodeString &value = tokens[1].trim();
                    RegexMatcherSmartPtr regexp;
                    status = U_ZERO_ERROR;
                    /* self が指定されている場合は自身のエフェクトのファイル名を設定する */
                    if (key == "self") {
                        const IModel *model = effectOwner(effect);
                        const UnicodeString &name = findModelBasename(model);
                        regexp.reset(new RegexMatcher("\\A\\Q" + name + "\\E\\z", 0, status));
                    }
                    else {
                        UnicodeString pattern = "\\A\\Q" + key + "\\E\\z";
                        pattern.findAndReplace("?", "\\E.\\Q");
                        pattern.findAndReplace("*", "\\E.*\\Q");
                        pattern.findAndReplace("\\Q\\E", "");
                        regexp.reset(new RegexMatcher(pattern, 0, status));
                    }
                    IEffect *offscreenEffectRef = 0;
                    /* hide/none でなければオフスクリーン専用のモデルのエフェクト（オフスクリーン側が指定）を読み込む */
                    bool hidden = (value == "hide" || value == "none");
                    if (!hidden) {
                        const UnicodeString &path = createPath(dir, value);
                        extensionMatcher.reset(path);
                        status = U_ZERO_ERROR;
                        const String s2(extensionMatcher.replaceAll(".cgfx", status));
                        offscreenEffectRef = createEffectRef(&s2);
                        if (offscreenEffectRef) {
                            offscreenEffectRef->setParentEffectRef(effect);
                            VPVL2_VLOG(2, "Loaded an individual effect by offscreen: path=" << internal::cstr(&s2, "") << " pattern=" << String::toStdString(key));
                        }
                    }
                    attachmentRules.push_back(EffectAttachmentRule(regexp.release(), std::make_pair(offscreenEffectRef, hidden)));
                }
            }
            if (!cg::Util::getSize2(parameter, size)) {
                Vector3 viewport;
                getViewport(viewport);
                size.setX(btMax(1.0f, viewport.x() * size.x()));
                size.setY(btMax(1.0f, viewport.y() * size.y()));
            }
            BaseSurface::Format format; /* unused */
            cg::Util::getTextureFormat(parameter, format);
            /* RenderContext 特有の OffscreenTexture に変換して格納 */
            m_offscreenTextures.append(new OffscreenTexture(renderTarget, attachmentRules, size));
        }
    }
}

void BaseRenderContext::renderOffscreen()
{
    Array<IRenderEngine *> engines;
    m_sceneRef->getRenderEngineRefs(engines);
    const int nengines = engines.count();
    /* オフスクリーンレンダリングを行う前に元のエフェクトを保存する */
    Hash<HashPtr, IEffect *> effects;
    for (int i = 0; i < nengines; i++) {
        IRenderEngine *engine = engines[i];
        if (IEffect *starndardEffect = engine->effectRef(IEffect::kStandard)) {
            effects.insert(engine, starndardEffect);
        }
        else if (IEffect *postEffect = engine->effectRef(IEffect::kPostProcess)) {
            effects.insert(engine, postEffect);
        }
        else if (IEffect *preEffect = engine->effectRef(IEffect::kPreProcess)) {
            effects.insert(engine, preEffect);
        }
        else {
            effects.insert(engine, 0);
        }
    }
    /* オフスクリーンレンダーターゲット毎にエフェクトを実行する */
    const int ntextures = m_offscreenTextures.count();
    for (int i = 0; i < ntextures; i++) {
        OffscreenTexture *offscreenTexture = m_offscreenTextures[i];
        const EffectAttachmentRuleList &rules = offscreenTexture->attachmentRules;
        const IEffect::OffscreenRenderTarget &renderTarget = offscreenTexture->renderTarget;
        const IEffect::IParameter *parameter = renderTarget.textureParameterRef;
        bool enableAA = false;
        /* セマンティクスから各種パラメータを設定 */
        if (const IEffect::IAnnotation *annotation = parameter->annotationRef("AntiAlias")) {
            enableAA = annotation->booleanValue();
        }
        /* オフスクリーンレンダリングターゲットを割り当ててレンダリング先をそちらに変更する */
        bindOffscreenRenderTarget(offscreenTexture, enableAA);
        const ITexture *texture = renderTarget.textureRef;
        const Vector3 &size = texture->size();
        updateCameraMatrices(glm::vec2(size.x(), size.y()));
        glViewport(0, 0, GLsizei(size.x()), GLsizei(size.y()));
        if (const IEffect::IAnnotation *annotation = parameter->annotationRef("ClearColor")) {
            int nvalues;
            const float *color = annotation->floatValues(&nvalues);
            if (nvalues == 4) {
                glClearColor(color[0], color[1], color[2], color[3]);
            }
        }
        else {
            glClearDepth(1);
        }
        /* オフスクリーンレンダリングターゲットに向けてレンダリングを実行する */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        for (int j = 0; j < nengines; j++) {
            IRenderEngine *engine = engines[j];
            const IModel *model = engine->parentModelRef();
            const UnicodeString &basename = findModelBasename(model);
            EffectAttachmentRuleList::const_iterator it2 = rules.begin();
            bool hidden = false;
            while (it2 != rules.end()) {
                const EffectAttachmentRule &rule = *it2;
                RegexMatcher *matcherRef = rule.first;
                matcherRef->reset(basename);
                if (matcherRef->find()) {
                    const EffectAttachmentValue &v = rule.second;
                    IEffect *effectRef = v.first;
                    engine->setEffect(IEffect::kStandardOffscreen, effectRef, 0);
                    hidden = v.second;
                    break;
                }
                ++it2;
            }
            if (!hidden) {
                engine->update();
                engine->renderModel();
                engine->renderEdge();
            }
        }
        /* オフスクリーンレンダリングターゲットの割り当てを解除 */
        releaseOffscreenRenderTarget(offscreenTexture, enableAA);
    }
    for (int i = 0; i < nengines; i++) {
        IRenderEngine *engine = engines[i];
        IEffect *const *effect = effects.find(engine);
        engine->setEffect(IEffect::kAutoDetection, *effect, 0);
    }
}

IEffect *BaseRenderContext::createEffectRef(const IString *path)
{
    IEffect *effectRef = 0;
    const HashString key(path->toHashString());
    if (IEffect *const *value = m_effectCaches.find(key)) {
        effectRef = *value;
    }
    else if (existsFile(static_cast<const String *>(path)->value())) {
        IEffectSmartPtr effectPtr(m_sceneRef->createEffectFromFile(path, this));
        if (!effectPtr.get() || !effectPtr->internalPointer()) {
            VPVL2_LOG(WARNING, "Cannot compile an effect: " << internal::cstr(path, "(null)") << " error=" << cgGetLastListing(static_cast<CGcontext>(effectPtr->internalContext())));
        }
        else if (!m_effectCaches.find(key)) {
            effectRef = m_effectCaches.insert(key, effectPtr.release());
        }
        else {
            VPVL2_LOG(INFO, "Duplicated effect was found and ignored it: " << internal::cstr(path, "(null)"));
        }
    }
    else {
        effectRef = m_effectCaches.insert(key, m_sceneRef->createDefaultStandardEffect(this));
        if (!effectRef || !effectRef->internalPointer()) {
            VPVL2_LOG(WARNING, "Cannot compile an effect: " << internal::cstr(path, "(null)") << " error=" << cgGetLastListing(static_cast<CGcontext>(effectRef->internalContext())));
        }
    }
    return effectRef;
}

IEffect *BaseRenderContext::createEffectRef(IModel *model, const IString *dir)
{
    const UnicodeString &pathForKey = static_cast<const String *>(effectFilePath(model, dir))->value();
    const String s(pathForKey);
    IEffect *effectRef = createEffectRef(&s);
    if (effectRef) {
        setEffectOwner(effectRef, model);
        const IString *name = model->name(); (void) name;
        VPVL2_LOG(INFO, "Loaded an model effect: model=" << internal::cstr(name, "(null)") << " path=" << internal::cstr(&s, ""));
    }
    return effectRef;
}

#endif /* VPVL2_ENABLE_NVIDIA_CG */

void BaseRenderContext::setArchive(Archive *value)
{
    m_archive = value;
}

void BaseRenderContext::setSceneRef(Scene *value)
{
    release();
    m_sceneRef = value;
}

void BaseRenderContext::getCameraMatrices(glm::mat4x4 &world, glm::mat4x4 &view, glm::mat4x4 &projection)
{
    world = m_cameraWorldMatrix;
    view = m_cameraViewMatrix;
    projection = m_cameraProjectionMatrix;
}

void BaseRenderContext::setCameraMatrices(const glm::mat4x4 &world, const glm::mat4x4 &view, const glm::mat4x4 &projection)
{
    m_cameraWorldMatrix = world;
    m_cameraViewMatrix = view;
    m_cameraProjectionMatrix = projection;
}

void BaseRenderContext::setViewport(const glm::vec2 &value)
{
    m_viewport = value;
}

void BaseRenderContext::updateCameraMatrices(const glm::vec2 &size)
{
    const ICamera *camera = m_sceneRef->camera();
    Scalar matrix[16];
    camera->modelViewTransform().getOpenGLMatrix(matrix);
    const glm::mediump_float &aspect = glm::max(size.x, size.y) / glm::min(size.x, size.y);
    const glm::mat4x4 world, &view = glm::make_mat4x4(matrix),
            &projection = glm::infinitePerspective(camera->fov(), aspect, camera->znear());
    setCameraMatrices(world, view, projection);
    setViewport(size);
}

void BaseRenderContext::createShadowMap(const Vector3 &size)
{
    if (Scene::isSelfShadowSupported()) {
        m_shadowMap.reset(new SimpleShadowMap(size_t(size.x()), size_t(size.y())));
        m_shadowMap->create();
        m_sceneRef->setShadowMapRef(m_shadowMap.get());
    }
}

void BaseRenderContext::renderShadowMap()
{
    if (m_shadowMap.get()) {
        m_shadowMap->bind();
        const Vector3 &size = m_shadowMap->size();
        glViewport(0, 0, GLsizei(size.x()), GLsizei(size.y()));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Array<IRenderEngine *> engines;
        m_sceneRef->getRenderEngineRefs(engines);
        const int nengines = engines.count();
        for (int i = 0; i < nengines; i++) {
            IRenderEngine *engine = engines[i];
            engine->renderZPlot();
        }
        m_shadowMap->unbind();
    }
}

const UnicodeString BaseRenderContext::createPath(const IString *dir, const UnicodeString &name)
{
    UnicodeString n = name;
    return static_cast<const String *>(dir)->value() + "/" + n.findAndReplace('\\', '/');
}

const UnicodeString BaseRenderContext::createPath(const IString *dir, const IString *name)
{
    const UnicodeString &d = static_cast<const String *>(dir)->value();
    UnicodeString n = static_cast<const String *>(name)->value();
    return d + "/" + n.findAndReplace('\\', '/');
}

UnicodeString BaseRenderContext::toonDirectory() const
{
    return m_configRef->value("dir.system.toon", UnicodeString(":textures"));
}

UnicodeString BaseRenderContext::shaderDirectory() const
{
    return m_configRef->value("dir.system.shaders", UnicodeString(":shaders"));
}

UnicodeString BaseRenderContext::effectDirectory() const
{
    return m_configRef->value("dir.system.effects", UnicodeString(":effects"));
}

UnicodeString BaseRenderContext::kernelDirectory() const
{
    return m_configRef->value("dir.system.kernels", UnicodeString(":kernels"));
}

void BaseRenderContext::debugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                             GLsizei /* length */, const GLchar *message, GLvoid * /* userParam */)
{
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        VPVL2_LOG(ERROR, "ID=" << id << " Type=" << DebugMessageTypeToString(type) << " Source=" << DebugMessageSourceToString(source) << ": " << message);
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        VPVL2_LOG(WARNING, "ID=" << id << " Type=" << DebugMessageTypeToString(type) << " Source=" << DebugMessageSourceToString(source) << ": " << message);
        break;
    case GL_DEBUG_SEVERITY_LOW:
        VPVL2_LOG(INFO, "ID=" << id << " Type=" << DebugMessageTypeToString(type) << " Source=" << DebugMessageSourceToString(source) << ": " << message);
        break;
    default:
        break;
    }
}

void BaseRenderContext::release()
{
    if (GLEW_ARB_sampler_objects) {
        glDeleteSamplers(1, &m_textureSampler);
        glDeleteSamplers(1, &m_toonTextureSampler);
    }
    m_sceneRef = 0;
#ifdef VPVL2_ENABLE_EXTENSION_ARCHIVE
    delete m_archive;
#endif
    m_archive = 0;
#ifdef VPVL2_ENABLE_NVIDIA_CG
    m_offscreenTextures.releaseAll();
    m_renderTargets.releaseAll();
    m_basename2modelRefs.clear();
    m_modelRef2Paths.clear();
    m_effectRef2modelRefs.clear();
    m_effectRef2owners.clear();
    m_sharedParameters.clear();
    m_effectPathPtr.reset();
    m_effectCaches.releaseAll();
#endif
#ifdef VPVL2_LINK_NVTT
    m_profileTimers.releaseAll();
#endif
}

#ifdef VPVL2_LINK_NVTT

nv::Timer *BaseRenderContext::getProfileTimer(ProfileType type) const
{
    nv::Timer *timer = 0;
    if (nv::Timer *const *value = m_profileTimers.find(type)) {
        timer = *value;
    }
    else {
        timer = m_profileTimers.insert(type, new nv::Timer());
    }
    return timer;
}

#endif /* VPVL2_LINK_NVTT */


} /* namespace extensions */
} /* namespace vpvl2 */

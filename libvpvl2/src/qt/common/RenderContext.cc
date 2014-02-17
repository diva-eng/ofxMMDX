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

/* include ICU first to resolve an issue of stdint.h on MSVC */
#include <unicode/unistr.h>
#include <vpvl2/qt/RenderContext.h>

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/Archive.h>
#include <vpvl2/qt/Util.h>

#include <QtCore>
#include <QColor>
#include <QImage>
#include <QMovie>

#ifndef VPVL2_LINK_GLEW
#include <QGLContext>
#endif

#ifdef VPVL2_LINK_NVTT
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif
#include <nvcore/Debug.h>
#include <nvcore/Stream.h>
#include <nvimage/DirectDrawSurface.h>
#include <nvimage/Image.h>
#include <nvimage/ImageIO.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
namespace {
struct MessageHandler : public nv::MessageHandler, public nv::AssertHandler {
    int assertion(const char *exp, const char *file, int line, const char *func) {
        qFatal("Assertion error: %s (%s in %s at %d)", exp, func, file, line);
        return 0;
    }
    void log(const char *format, va_list arg) {
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
        fprintf(stderr, format, arg);
#ifdef __clang__
#pragma clang diagnostic pop
#endif
    }
};
MessageHandler s_messageHandler;
}
#else
namespace nv {
class Stream {
public:
    Stream() {}
    virtual ~Stream() {}
};
}
#endif

using namespace vpvl2;
using namespace vpvl2::extensions;
using namespace vpvl2::qt;

namespace
{

#ifdef VPVL2_LINK_NVTT

class ReadonlyFileStream : public nv::Stream {
public:
    ReadonlyFileStream(const QString &path) : m_file(path) { m_file.open(QFile::ReadOnly); }
    ~ReadonlyFileStream() {}

    bool isSaving() const { return false; }
    bool isError() const { return m_file.error() != QFile::NoError; }
    void seek(uint pos) { m_file.seek(pos); }
    uint tell() const { return m_file.pos(); }
    uint size() const { return m_file.size(); }
    void clearError() {}
    bool isAtEnd() const { return m_file.atEnd(); }
    bool isSeekable() const { return m_file.isSequential(); }
    bool isLoading() const { return true; }
    uint serialize(void *data, uint len) { return m_file.read(static_cast<char *>(data), len); }

private:
    QFile m_file;
};

class ReadonlyMemoryStream : public nv::Stream {
public:
    ReadonlyMemoryStream(QByteArray &bytes) : m_buffer(&bytes) { m_buffer.open(QBuffer::ReadOnly); }
    ~ReadonlyMemoryStream() {}

    bool isSaving() const { return false; }
    bool isError() const { return false; }
    void seek(uint pos) { m_buffer.seek(pos); }
    uint tell() const { return m_buffer.pos(); }
    uint size() const { return m_buffer.size(); }
    void clearError() {}
    bool isAtEnd() const { return m_buffer.atEnd(); }
    bool isSeekable() const { return m_buffer.isSequential(); }
    bool isLoading() const { return true; }
    uint serialize(void *data, uint len) { return m_buffer.read(static_cast<char *>(data), len); }

private:
    QBuffer m_buffer;
};

#else /* VPVL2_LINK_NVTT */

class ReadonlyFileStream : public nv::Stream {
public:
    ReadonlyFileStream(const QString &/*path*/) {}
    ~ReadonlyFileStream() {}
};

class ReadonlyMemoryStream : public nv::Stream {
public:
    ReadonlyMemoryStream(QByteArray &/*bytes*/) {}
    ~ReadonlyMemoryStream() {}
};

#endif /* VPVL2_LINK_NVTT */

#ifdef VPVL2_LINK_NVTT
static const QImage UIConvertNVImageToQImage(const nv::Image &image)
{
    const uint8_t *pixels = reinterpret_cast<const uchar *>(image.pixels());
    QImage::Format format = image.format() == nv::Image::Format_ARGB
            ? QImage::Format_ARGB32 : QImage::Format_RGB32;
    return QImage(pixels, image.width(), image.height(), format);
}
#endif

}

namespace vpvl2
{
namespace qt
{

using namespace extensions::gl;

QSet<QString> RenderContext::loadableTextureExtensions()
{
    /* QImage に読み込ませる画像の拡張子を返す */
    static QSet<QString> extensions;
    if (extensions.isEmpty()) {
        extensions << "jpg";
        extensions << "png";
        extensions << "bmp";
        extensions << "sph";
        extensions << "spa";
    }
    return extensions;
}

RenderContext::RenderContext(Scene *sceneRef, IEncoding *encodingRef, const StringMap *settingsRef)
    : BaseRenderContext(sceneRef, encodingRef, settingsRef)
{
    m_timer.start();
#ifdef VPVL2_LINK_NVTT
    nv::debug::setAssertHandler(&s_messageHandler);
    nv::debug::setMessageHandler(&s_messageHandler);
#endif
}

RenderContext::~RenderContext()
{
}

#ifdef VPVL2_ENABLE_NVIDIA_CG
void RenderContext::getToonColor(const IString *name, const IString *dir, Color &value, void * /* context */)
{
    const QString &path = createQPath(dir, name);
    bool ok = false;
    /* ファイルが存在する、またはアーカイブ内にあると予想される場合はそちらを読み込む */
    if (m_archive || QFile::exists(path)) {
        getToonColorInternal(path, false, value, ok);
    }
    /* 上でなければシステム側のトゥーンテクスチャを読み込む */
    if (!ok) {
        String s(toonDirectory());
        const QString &fallback = createQPath(&s, name);
        getToonColorInternal(fallback, true, value, ok);
    }
}

void RenderContext::uploadAnimatedTexture(float offset, float speed, float seek, void *texture)
{
    ITexture *textureRef = static_cast<ITexture *>(texture);
    QMovie *movie = 0;
    /* キャッシュを読み込む */
    if (m_texture2Movies.contains(textureRef)) {
        movie = m_texture2Movies[textureRef].data();
    }
    else {
        /* アニメーションテクスチャを読み込み、キャッシュに格納する */
        const QString &path = m_texture2Paths[textureRef];
        m_texture2Movies.insert(textureRef, QSharedPointer<QMovie>(new QMovie(path)));
        movie = m_texture2Movies[textureRef].data();
        movie->setCacheMode(QMovie::CacheAll);
    }
    /* アニメーションテクスチャが読み込み可能な場合はパラメータを設定してテクスチャを取り出す */
    if (movie->isValid()) {
        offset *= Scene::defaultFPS();
        int frameCount = movie->frameCount();
        offset = qBound(0, int(offset), frameCount);
        int left = int(seek * speed * Scene::defaultFPS() + frameCount - offset);
        int right = qMax(int(frameCount - offset), 1);
        int frameIndex = left % right + int(offset);
        /* アニメーションテクスチャ内のフレーム移動を行い、該当の画像をテクスチャに変換する */
        if (movie->jumpToFrame(frameIndex)) {
            const QImage &image = movie->currentImage();
#ifdef VPVL2_LINK_GLEW
            textureRef->bind();
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width(), image.height(),
                            GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image.constBits());
            glBindTexture(GL_TEXTURE_2D, 0);
#else
            const QImage &textureImage = QGLWidget::convertToGLFormat(image.mirrored());
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width(), image.height(), GL_RGBA, GL_UNSIGNED_BYTE, textureImage.constBits());
            glBindTexture(GL_TEXTURE_2D, 0);
#endif
        }
    }
}

void RenderContext::getTime(float &value, bool sync) const
{
    value = sync ? 0 : m_timer.elapsed() / 1000.0f;
}

void RenderContext::getElapsed(float &value, bool sync) const
{
    value = sync ? 0 : 1.0 / 60.0;
}
#endif

void *RenderContext::findProcedureAddress(const void **candidatesPtr) const
{
#ifndef VPVL2_LINK_GLEW
    const QGLContext *context = QGLContext::currentContext();
    const char **candidates = reinterpret_cast<const char **>(candidatesPtr);
    const char *candidate = candidates[0];
    int i = 0;
    while (candidate) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        void *address = reinterpret_cast<void *>(context->getProcAddress(candidate));
#elif defined(WIN32)
        void *address = wglGetProcAddress(candidate);
#else
        void *address = context->getProcAddress(candidate);
#endif
        if (address) {
            return address;
        }
        candidate = candidates[++i];
    }
#else
    Q_UNUSED(candidatesPtr)
#endif
    return 0;
}

//#define VPVL2_USE_MMAP

bool RenderContext::mapFile(const UnicodeString &path, MapBuffer *buffer) const
{
    QScopedPointer<QFile> file(new QFile(Util::toQString(path)));
    if (file->open(QFile::ReadOnly | QFile::Unbuffered)) {
        bool ok = true;
        size_t size = 0;
#ifdef VPVL2_USE_MMAP
        size = file->size();
        buffer->address = file->map(0, size);
        ok = buffer->address != 0;
#else
        const QByteArray &bytes = file->readAll();
        size = bytes.size();
        buffer->address = new uint8_t[size];
        memcpy(buffer->address, bytes.constData(), size);
#endif
        buffer->size = size;
        buffer->opaque = file.take();
        return ok;
    }
    VPVL2_LOG(WARNING, "Cannot load " << qPrintable(file->fileName()) << ": " << qPrintable(file->errorString()));
    return false;
}

bool RenderContext::unmapFile(MapBuffer *buffer) const
{
    if (QFile *file = static_cast<QFile *>(buffer->opaque)) {
#ifdef VPVL2_USE_MMAP
        file->unmap(buffer->address);
#else
        delete[] buffer->address;
#endif
        file->close();
        delete file;
        return true;
    }
    return false;
}

#undef VPVL2_USE_MMAP

bool RenderContext::existsFile(const UnicodeString &path) const
{
    return QFile::exists(Util::toQString(path));
}

void RenderContext::removeModel(IModel * /* model */)
{
#if 0
    /* ファイル名からモデルインスタンスのハッシュの全ての参照を削除 */
    QMutableHashIterator<const QString, IModel *> it(m_basename2ModelRefs);
    while (it.hasNext()) {
        it.next();
        IModel *m = it.value();
        if (m == model) {
            it.remove();
        }
    }
    /* エフェクトインスタンスからモデルインスタンスのハッシュの全ての参照を削除 */
    QMutexLocker locker(&m_effect2modelsLock); Q_UNUSED(locker);
    QMutableHashIterator<const IEffect *, IModel *> it2(m_effectRef2modelRefs);
    while (it2.hasNext()) {
        it2.next();
        IModel *m = it2.value();
        if (m == model) {
            it2.remove();
        }
    }
#endif
}

bool RenderContext::uploadTextureNVTT(const QString &suffix,
                                      const QString &path,
                                      QScopedPointer<nv::Stream> &stream,
                                      Texture &texture,
                                      ModelContext *modelContext)
{
#ifdef VPVL2_LINK_NVTT
    if (suffix == "dds") {
        nv::DirectDrawSurface surface;
        if (surface.load(stream.take())) {
            nv::Image nvimage;
            surface.mipmap(&nvimage, 0, 0);
            QImage image(UIConvertNVImageToQImage(nvimage));
            return generateTextureFromImage(image, path, texture, modelContext);
        }
        else {
            warning(modelContext, "%s cannot be loaded", qPrintable(path));
        }
    }
    else {
        QScopedPointer<nv::Image> nvimage(nv::ImageIO::load(path.toUtf8().constData(), *stream));
        if (nvimage) {
            QImage image(UIConvertNVImageToQImage(*nvimage));
            return generateTextureFromImage(image, path, texture, modelContext);
        }
        else {
            warning(modelContext, "%s cannot be loaded", qPrintable(path));
        }
    }
#else
    Q_UNUSED(suffix)
    Q_UNUSED(path)
    Q_UNUSED(stream)
    Q_UNUSED(texture)
    Q_UNUSED(modelContext)
#endif
    return true;
}

QString RenderContext::createQPath(const IString *dir, const IString *name)
{
    const UnicodeString &d = static_cast<const String *>(dir)->value();
    const UnicodeString &n = static_cast<const String *>(name)->value();
    const QString &d2 = Util::toQString(d);
    const QString &n2 = Util::toQString(n);
    return QDir(d2).absoluteFilePath(n2);
}

bool RenderContext::uploadTextureInternal(const UnicodeString &path, Texture &texture, void *context)
{
    const QString &newPath = Util::toQString(path);
    const QFileInfo info(newPath);
    ModelContext *modelContext = static_cast<ModelContext *>(context);
#ifdef VPVL2_ENABLE_EXTENSIONS_ARCHIVE
    /*
     * ZIP 圧縮からの読み込み (ただしシステムが提供する toon テクスチャは除く)
     * Archive が持つ仮想ファイルシステム上にあるため、キャッシュより後、物理ファイル上より先に検索しないといけない
     */
    if (m_archive && !texture.system) {
        if (const std::string *byteArray = m_archive->data(path)) {
            const uint8_t *ptr = reinterpret_cast<const uint8_t *>(byteArray->data());
            return modelContext->uploadTextureData(ptr, byteArray->size(), path, texture);
        }
        VPVL2_LOG(WARNING, "Cannot load a texture from archive: " << qPrintable(newPath));
        /* force true to continue loading textures if path is directory */
        bool ok = texture.ok = info.isDir();
        return ok;
    }
    /* ディレクトリの場合はスキップする。ただしトゥーンの場合は白テクスチャの読み込みを行う */
    else if (info.isDir()) {
#else
    if (info.isDir()) {
#endif
        if (texture.toon) { /* force loading as white toon texture */
            String d(toonDirectory());
            const UnicodeString &newToonPath = createPath(&d, UnicodeString::fromUTF8("toon0.bmp"));
            if (modelContext && !modelContext->findTextureCache(newToonPath, texture)) {
                /* fallback to default texture loader */
                return modelContext->uploadTextureFile(newToonPath, texture);
            }
        }
        return true; /* skip */
    }
    else if (newPath.startsWith(":textures/")) {
        QFile file(newPath);
        /* open a (system) toon texture from library resource */
        if (file.open(QFile::ReadOnly | QFile::Unbuffered)) {
            const QByteArray &bytes = file.readAll();
            const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
            return modelContext->uploadTextureData(data, bytes.size(), path, texture);
        }
        return false;
    }
    else if (!info.exists()) {
        VPVL2_LOG(WARNING, "Cannot load inexist " << qPrintable(newPath));
        return true; /* skip */
    }
    else if (info.suffix() == "jpg" || info.suffix() == "png" || info.suffix() == "bmp") {
        /* use Qt's pluggable image loader (jpg/png is loaded with libjpeg/libpng) */
        QImage image(newPath);
        BaseSurface::Format format(GL_BGRA, GL_RGBA8, GL_UNSIGNED_INT_8_8_8_8_REV, GL_TEXTURE_2D);
        const Vector3 size(image.width(), image.height(), 1);
        ITexture *texturePtr = modelContext->createTexture(image.constBits(), format, size, texture.mipmap, false);
        return modelContext->cacheTexture(texturePtr, texture, path);
    }
    /* fallback to default texture loader */
    return modelContext->uploadTextureFile(path, texture);
}

bool RenderContext::generateTextureFromImage(const QImage &image,
                                             const QString &path,
                                             Texture &texture,
                                             ModelContext *modelContext)
{
    if (!image.isNull()) {
        BaseSurface::Format format(GL_BGRA, GL_RGBA8, GL_UNSIGNED_INT_8_8_8_8_REV, GL_TEXTURE_2D);
        const Vector3 size(image.width(), image.height(), 1);
        ITexture *textureRef = modelContext->createTexture(image.constBits(), format, size, texture.mipmap, false);
        texture.texturePtrRef = textureRef;
        m_texture2Paths.insert(textureRef, path);
        if (modelContext) {
            modelContext->addTextureCache(Util::fromQString(path), textureRef);
        }
        VPVL2_VLOG(2, "Loaded a texture: ID=" << textureRef << " width=" << size.x() << " height=" << size.y() << " depth=" << size.z() << " path=" << qPrintable(path));
        bool ok = texture.ok = textureRef != 0;
        return ok;
    }
    else {
        VPVL2_LOG(WARNING, "Failed loading a image to convert the texture: " << qPrintable(path));
        return false;
    }
}

void RenderContext::getToonColorInternal(const QString &path, bool isSystem, Color &value, bool &ok)
{
    QImage image(path);
#ifdef VPVL2_ENABLE_EXTENSIONS_ARCHIVE
    if (!isSystem && m_archive) {
        QByteArray suffix = QFileInfo(path).suffix().toLower().toUtf8();
        if (suffix == "sph" || suffix == "spa")
            suffix.setRawData("bmp", 3);
        if (const std::string *bytes = m_archive->data(Util::fromQString(path))) {
            image.loadFromData(bytes->data(), suffix.constData());
        }
    }
#else
    (void) isSystem;
#endif
    if (!image.isNull()) {
        const QRgb &rgb = image.pixel(image.width() - 1, image.height() - 1);
        const QColor color(rgb);
        value.setValue(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        ok = true;
    }
    else {
        value.setValue(1, 1, 1, 1);
        ok = true;
    }
}

} /* namespace qt */
} /* namespace vpvl2 */

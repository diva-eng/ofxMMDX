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

#include <QtGlobal>
#include <QtCore>
#include <QApplication>

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/AudioSource.h>
#include <vpvl2/qt/Util.h>
#include "UI.h"

using namespace vpvl2;

int main(int argc, char *argv[])
{
#ifdef VPVL2_LINK_GLOG
#if !defined(_WIN32)
    google::InstallFailureSignalHandler();
#endif
    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = true;
    FLAGS_v = 2;
#endif
    int ret = 0;
    QApplication app(argc, argv);
    extensions::AudioSource::initialize();
    qt::Util::initializeResources();
#if 1
    QGLFormat format;
    format.setAlpha(true);
    format.setSampleBuffers(true);
    vpvl2::render::qt::UI ui(format);
    ui.load(QDir::current().absoluteFilePath("config.ini"));
    ui.show();
    ret = app.exec();
#else
    UI *ui = new UI();
    ui->load(QDir::current().absoluteFilePath("config.ini"));
    ui->show();
    delete ui;
    ret = 0;
#endif
    extensions::AudioSource::terminate();
    qt::Util::cleanupResources();
#ifdef VPVL2_LINK_GLOG
    google::ShutdownGoogleLogging();
#endif
    return ret;
}

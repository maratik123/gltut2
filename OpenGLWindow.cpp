//
// Created by maratik on 25.02.18.
//

#include "OpenGLWindow.h"
#include <QCoreApplication>
#include <QOpenGLDebugLogger>
#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QOpenGLFunctions>

OpenGLWindow::OpenGLWindow(bool enableLogger, QWindow *parent) :
        QWindow(parent),
        mUpdatePending(false),
        mAnimating(false),
        mEnableLogger(enableLogger),
        mContext(nullptr),
        mDevice(nullptr),
        mLogger(nullptr) {
    setSurfaceType(QWindow::OpenGLSurface);
}

OpenGLWindow::~OpenGLWindow() {
    delete mDevice;
}

void OpenGLWindow::setAnimation(bool animating) {
    mAnimating = animating;

    if (animating) {
        renderLater();
    }
}

void OpenGLWindow::renderLater() {
    if (Q_LIKELY(!mUpdatePending)) {
        mUpdatePending = true;
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}

void OpenGLWindow::renderNow() {
    if (Q_UNLIKELY(!isExposed())) {
        return;
    }

    if (Q_UNLIKELY(mContext == nullptr)) {
        mContext = new QOpenGLContext(this);
        mContext->setFormat(requestedFormat());
        mContext->create();
        mContext->makeCurrent(this);

        if (mEnableLogger) {
            mLogger = new QOpenGLDebugLogger(this);
            if (mLogger->initialize()) {
                connect(mLogger, &QOpenGLDebugLogger::messageLogged, this, &OpenGLWindow::messageLogged);
                mLogger->startLogging();
                for (const auto &message : mLogger->loggedMessages()) {
                    emit messageLogged(message);
                }
            }
        }

        initialize();
    } else {
        mContext->makeCurrent(this);
    }

    render();

    mContext->swapBuffers(this);

    if (Q_LIKELY(mAnimating)) {
        renderLater();
    }
}

bool OpenGLWindow::event(QEvent *event) {
    switch (event->type()) {
        case QEvent::UpdateRequest:
            mUpdatePending = false;
            renderNow();
            return true;
        case QEvent::Close:
            deinitializeNow();
            break;
        default:
            break;
    }
    return QWindow::event(event);
}

void OpenGLWindow::render() {
    if (Q_UNLIKELY(mDevice == nullptr)) {
        mDevice = new QOpenGLPaintDevice;
    }

    mContext->functions()->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    mDevice->setSize(size());

    render(QPainter(mDevice));
}

void OpenGLWindow::deinitializeNow() {
    if (mContext != nullptr) {
        mContext->makeCurrent(this);
        deinitialize();
    }
}

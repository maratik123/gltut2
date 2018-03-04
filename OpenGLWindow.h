//
// Created by maratik on 25.02.18.
//

#ifndef GLTUT2_OPENGLWINDOW_H
#define GLTUT2_OPENGLWINDOW_H

#include <QWindow>

class QOpenGLPaintDevice;
class QOpenGLDebugLogger;
class QOpenGLDebugMessage;
class QOpenGLFunctions;

class OpenGLWindow : public QWindow {
    Q_OBJECT

public:
    explicit OpenGLWindow(bool enableLogger = false, QWindow *parent = nullptr);
    ~OpenGLWindow() override;

    void setAnimation(bool animating);

public slots:
    void renderLater();
    void renderNow();

signals:
    void messageLogged(const QOpenGLDebugMessage &debugMessage);

protected:
    bool event(QEvent *event) override;

    void exposeEvent(QExposeEvent */*event*/) override { renderNow(); }

    virtual void render(const QPainter &/*painter*/) {}
    virtual void render();

    virtual void initialize() {}
    virtual void deinitialize() {}

private:
    void deinitializeNow();

    bool mUpdatePending;
    bool mAnimating;
    bool mEnableLogger;

    QOpenGLContext *mContext;
    QOpenGLPaintDevice *mDevice;
    QOpenGLDebugLogger *mLogger;
};

#endif //GLTUT2_OPENGLWINDOW_H

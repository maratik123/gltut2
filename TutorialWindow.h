//
// Created by maratik on 25.02.18.
//

#ifndef GLTUT2_TUTORIALWINDOW_H
#define GLTUT2_TUTORIALWINDOW_H

#include "OpenGLWindow.h"
#include <QOpenGLFunctions_4_5_Core>
#include <QMatrix4x4>

class QOpenGLShaderProgram;
class QOpenGLBuffer;
class QOpenGLVertexArrayObject;
class QOpenGLTexture;

class TutorialWindow : public OpenGLWindow, protected QOpenGLFunctions_4_5_Core {
    Q_OBJECT

public:
    enum Direction {
        Forward = 0x1,
        Backward = 0x2,
        Left = 0x4,
        Right = 0x8
    };
    Q_DECLARE_FLAGS(Directions, Direction)

    explicit TutorialWindow(bool enableLogger = false, QWindow *parent = nullptr);
    ~TutorialWindow() override;

protected:
    void initialize() override;
    void render() override;
    void deinitialize() override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void updateSize(const QSize &newSize);
    void updateMixBalance(float delta);
    void updateProjViewMat();
    void updateViewMat();
    bool keyEvent(QKeyEvent *event, bool isKeyPressed);
    void explicitUpdateViewMat();
    void updateCameraFront();

    QOpenGLShaderProgram *mProgram;
    QOpenGLBuffer *mVbo;
    QOpenGLBuffer *mLeftTriangleEbo;
    QOpenGLVertexArrayObject *mLeftTriangleVao;
    QSize mPrevSize;
    QOpenGLTexture *mContainerTexture;
    QOpenGLTexture *mAwesomeTexture;
    float mMixBalance;
    int mMixBalanceLocation;
    const qint64 mStartTime;
    int mTransformLocation;
    float mScreenRatio;
    QMatrix4x4 mViewMat;
    QMatrix4x4 mProjMat;
    QMatrix4x4 mProjViewMat;
    QVector3D mCameraPos;
    QVector3D mCameraFront;
    QVector3D mCameraUp;
    Directions mDirections;
    float mDeltaTime;
    float mLastFrame;
    bool mMouseGrabbed;
    QPoint mWindowCenter;
    float mPitch;
    float mYaw;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TutorialWindow::Directions)

#endif //GLTUT2_TUTORIALWINDOW_H

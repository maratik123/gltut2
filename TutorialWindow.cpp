//
// Created by maratik on 25.02.18.
//

#include "TutorialWindow.h"
#include <cmath>
#include <functional>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>
#include <QWheelEvent>
#include <QDateTime>
#include <QCoreApplication>
#include <QtMath>
#include <QApplication>
#include <QDesktopWidget>

namespace {
    struct VertexAttributes {
        QVector3D position;
        QVector2D texCoord;
    };

    constexpr std::array<VertexAttributes, 4> planeVertices { // NOLINT
            VertexAttributes { QVector3D(0.5f, 0.5f, 0.0f), QVector2D(1.0f, 1.0f) },
            VertexAttributes { QVector3D(0.5f, -0.5f, 0.0f), QVector2D(1.0f, 0.0f) },
            VertexAttributes { QVector3D(-0.5f, -0.5f, 0.0f), QVector2D(0.0f, 0.0f) },
            VertexAttributes { QVector3D(-0.5f, 0.5f, 0.0f), QVector2D(0.0f, 1.0f) }
    };
    constexpr std::array<unsigned int, 6> planeIndices {
            3, 2, 1,
            3, 1, 0
    };

    template <typename A>
    constexpr const A &clamp(const A &min, const A &value, const A &max) {
        if (Q_UNLIKELY(value < min)) {
            return min;
        }
        if (Q_LIKELY(value <= max)) {
            return value;
        }
        return max;
    };

    class Cube {
    public:
        Cube() noexcept;
        const auto &vertices() const { return mVertices; }
        const auto &indices() const { return mIndices; }

    private:
        static const size_t mPlanesCount = 6;
        std::array<VertexAttributes, mPlanesCount * planeVertices.size()> mVertices;
        std::array<unsigned int, mPlanesCount * planeIndices.size()> mIndices;
    };

    Cube::Cube() noexcept : mVertices(), mIndices() {
        auto verticesIt = mVertices.begin();
        auto updateVertices = [&verticesIt](auto initTransform) {
            QMatrix4x4 transform;
            initTransform(transform);
            for (const auto &planeVertex : planeVertices) {
                verticesIt->position = transform * planeVertex.position;
                verticesIt->texCoord = planeVertex.texCoord;
                ++verticesIt;
            }
        };
        updateVertices([](auto &front) {
            front.translate(0.0f, 0.0f, 0.5f);
        });
        updateVertices([](auto &back) {
            back.translate(0.0f, 0.0f, -0.5f);
            back.rotate(180.0f, 0.0f, 1.0f, 0.0f);
        });
        updateVertices([](auto &bottom) {
            bottom.translate(0.0f, -0.5f, 0.0f);
            bottom.rotate(90.0f, 1.0f, 0.0f, 0.0f);
        });
        updateVertices([](auto &top) {
            top.translate(0.0f, 0.5f, 0.0f);
            top.rotate(-90.0f, 1.0f, 0.0f, 0.0f);
        });
        updateVertices([](auto &left) {
            left.translate(-0.5f, 0.0f, 0.0f);
            left.rotate(-90.0f, 0.0f, 1.0f, 0.0f);
        });
        updateVertices([](auto &right) {
            right.translate(0.5f, 0.0f, 0.0f);
            right.rotate(90.0f, 0.0f, 1.0f, 0.0f);
        });
        for (std::size_t i = 0; i < mPlanesCount; ++i) {
            for (std::size_t j = 0; j < planeIndices.size(); ++j) {
                mIndices[i * mPlanesCount + j] = planeIndices[j] + i * planeVertices.size();
            }
        }
    }

    const Cube cube;

    constexpr std::array<QVector3D, 10>  cubePositions { // NOLINT
            QVector3D( 0.0f,  0.0f,  0.0f),
            QVector3D( 2.0f,  5.0f, -15.0f),
            QVector3D(-1.5f, -2.2f, -2.5f),
            QVector3D(-3.8f, -2.0f, -12.3f),
            QVector3D( 2.4f, -0.4f, -3.5f),
            QVector3D(-1.7f,  3.0f, -7.5f),
            QVector3D( 1.3f, -2.0f, -2.5f),
            QVector3D( 1.5f,  2.0f, -2.5f),
            QVector3D( 1.5f,  0.2f, -1.5f),
            QVector3D(-1.3f,  1.0f, -1.5f)
    };
}

TutorialWindow::TutorialWindow(bool enableLogger, QWindow *parent) :
        OpenGLWindow(enableLogger, parent),
        mProgram(nullptr),
        mVbo(nullptr),
        mLeftTriangleEbo(nullptr),
        mLeftTriangleVao(nullptr),
        mPrevSize(),
        mContainerTexture(nullptr),
        mAwesomeTexture(nullptr),
        mMixBalance(0.5f),
        mMixBalanceLocation(-1),
        mStartTime(QDateTime::currentMSecsSinceEpoch()),
        mTransformLocation(-1),
        mScreenRatio(1.0f),
        mViewMat(),
        mProjMat(),
        mProjViewMat(),
        mCameraPos(),
        mCameraFront(),
        mCameraUp(),
        mDirections(),
        mDeltaTime(0.0f),
        mLastFrame(0.0f),
        mMouseGrabbed(false),
        mWindowCenter(QApplication::desktop()->geometry().center()),
        mPitch(0.0f),
        mYaw(-90.0f) {
    QSurfaceFormat surfaceFormat(QSurfaceFormat::DebugContext);
    surfaceFormat.setSamples(16);
    surfaceFormat.setMajorVersion(4);
    surfaceFormat.setMinorVersion(5);
    surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
    surfaceFormat.setRenderableType(QSurfaceFormat::OpenGL);
    surfaceFormat.setSwapBehavior(QSurfaceFormat::TripleBuffer);
    surfaceFormat.setDepthBufferSize(24);
    surfaceFormat.setStencilBufferSize(8);
    surfaceFormat.setAlphaBufferSize(8);
    surfaceFormat.setRedBufferSize(8);
    surfaceFormat.setGreenBufferSize(8);
    surfaceFormat.setBlueBufferSize(8);
    setFormat(surfaceFormat);
}

void TutorialWindow::initialize() {
    initializeOpenGLFunctions();
    qDebug() << format();
    qDebug() << requestedFormat();

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_DEPTH_TEST);

    updateSize(size());

    mVbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    mVbo->create();
    mLeftTriangleEbo = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    mLeftTriangleEbo->create();
    mLeftTriangleVao = new QOpenGLVertexArrayObject(this);
    mLeftTriangleVao->create();

    mProgram = new QOpenGLShaderProgram(this);
    mProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, QStringLiteral(":/shaders/vertex.glsl"));
    mProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, QStringLiteral(":/shaders/fragment.glsl"));
    mProgram->link();
    mProgram->bind();

    mVbo->bind();
    mVbo->setUsagePattern(QOpenGLBuffer::StaticDraw);
    mVbo->allocate(cube.vertices().data(), sizeof(cube.vertices()));

    mLeftTriangleEbo->bind();
    mLeftTriangleEbo->setUsagePattern(QOpenGLBuffer::StaticDraw);
    mLeftTriangleEbo->allocate(cube.indices().data(), sizeof(cube.indices()));

    mContainerTexture = new QOpenGLTexture(QImage(QStringLiteral(":/textures/container.jpg")).mirrored());
    mContainerTexture->setWrapMode(QOpenGLTexture::Repeat);
    mContainerTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    mContainerTexture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    mProgram->setUniformValue("texture1", 0);

    mAwesomeTexture = new QOpenGLTexture(QImage(QStringLiteral(":/textures/awesomeface.png")).mirrored());
    mAwesomeTexture->setWrapMode(QOpenGLTexture::Repeat);
    mAwesomeTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    mAwesomeTexture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    mProgram->setUniformValue("texture2", 1);

    mMixBalanceLocation = mProgram->uniformLocation("mixBalance");
    mTransformLocation = mProgram->uniformLocation("transform");

    mCameraPos = QVector3D(0.0f, 0.0f, 3.0f);
    mCameraUp = QVector3D(0.0f, 1.0f, 0.0f);
    updateCameraFront();

    {
        const QOpenGLVertexArrayObject::Binder vao_binder(mLeftTriangleVao);

        mVbo->bind();
        mLeftTriangleEbo->bind();
        mProgram->setAttributeBuffer(0, GL_FLOAT, static_cast<int>(offsetof(VertexAttributes, position)), 3, sizeof(VertexAttributes));
        mProgram->enableAttributeArray(0);
        mProgram->setAttributeBuffer(1, GL_FLOAT, static_cast<int>(offsetof(VertexAttributes, texCoord)), 2, sizeof(VertexAttributes));
        mProgram->enableAttributeArray(1);
    }
}

void TutorialWindow::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const QSize &newSize = size();
    if (Q_UNLIKELY(newSize != mPrevSize)) {
        updateSize(newSize);
    }

    mProgram->bind();
    mContainerTexture->bind(0);
    mAwesomeTexture->bind(1);
    mProgram->setUniformValue(mMixBalanceLocation, mMixBalance);

    const float currentTime = static_cast<float>(QDateTime::currentMSecsSinceEpoch() - mStartTime) / 1000.0f;
    mDeltaTime = currentTime - mLastFrame;
    mLastFrame = currentTime;

    updateViewMat();

    int i = 0;
    for(const auto &cubePosition : cubePositions) {
        QMatrix4x4 model;
        model.translate(cubePosition);
        const float angle = 20.0f * i + currentTime * 50.0f;
        model.rotate(angle, 1.0f, 0.3f, 0.5f);
        mProgram->setUniformValue(mTransformLocation, mProjViewMat * model);
        {
            const QOpenGLVertexArrayObject::Binder vao_binder(mLeftTriangleVao);
            glDrawElements(GL_TRIANGLES, cube.indices().size(), GL_UNSIGNED_INT, nullptr);
        }
        ++i;
    }
}

void TutorialWindow::deinitialize() {
    if (mLeftTriangleVao != nullptr) {
        mLeftTriangleVao->destroy();
    }
    if (mVbo != nullptr) {
        mVbo->destroy();
    }
    if (mLeftTriangleEbo != nullptr) {
        mLeftTriangleEbo->destroy();
    }
    if (mProgram != nullptr) {
        mProgram->removeAllShaders();
    }
    if (mContainerTexture != nullptr) {
        mContainerTexture->destroy();
    }
    if (mAwesomeTexture != nullptr) {
        mAwesomeTexture->destroy();
    }
}

TutorialWindow::~TutorialWindow() {
    delete mContainerTexture;
    delete mAwesomeTexture;
    delete mVbo;
    delete mLeftTriangleEbo;
}

void TutorialWindow::updateSize(const QSize &newSize) {
    const double dpr = devicePixelRatio();
    const int width = newSize.width();
    const int height = newSize.height();
    glViewport(0, 0,
               static_cast<GLsizei>(std::lround(width * dpr)),
               static_cast<GLsizei>(std::lround(height * dpr))
    );
    mPrevSize = newSize;
    mScreenRatio = height == 0 ? 1.0f : static_cast<float>(width) / static_cast<float>(height);
    mProjMat.setToIdentity();
    mProjMat.perspective(45.0f, mScreenRatio, 0.1f, 100.0f);
    updateProjViewMat();
}

void TutorialWindow::wheelEvent(QWheelEvent *event) {
    event->accept();
    const int angleDelta = event->angleDelta().y();
    if (Q_LIKELY(angleDelta != 0)) {
        updateMixBalance(static_cast<float>(angleDelta) / 120.0f / 25.0f);
        return;
    }
    const int numPixels = event->pixelDelta().y();
    if (Q_LIKELY(numPixels != 0)) {
        updateMixBalance(static_cast<float>(numPixels) / 250.0f);
    }
}

void TutorialWindow::updateMixBalance(float delta) {
    mMixBalance = clamp(0.0f, mMixBalance + delta, 1.0f);
}

void TutorialWindow::updateProjViewMat() {
    mProjViewMat = mProjMat * mViewMat;
}

void TutorialWindow::updateViewMat() {
    if (!mDirections) {
        return;
    }
    const bool forward = mDirections.testFlag(Direction::Forward);
    const bool backward = mDirections.testFlag(Direction::Backward);
    const bool frontMove = forward != backward;
    const bool left = mDirections.testFlag(Direction::Left);
    const bool right = mDirections.testFlag(Direction::Right);
    const bool strafeMove = left != right;
    if (!frontMove && !strafeMove) {
        return;
    }
    const float cameraSpeed = 2.5f * mDeltaTime;
    if (frontMove) {
        const float frontSpeed = forward ? cameraSpeed : -cameraSpeed;
        mCameraPos += frontSpeed * mCameraFront;
    }
    if (strafeMove) {
        const float strafeSpeed = right ? cameraSpeed : -cameraSpeed;
        mCameraPos += strafeSpeed * QVector3D::crossProduct(mCameraFront, mCameraUp);
    }
    explicitUpdateViewMat();
}

void TutorialWindow::explicitUpdateViewMat() {
    mViewMat.setToIdentity();
    mViewMat.lookAt(mCameraPos, mCameraPos + mCameraFront, mCameraUp);
    updateProjViewMat();
}

void TutorialWindow::keyPressEvent(QKeyEvent *event) {
    if (Q_LIKELY(keyEvent(event, true))) {
        return;
    }
    switch (event->key()) {
        case Qt::Key_Escape:
            QCoreApplication::postEvent(this, new QEvent(QEvent::Close));
            break;
        case Qt::Key_BracketLeft:
            updateMixBalance(-0.05f);
            break;
        case Qt::Key_BracketRight:
            updateMixBalance(0.05f);
            break;
        default:
            OpenGLWindow::keyPressEvent(event);
            break;
    }
}

void TutorialWindow::keyReleaseEvent(QKeyEvent *event) {
    if (Q_UNLIKELY(!keyEvent(event, false))) {
        OpenGLWindow::keyReleaseEvent(event);
    }
}

bool TutorialWindow::keyEvent(QKeyEvent *event, bool isKeyPressed) {
    switch (event->key()) {
        case Qt::Key_W:
        case Qt::Key_Up:
            mDirections.setFlag(Direction::Forward, isKeyPressed);
            break;
        case Qt::Key_S:
        case Qt::Key_Down:
            mDirections.setFlag(Direction::Backward, isKeyPressed);
            break;
        case Qt::Key_A:
        case Qt::Key_Left:
            mDirections.setFlag(Direction::Left, isKeyPressed);
            break;
        case Qt::Key_D:
        case Qt::Key_Right:
            mDirections.setFlag(Direction::Right, isKeyPressed);
            break;
        default:
            return false;
    }
    return true;
}

void TutorialWindow::mousePressEvent(QMouseEvent *event) {
    if (Q_LIKELY(event->button() == Qt::LeftButton)) {
        mMouseGrabbed = !mMouseGrabbed;
        setMouseGrabEnabled(mMouseGrabbed);
        QCursor::setPos(mWindowCenter);
        setCursor(mMouseGrabbed ? Qt::BlankCursor : Qt::ArrowCursor);
        return;
    }
    OpenGLWindow::mousePressEvent(event);
}

void TutorialWindow::mouseMoveEvent(QMouseEvent *event) {
    if (Q_UNLIKELY(!mMouseGrabbed)) {
        OpenGLWindow::mouseMoveEvent(event);
        return;
    }
    const QPoint &pos = event->globalPos();
    if (pos == mWindowCenter) {
        return;
    }
    const QPoint &offset = pos - mWindowCenter;
    const float sensitivity = 0.05f;
    const float xOffset = static_cast<float>(offset.x()) * sensitivity;
    const float yOffset = static_cast<float>(offset.y()) * sensitivity;
    mYaw += xOffset;
    mPitch = clamp(-89.0f, mPitch + yOffset, 89.0f);
    updateCameraFront();

    QCursor::setPos(mWindowCenter);
}

void TutorialWindow::updateCameraFront() {
    const float yawRad = qDegreesToRadians(mYaw);
    const float pitchRad = qDegreesToRadians(mPitch);
    const auto sinYaw = static_cast<float>(qFastSin(yawRad));
    const auto cosYaw = static_cast<float>(qFastCos(yawRad));
    const auto sinPitch = static_cast<float>(qFastSin(pitchRad));
    const auto cosPitch = static_cast<float>(qFastCos(pitchRad));
    mCameraFront = QVector3D(cosYaw * cosPitch, -sinPitch, sinYaw * cosPitch).normalized();
    explicitUpdateViewMat();
}

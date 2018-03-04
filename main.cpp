#include "TutorialWindow.h"
#include <QOpenGLDebugMessage>
#include <QApplication>

int main(int argc, char *argv[]) {
    const QApplication application(argc, argv);
    const QString &applicationName = QStringLiteral("LearnOpenGL");

    QCoreApplication::setApplicationName(applicationName);
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0"));

    TutorialWindow window(true);
    window.setTitle(applicationName);
    QObject::connect(&window, &OpenGLWindow::messageLogged, [](const auto &message){ qDebug() << message; });
    window.resize(800, 600);
    window.show();

    window.setAnimation(true);

    return QApplication::exec();
}

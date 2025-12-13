#include <QtTest/QtTest>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    extern int runSceneManagementTests(int argc, char** argv);
    extern int runFileIoTests(int argc, char** argv);
    extern int runUndoRedoTests(int argc, char** argv);

    int status = 0;
    status |= runSceneManagementTests(argc, argv);
    status |= runFileIoTests(argc, argv);
    status |= runUndoRedoTests(argc, argv);

    return status;
}

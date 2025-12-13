#include <QtTest/QtTest>
#include <QApplication>
#include <QVector>
#include <QByteArray>

// 双保险：拦截 Qt 的 message（如果有输出走到这里就丢弃）
static void testMessageHandler(QtMsgType type, const QMessageLogContext&, const QString&)
{
    // 丢弃 debug/info/warning，减少噪音与卡顿
    if (type == QtDebugMsg || type == QtInfoMsg || type == QtWarningMsg) return;
    // critical/fatal 仍输出（可按需要扩展）
}

// 为每个 QTest::qExec 构造 argv，自动注入 -silent / -maxwarnings
static int qExecSilent(QObject* testObject, int argc, char** argv)
{
    // 保留原始参数，但如果用户显式传了 -silent，就不重复注入
    bool hasSilent = false;
    bool hasMaxwarnings = false;

    for (int i = 1; i < argc; ++i) {
        const QString a = QString::fromLocal8Bit(argv[i]);
        if (a == "-silent") hasSilent = true;
        if (a.startsWith("-maxwarnings")) hasMaxwarnings = true;
    }

    QVector<QByteArray> storage;
    QVector<char*> newArgv;

    // argv[0]
    storage.push_back(QByteArray(argv[0]));
    newArgv.push_back(storage.back().data());

    // 复制原始参数
    for (int i = 1; i < argc; ++i) {
        storage.push_back(QByteArray(argv[i]));
        newArgv.push_back(storage.back().data());
    }

    // 自动注入：不需要用户加 -silent
    if (!hasSilent) {
        storage.push_back(QByteArray("-silent"));
        newArgv.push_back(storage.back().data());
    }

    // 让 QtTest 不因为 warning 太多而截断输出（但我们已经 -silent）
    // 也可以设为 0，表示不要输出过多 warning（配合 -silent 基本无输出）
    if (!hasMaxwarnings) {
        storage.push_back(QByteArray("-maxwarnings"));
        newArgv.push_back(storage.back().data());
        storage.push_back(QByteArray("0"));
        newArgv.push_back(storage.back().data());
    }

    int newArgc = newArgv.size();
    return QTest::qExec(testObject, newArgc, newArgv.data());
}

int main(int argc, char** argv)
{
    // 关键：在 QApplication 前设置，避免 native QFileDialog 导致卡死/未响应
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs, true);

    // 双保险：丢弃 debug/info/warn（即便 QtTest 捕获不到也尽量抑制）
    qInstallMessageHandler(testMessageHandler);

    QApplication app(argc, argv);

    extern int runSceneManagementTests(int argc, char** argv);
    extern int runFileIoTests(int argc, char** argv);
    extern int runUndoRedoTests(int argc, char** argv);

    extern int runFindReplaceDialogTests(int argc, char** argv);
    extern int runDiagramTextItemEditTests(int argc, char** argv);
    extern int runDiagramPathConnectionTests(int argc, char** argv);
    extern int runDiagramItemCreationTests(int argc, char** argv);
    extern int runDiagramItemTransformTests(int argc, char** argv);
    extern int runDiagramItemPropertiesTests(int argc, char** argv);
    extern int runConnectionLineStyleTests(int argc, char** argv);
    extern int runArrowStraightConnectionTests(int argc, char** argv);

    // 由于你现在的 runXXXTests 里是 QTest::qExec(&tc, argc, argv)
    // 为了统一静默，我们不再调用 runXXXTests，而是直接 qExecSilent(&tc,...)
    // 所以这里需要把每个 test class 自己在各 cpp 内暴露出来才行（不现实）。
    //
    // ✅ 更简单：保持 runXXXTests 不变，但要求它们内部调用 QTest::qExec 时也用“silent argv”。
    // 我们在这里提供一个全局可用的 wrapper：runXXXTests 仍然接收 argc/argv，
    // 但它们内部若直接 QTest::qExec(tc, argc, argv) 则不会注入 -silent。
    //
    // 因此：我们在这里“手动注入 argv”，然后把注入后的 argc/argv 传给所有 runXXXTests。
    //
    QVector<QByteArray> storage;
    QVector<char*> newArgv;

    bool hasSilent = false;
    bool hasMaxwarnings = false;
    for (int i = 1; i < argc; ++i) {
        const QString a = QString::fromLocal8Bit(argv[i]);
        if (a == "-silent") hasSilent = true;
        if (a.startsWith("-maxwarnings")) hasMaxwarnings = true;
    }

    storage.push_back(QByteArray(argv[0]));
    newArgv.push_back(storage.back().data());
    for (int i = 1; i < argc; ++i) {
        storage.push_back(QByteArray(argv[i]));
        newArgv.push_back(storage.back().data());
    }
    if (!hasSilent) {
        storage.push_back(QByteArray("-silent"));
        newArgv.push_back(storage.back().data());
    }
    if (!hasMaxwarnings) {
        storage.push_back(QByteArray("-maxwarnings"));
        newArgv.push_back(storage.back().data());
        storage.push_back(QByteArray("0"));
        newArgv.push_back(storage.back().data());
    }

    int injectedArgc = newArgv.size();
    char** injectedArgv = newArgv.data();

    int status = 0;
    status |= runSceneManagementTests(injectedArgc, injectedArgv);
    status |= runFileIoTests(injectedArgc, injectedArgv);
    status |= runUndoRedoTests(injectedArgc, injectedArgv);

    status |= runFindReplaceDialogTests(injectedArgc, injectedArgv);
    status |= runDiagramTextItemEditTests(injectedArgc, injectedArgv);
    status |= runDiagramPathConnectionTests(injectedArgc, injectedArgv);
    status |= runDiagramItemCreationTests(injectedArgc, injectedArgv);
    status |= runDiagramItemTransformTests(injectedArgc, injectedArgv);
    status |= runDiagramItemPropertiesTests(injectedArgc, injectedArgv);
    status |= runConnectionLineStyleTests(injectedArgc, injectedArgv);
    status |= runArrowStraightConnectionTests(injectedArgc, injectedArgv);

    return status;
}

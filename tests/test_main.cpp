#include <QtTest/QtTest>
#include <QApplication>
#include <QVector>
#include <QByteArray>

// ========== 1. 修复日志处理器：保留覆盖率相关日志 ==========
static void testMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    // 仅放行覆盖率相关日志，其余debug/info/warning丢弃
    if (msg.contains("覆盖率") || msg.contains("gcov")) {
        // 覆盖率日志正常输出
        switch (type) {
        case QtInfoMsg:
            fprintf(stderr, "INFO: %s\n", msg.toLocal8Bit().constData());
            break;
        case QtWarningMsg:
            fprintf(stderr, "WARNING: %s\n", msg.toLocal8Bit().constData());
            break;
        case QtCriticalMsg:
            fprintf(stderr, "CRITICAL: %s\n", msg.toLocal8Bit().constData());
            break;
        case QtFatalMsg:
            fprintf(stderr, "FATAL: %s\n", msg.toLocal8Bit().constData());
            abort();
        default:
            break;
        }
        return;
    }
    // 其他非覆盖率日志：debug/info/warning丢弃，critical/fatal保留
    if (type == QtDebugMsg || type == QtInfoMsg || type == QtWarningMsg) return;
    if (type == QtCriticalMsg) {
        fprintf(stderr, "CRITICAL: %s\n", msg.toLocal8Bit().constData());
    } else if (type == QtFatalMsg) {
        fprintf(stderr, "FATAL: %s\n", msg.toLocal8Bit().constData());
        abort();
    }
}

// ========== 2. 保留原有静默执行逻辑 ==========
static int qExecSilent(QObject* testObject, int argc, char** argv)
{
    bool hasSilent = false;
    bool hasMaxwarnings = false;

    for (int i = 1; i < argc; ++i) {
        const QString a = QString::fromLocal8Bit(argv[i]);
        if (a == "-silent") hasSilent = true;
        if (a.startsWith("-maxwarnings")) hasMaxwarnings = true;
    }

    QVector<QByteArray> storage;
    QVector<char*> newArgv;

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

    int newArgc = newArgv.size();
    return QTest::qExec(testObject, newArgc, newArgv.data());
}

// ========== 3. 声明覆盖率测试入口函数 ==========
extern int runDiagramItemTypesFullTests(int argc, char** argv);

// ========== 4. 原有测试函数声明（保留） ==========
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
extern int runArrowConnectionSystemTests(int argc, char** argv);
extern int runDiagramPathPathDevTests(int argc, char** argv);
extern int runComponentSignalIntegrateTests(int argc, char** argv);
extern int runDataflowIntegrityTests(int argc, char** argv);
extern int runPerformanceWorkflowTests(int argc, char** argv);
extern int runPerformanceStressTests(int argc, char** argv);
extern int runShortcutTests(int argc, char** argv);
int main(int argc, char** argv)
{
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs, true);

    // 安装修复后的日志处理器（保留覆盖率日志）
    qInstallMessageHandler(testMessageHandler);

    QApplication app(argc, argv);

    // ========== 5. 构造注入后的参数（保留原有逻辑） ==========
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

    // ========== 6. 执行测试：先执行覆盖率测试，再执行其他 ==========
    int status = 0;

    // ✅ 核心：执行DiagramItem覆盖率测试（优先执行，确保数据不被覆盖）
    status |= runDiagramItemTypesFullTests(injectedArgc, injectedArgv);

    // 原有测试（保留）
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
    status |= runArrowConnectionSystemTests(injectedArgc, injectedArgv);
    status |= runDiagramPathPathDevTests(injectedArgc, injectedArgv);
    status |= runComponentSignalIntegrateTests(injectedArgc, injectedArgv);
    status |= runDataflowIntegrityTests(injectedArgc, injectedArgv);
    status |= runPerformanceWorkflowTests(injectedArgc, injectedArgv);
    status |= runPerformanceStressTests(injectedArgc, injectedArgv);
    status |= runDiagramTextItemEditTests(injectedArgc, injectedArgv);
    status |= runShortcutTests(injectedArgc, injectedArgv);

    return status;
}

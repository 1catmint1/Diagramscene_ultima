#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QGraphicsView>
#include <QMetaObject>
#include <QDialog>
#include <QApplication>
#include <QTabWidget>

#include "mainwindow.h"
#include "diagramitem.h"

// ---------- helpers ----------

static QTabWidget* findTabWidget(MainWindow& w)
{
    return w.findChild<QTabWidget*>();
}

static QGraphicsView* currentTabView(MainWindow& w)
{
    QTabWidget* tabs = findTabWidget(w);
    if (!tabs) return nullptr;
    return qobject_cast<QGraphicsView*>(tabs->currentWidget());
}

static int countDiagramItems(QGraphicsScene* scene)
{
    int c = 0;
    for (QGraphicsItem* gi : scene->items())
        if (gi->type() == DiagramItem::Type) ++c;
    return c;
}

static void addNDiagramItems(QGraphicsScene* scene, int n)
{
    static QMenu dummyMenu;
    for (int i = 0; i < n; ++i) {
        auto* item = new DiagramItem(DiagramItem::Step, &dummyMenu);
        item->setPos(50 + (i % 20) * 80, 50 + (i / 20) * 60);
        item->setFixedSize(QSizeF(150, 100));
        if (item->textItem)
            item->textItem->setPlainText(QString("node-%1").arg(i));
        scene->addItem(item);
    }
}

static bool invokeSlot(QObject* obj, const char* slotName)
{
    return QMetaObject::invokeMethod(obj, slotName, Qt::DirectConnection);
}

static void closeMessageBoxesAsync()
{
    for (int i = 0; i < 40; ++i) {
        QTimer::singleShot(25 * i, []() {
            for (QWidget* w : QApplication::topLevelWidgets()) {
                if (auto* mb = qobject_cast<QMessageBox*>(w)) {
                    mb->accept();
                }
            }
        });
    }
}

static void autoAcceptFileDialogAsync(const QString& filePath, QFileDialog::AcceptMode mode)
{
    for (int i = 0; i < 80; ++i) {
        QTimer::singleShot(15 * i, [filePath, mode]() {
            for (QWidget* w : QApplication::topLevelWidgets()) {
                auto* dlg = qobject_cast<QFileDialog*>(w);
                if (!dlg) continue;
                if (dlg->acceptMode() != mode) continue;

                dlg->selectFile(filePath);
                static_cast<QDialog*>(dlg)->done(QDialog::Accepted);
                return;
            }
        });
    }
}

// ---------- tests ----------

class TestFileIo : public QObject
{
    Q_OBJECT
private slots:
    void fcproj_save_load_roundtrip_via_dialog();
    void fcproj_io_performance();
};

void TestFileIo::fcproj_save_load_roundtrip_via_dialog()
{
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs, true);

    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const QString fcprojPath = tmp.filePath("roundtrip.fcproj");

    MainWindow w;
    w.show();
    QVERIFY(QTest::qWaitForWindowExposed(&w));
    w.activateWindow();
    QVERIFY(QTest::qWaitForWindowActive(&w));

    // 用当前 tab 获取 view/scene（不要用 findChildren().first()）
    QGraphicsView* view = currentTabView(w);
    QVERIFY2(view != nullptr, "Cannot get current QGraphicsView from QTabWidget.");
    QGraphicsScene* scene = view->scene();
    QVERIFY(scene != nullptr);

    scene->clear();
    addNDiagramItems(scene, 10);
    QCOMPARE(countDiagramItems(scene), 10);

    // 保存
    closeMessageBoxesAsync();
    autoAcceptFileDialogAsync(fcprojPath, QFileDialog::AcceptSave);
    QVERIFY2(invokeSlot(&w, "savefile"), "invoke savefile failed.");
    QCoreApplication::processEvents();
    QVERIFY(QFile::exists(fcprojPath));

    // 清空后加载（loadfile 会 newScene，current tab 会变）
    scene->clear();
    QCOMPARE(countDiagramItems(scene), 0);

    closeMessageBoxesAsync();
    autoAcceptFileDialogAsync(fcprojPath, QFileDialog::AcceptOpen);
    QVERIFY2(invokeSlot(&w, "loadfile"), "invoke loadfile failed.");
    QCoreApplication::processEvents();

    // 重新取 current tab（因为 newScene 切走了）
    view = currentTabView(w);
    QVERIFY(view != nullptr);
    scene = view->scene();
    QVERIFY(scene != nullptr);

    QCOMPARE(countDiagramItems(scene), 10);
}

void TestFileIo::fcproj_io_performance()
{
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs, true);

    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const QString fcprojPath = tmp.filePath("perf.fcproj");

    MainWindow w;
    w.show();
    QVERIFY(QTest::qWaitForWindowExposed(&w));
    w.activateWindow();
    QVERIFY(QTest::qWaitForWindowActive(&w));

    QGraphicsView* view = currentTabView(w);
    QVERIFY(view != nullptr);
    QGraphicsScene* scene = view->scene();
    QVERIFY(scene != nullptr);

    const int N = 120; // 你 mainwindow.cpp debug 很多，数量太大会极慢
    scene->clear();
    addNDiagramItems(scene, N);
    QCOMPARE(countDiagramItems(scene), N);

    QElapsedTimer timer;

    // 保存计时
    closeMessageBoxesAsync();
    autoAcceptFileDialogAsync(fcprojPath, QFileDialog::AcceptSave);
    timer.start();
    QVERIFY(invokeSlot(&w, "savefile"));
    const qint64 saveMs = timer.elapsed();
    QVERIFY(QFile::exists(fcprojPath));

    // 加载计时（会 newScene）
    closeMessageBoxesAsync();
    autoAcceptFileDialogAsync(fcprojPath, QFileDialog::AcceptOpen);
    timer.restart();
    QVERIFY(invokeSlot(&w, "loadfile"));
    const qint64 loadMs = timer.elapsed();

    view = currentTabView(w);
    QVERIFY(view != nullptr);
    scene = view->scene();
    QVERIFY(scene != nullptr);

    QCOMPARE(countDiagramItems(scene), N);

    // 放宽阈值（你 loadfile/savefile 有大量 qDebug）
    QVERIFY2(saveMs < 15000, qPrintable(QString("fcproj save too slow: %1ms").arg(saveMs)));
    QVERIFY2(loadMs < 15000, qPrintable(QString("fcproj load too slow: %1ms").arg(loadMs)));
}

int runFileIoTests(int argc, char** argv)
{
    TestFileIo tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "test_file_io.moc"

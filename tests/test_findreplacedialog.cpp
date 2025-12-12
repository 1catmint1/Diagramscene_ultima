#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QLineEdit>
#include <QPushButton>
#include <QDialog>
#include <QStringList>
#include <QElapsedTimer>
#include <stdexcept>

#include "../findreplacedialog.h"

// =====================================================
// 1) 黑盒控件定位（不访问 private 成员）
// =====================================================
static QLineEdit* pickLineEditOrThrow(QDialog &dlg, int index)
{
    const auto edits = dlg.findChildren<QLineEdit*>();
    if (edits.size() < 2)
        throw std::runtime_error("未找到足够的 QLineEdit（至少需要查找/替换两个）");
    if (index < 0 || index >= edits.size())
        throw std::runtime_error("pickLineEdit index 越界");
    return edits.at(index);
}

static QPushButton* pickButtonByTextOrThrow(QDialog &dlg, const QString &text)
{
    const auto btns = dlg.findChildren<QPushButton*>();
    for (auto *b : btns)
        if (b && b->text() == text)
            return b;

    QStringList names;
    for (auto *b : btns)
        if (b) names << b->text();

    throw std::runtime_error(
        QString("未找到按钮 '%1'，当前按钮：[%2]")
            .arg(text, names.join(", "))
            .toStdString());
}

// =====================================================
// 2) 输入工具（避免 Unicode / 控制字符触发 qasciikey）
// =====================================================
static bool isPureAsciiPrintableNoCtl(const QString &s)
{
    for (QChar c : s) {
        const ushort u = c.unicode();
        if (u > 0x7F) return false;          // 非 ASCII
        if (u < 0x20) return false;          // 控制字符（含 \n \t 等）
        if (u == 0x7F) return false;
    }
    return true;
}

static void setLineEditSmart(QLineEdit *edit, const QString &text)
{
    QVERIFY(edit);
    edit->setFocus();
    edit->selectAll();
    edit->del();

    // ASCII 且不含控制字符：用 keyClicks 更像用户输入
    // 否则：直接 setText，避免 qasciikey 对 Unicode/控制字符断言
    if (isPureAsciiPrintableNoCtl(text)) {
        QTest::keyClicks(edit, text);
    } else {
        edit->setText(text);
    }

    QCoreApplication::processEvents();
}

// =====================================================
// 3) FakeEditor：模拟外部编辑器逻辑（与测试语义一致）
//    - find: case-sensitive, find-next non-overlap
//    - replace: 仅替换 lastStart 命中（通常由 find 提供）
//    - replaceAll: 非重叠替换（pos += repl.size）
// =====================================================
class FakeEditor : public QObject
{
    Q_OBJECT
public:
    QString text;
    int cursor = 0;
    int lastStart = -1;

    explicit FakeEditor(QString init, QObject *p=nullptr)
        : QObject(p), text(std::move(init)) {}

public slots:
    void onFind(const QString &needle)
    {
        if (needle.isEmpty()) {
            lastStart = -1;
            return;
        }
        const int pos = text.indexOf(needle, cursor, Qt::CaseSensitive);
        if (pos >= 0) {
            lastStart = pos;
            cursor = pos + needle.size();   // 非重叠 find-next
        } else {
            lastStart = -1;
        }
    }

    void onReplace(const QString &needle, const QString &repl)
    {
        if (needle.isEmpty()) return;
        if (lastStart < 0) return;
        if (text.mid(lastStart, needle.size()) != needle) return; // 防御
        text.replace(lastStart, needle.size(), repl);
        cursor = lastStart + repl.size();
    }

    void onReplaceAll(const QString &needle, const QString &repl)
    {
        if (needle.isEmpty()) return;
        int pos = 0;
        while (true) {
            pos = text.indexOf(needle, pos, Qt::CaseSensitive);
            if (pos < 0) break;
            text.replace(pos, needle.size(), repl);
            pos += repl.size();  // 非重叠推进：关键！避免 repl 含 needle 时死循环
        }
        lastStart = -1;
        cursor = 0;
    }
};

// =====================================================
// 4) 方案B：replaceAll 期望值运行时推导（非重叠）
// =====================================================
static QString computeReplaceAllNonOverlapping(QString src,
                                               const QString &needle,
                                               const QString &repl)
{
    if (needle.isEmpty())
        return src;

    int pos = 0;
    while (true) {
        pos = src.indexOf(needle, pos, Qt::CaseSensitive);
        if (pos < 0) break;
        src.replace(pos, needle.size(), repl);
        pos += repl.size();
    }
    return src;
}

// =====================================================
// 5) TestFindReplaceDialog：等价类覆盖 + 边界值 + stress
// =====================================================
class TestFindReplaceDialog : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void algorithm_equivalence_data();
    void algorithm_equivalence();

    void stress_replaceAll_longText();
};

// ---------------- 数据：等价类 + 边界值 ----------------
//
// 字段说明：
//  initial           : 初始文本（haystack）
//  needle            : 查找串
//  repl              : 替换串
//  expectFindPos1    : 第一次 find 的期望位置（-1 表示找不到或 needle 为空）
//  doReplaceAfterFind: 是否执行“find 后 replace”
//  expectAfterReplace: 执行 replace 后文本（若 doReplaceAfterFind=false 则忽略）
//
void TestFindReplaceDialog::algorithm_equivalence_data()
{
    QTest::addColumn<QString>("initial");
    QTest::addColumn<QString>("needle");
    QTest::addColumn<QString>("repl");
    QTest::addColumn<int>("expectFindPos1");
    QTest::addColumn<bool>("doReplaceAfterFind");
    QTest::addColumn<QString>("expectAfterReplace");

    // A) Needle 空（边界）
    QTest::newRow("needle_empty")
        << "abc" << "" << "X"
        << -1 << false << "";

    // B) 空文本（边界）
    QTest::newRow("text_empty")
        << "" << "a" << "x"
        << -1 << false << "";

    // C) needle 比文本长（边界）
    QTest::newRow("needle_longer_than_text")
        << "ab" << "abc" << "X"
        << -1 << false << "";

    // D) 单字符 needle（最小非空）
    QTest::newRow("needle_len1_hit")
        << "bbb" << "b" << "x"
        << 0 << true << "xbb";

    // E) 命中在开头/中间/末尾
    QTest::newRow("hit_at_begin")
        << "abcxx" << "abc" << "DEF"
        << 0 << true << "DEFxx";

    QTest::newRow("hit_in_middle")
        << "xxabcxx" << "abc" << "DEF"
        << 2 << true << "xxDEFxx";

    QTest::newRow("hit_at_end")
        << "xxabc" << "abc" << "DEF"
        << 2 << true << "xxDEF";

    // F) 多次命中（非重叠）
    QTest::newRow("multi_hits_nonoverlap")
        << "abc--abc--X" << "abc" << "DEF"
        << 0 << true << "DEF--abc--X";

    // G) 删除替换（repl 为空）
    QTest::newRow("delete_repl")
        << "aaabaa" << "aa" << ""
        << 0 << true << "abaa";

    // H) 重叠命中（锁定语义）
    QTest::newRow("overlap_case_find_replace")
        << "aaaaa" << "aa" << ""
        << 0 << true << "aaa";

    // I) 特殊字符（普通字符串匹配，不是正则）
    QTest::newRow("special_chars")
        << "a*.+?b*.+?c" << "*.+?" << "Z"
        << 1 << true << "aZb*.+?c";

    // J) Unicode 文本/needle/repl
    QTest::newRow("unicode_all")
        << QStringLiteral("中文中文A") << QStringLiteral("中文") << QStringLiteral("测")
        << 0 << true << QStringLiteral("测中文A");

    // K) 大小写敏感找不到（若 CaseSensitive）
    QTest::newRow("case_sensitive_not_found")
        << "Abc abc" << "ABC" << "X"
        << -1 << false << "";

    // ===== 你新增的场景：必须覆盖 =====

    // 1) 替换串包含查找串：防止 replaceAll 无限循环
    // initial="a", needle="a", repl="aa" → replaceAll 后应为 "aa"
    QTest::newRow("repl_contains_needle_no_infinite_loop")
        << "a" << "a" << "aa"
        << 0 << true << "aa";

    // 2) 查找串 == 替换串：文本不变（replace/replaceAll 都应不变）
    QTest::newRow("needle_eq_repl_idempotent")
        << "xxx" << "x" << "x"
        << 0 << true << "xxx";

    // 3) 全角/半角字符：默认不做归一化 -> 应找不到（如果你要“支持”，改期望即可）
    QTest::newRow("fullwidth_halfwidth_mismatch_default")
        << QStringLiteral("ＡＢＣ") << QStringLiteral("ABC") << QStringLiteral("X")
        << -1 << false << "";

    // 4) 换行符支持：多行查找/替换
    QTest::newRow("newline_support")
        << QStringLiteral("a\nb") << QStringLiteral("\n") << QStringLiteral("|")
        << 1 << true << QStringLiteral("a|b");

    // 5) 制表符支持：\t 查找替换
    QTest::newRow("tab_support")
        << QStringLiteral("a\tb") << QStringLiteral("\t") << QStringLiteral(" ")
        << 1 << true << QStringLiteral("a b");
}

// ---------------- 测试主体 ----------------
void TestFindReplaceDialog::algorithm_equivalence()
{
    QFETCH(QString, initial);
    QFETCH(QString, needle);
    QFETCH(QString, repl);
    QFETCH(int, expectFindPos1);
    QFETCH(bool, doReplaceAfterFind);
    QFETCH(QString, expectAfterReplace);

    FindReplaceDialog dlg;
    dlg.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dlg));

    FakeEditor editor(initial);
    connect(&dlg, SIGNAL(findText(QString)), &editor, SLOT(onFind(QString)));
    connect(&dlg, SIGNAL(replaceText(QString,QString)), &editor, SLOT(onReplace(QString,QString)));
    connect(&dlg, SIGNAL(replaceAllText(QString,QString)), &editor, SLOT(onReplaceAll(QString,QString)));

    QLineEdit *findEdit = nullptr;
    QLineEdit *repEdit  = nullptr;
    QPushButton *btnFind = nullptr;
    QPushButton *btnReplace = nullptr;
    QPushButton *btnReplaceAll = nullptr;

    try {
        findEdit = pickLineEditOrThrow(dlg, 0);
        repEdit  = pickLineEditOrThrow(dlg, 1);
        btnFind       = pickButtonByTextOrThrow(dlg, QStringLiteral("查找下一个"));
        btnReplace    = pickButtonByTextOrThrow(dlg, QStringLiteral("替换"));
        btnReplaceAll = pickButtonByTextOrThrow(dlg, QStringLiteral("全部替换"));
    } catch (const std::exception &e) {
        QFAIL(e.what());
    }

    // 设置输入
    setLineEditSmart(findEdit, needle);
    setLineEditSmart(repEdit,  repl);

    // ---------- Find ----------
    QTest::mouseClick(btnFind, Qt::LeftButton);
    QCoreApplication::processEvents();
    QCOMPARE(editor.lastStart, expectFindPos1);

    // ---------- Replace（单次） ----------
    if (doReplaceAfterFind) {
        QTest::mouseClick(btnReplace, Qt::LeftButton);
        QCoreApplication::processEvents();
        QCOMPARE(editor.text, expectAfterReplace);
    } else {
        // 若不应替换：点 replace 不应改变文本
        const QString before = editor.text;
        QTest::mouseClick(btnReplace, Qt::LeftButton);
        QCoreApplication::processEvents();
        QCOMPARE(editor.text, before);
    }

    // ---------- ReplaceAll（方案 B：运行时推导期望） ----------
    editor.text = initial;
    editor.cursor = 0;
    editor.lastStart = -1;

    QTest::mouseClick(btnReplaceAll, Qt::LeftButton);
    QCoreApplication::processEvents();

    const QString expectedAll = computeReplaceAllNonOverlapping(initial, needle, repl);
    QCOMPARE(editor.text, expectedAll);
}

// ---------------- Stress：超长字符串 replaceAll 不崩溃且结果正确 ----------------
void TestFindReplaceDialog::stress_replaceAll_longText()
{
    FindReplaceDialog dlg;
    dlg.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dlg));

    // 构造一个较长文本：重复 "abc-" N 次，再加尾巴
    const int N = 200000; // 20万段，量级足够验证“不卡死/不崩溃”
    QString initial;
    initial.reserve(N * 4 + 10);
    for (int i = 0; i < N; ++i) initial += "abc-";
    initial += "END";

    const QString needle = "abc";
    const QString repl   = "X";  // 变短，压力更大一些（多次修改字符串）

    FakeEditor editor(initial);
    connect(&dlg, SIGNAL(replaceAllText(QString,QString)), &editor, SLOT(onReplaceAll(QString,QString)));

    QLineEdit *findEdit = nullptr;
    QLineEdit *repEdit  = nullptr;
    QPushButton *btnReplaceAll = nullptr;

    try {
        findEdit = pickLineEditOrThrow(dlg, 0);
        repEdit  = pickLineEditOrThrow(dlg, 1);
        btnReplaceAll = pickButtonByTextOrThrow(dlg, QStringLiteral("全部替换"));
    } catch (const std::exception &e) {
        QFAIL(e.what());
    }

    setLineEditSmart(findEdit, needle);
    setLineEditSmart(repEdit,  repl);

    QElapsedTimer timer;
    timer.start();

    QTest::mouseClick(btnReplaceAll, Qt::LeftButton);
    QCoreApplication::processEvents();

    const qint64 ms = timer.elapsed();
    qInfo("stress_replaceAll_longText elapsed: %lld ms", (long long)ms);

    // 不做严格耗时断言（避免机器差异导致 flaky），只验证结果正确且能完成
    const QString expectedAll = computeReplaceAllNonOverlapping(initial, needle, repl);
    QCOMPARE(editor.text, expectedAll);
}

QTEST_MAIN(TestFindReplaceDialog)
#include "test_findreplacedialog.moc"

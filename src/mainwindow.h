#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
class QPrinter;
class QSettings;
class QTimer;
namespace QtSpell { class TextEditChecker; }
namespace QSourceHighlite { class QSourceHighliter; }
class QActionGroup;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void openFile(const QString &path);

    inline void setLanguage(const QString lang) { language = lang; };

protected:
    void closeEvent(QCloseEvent *e) override;

signals:
    void modificationChanged(bool);

private slots:
    void onFileNew();
    void onFileOpen();
    void onFileSave();
    void onFileSaveAs();
    void onHelpAbout();
    void onTextChanged();
    void changeMode(const QString &);
    void exportHtml(QString file = "");

    void filePrint();
    void filePrintPreview();
    void printPreview(QPrinter *);

    void changeHighlighting(bool enabled);
    void changeSpelling(bool checked);
    void pausePreview(bool checked);
    void disablePreview(bool checked);

    void settingsDialog();

    void undo();
    void redo();

    void autoSave();
    void clearAutoSave();

private:
    bool isModified() const;
    void loadSettings();
    void saveSettings();
    void updateOpened();
    void openRecent();

    Ui::MainWindow *ui;

    QString path;
    int _mode;

    QTimer *timer;

    QSettings *settings;

    QStringList recentOpened;

    QString language;

    QString originalMd = "";
    int originalMdLength = 0;

    bool dontUpdate = false;
    bool setPath = true;
    bool html = false;
    bool spelling = true;
    bool highlighting = true;

    QActionGroup *fileActions;
    QActionGroup *editActions;

    QtSpell::TextEditChecker *checker;
    QSourceHighlite::QSourceHighliter *htmlHighliter;
};
#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "CustomFileSystemModel.h"
#include <QTreeView>
#include <QTextBrowser>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QStackedWidget>
#include <Qsci/qsciscintilla.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public slots:
    void onEditorModified(bool changed);

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;

    CustomFileSystemModel *fileModel;     // Custom model to override icons and enable scoped refresh
    QTreeView *treeView;                  // File/folder tree view
    QWidget *treeContainer;              // Container holding treeView + header
    // QsciScintilla *editor;              // Code editor
    QPushButton *openFolderButton;      // Welcome screen open folder button
    QStackedWidget *stackedWidget;      // For toggling between welcome and editor
    QWidget *editorContainer;           // Main layout holder
    QString currentFilePath;            // Currently opened file path
    QTabWidget *editorTabs;
    QTimer *autosaveTimer;
    QAction *autosaveAction;
    QTextBrowser *consoleOutput;
    QAction *toggleTerminalAction;
    int errorMarker = -1;
    /*QTabWidget *terminalTabs;
    QList<QTextBrowser *> terminalWidgets;

    void addNewTerminalTab(const QString &label);*/
    void autosaveFiles();
    void createNewFile();               // Slot to create a file
    void createNewFolder();            // Slot to create a folder
    void refreshExplorer();            // Slot to refresh the current root index only
    void collapseAllFolders();         // Collapse all in treeView
    void collapseRecursive(const QModelIndex &index);
    QIcon getFileIcon(const QString &fileName);  // 🔽 Add this
    void saveCurrentFile();
    void saveFileAs();
    void updateTabTitle(QsciScintilla *editor);
    void saveAllFiles();
    void runCurrentFile();
    void appendToConsole(const QString &text);
    void runSelectedText();
    void openFileInEditor(const QString &filePath, int lineNumber);
    QsciScintilla* createEditorForContent(const QString &filePath, const QString &content);
};

#endif // MAINWINDOW_H

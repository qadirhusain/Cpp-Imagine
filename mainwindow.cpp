#include "mainwindow.h"
#include "CustomFileSystemModel.h"
#include "./ui_mainwindow.h"

#include <QLineEdit>
#include <QTextBrowser>
#include <QRegularExpression>
#include<QScrollBar>
#include <QProcess>
#include <QToolButton>
#include <QCloseEvent>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QTreeView>
#include <QFileSystemModel>
#include <QPushButton>
#include <QStackedWidget>
#include <QLabel>
#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexercpp.h>

QString openedFolderPath; // Track currently opened folder

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Add this to your constructor (after UI setup):
    autosaveTimer = new QTimer(this);
    autosaveTimer->setInterval(30000); // 30 seconds
    connect(autosaveTimer, &QTimer::timeout, this, &MainWindow::autosaveFiles);
    // autosaveTimer->start();

    errorMarker = -1;

    // === TOP BAR ===
    QWidget *topBar = new QWidget(this);
    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(5, 2, 5, 2);
    topLayout->setSpacing(0);

    QMenuBar *menuBar = new QMenuBar(topBar);
    menuBar->setStyleSheet("QMenuBar { background-color: #2d2d30; color: white; font: 13px 'Segoe UI'; }"
                           "QMenuBar::item:selected { background: #094771; }");
    topLayout->addWidget(menuBar);
    topLayout->addSpacing(10);
    topLayout->addStretch();

    QLineEdit *searchBar = new QLineEdit(topBar);
    searchBar->setPlaceholderText("Search...");
    searchBar->setFixedWidth(200);
    searchBar->setStyleSheet("QLineEdit { background-color: #3c3c3c; color: #d4d4d4; border: 1px solid #555; padding: 4px; border-radius: 4px; font: 13px 'Segoe UI'; }"
                             "QLineEdit:focus { border: 1px solid #007acc; }");
    topLayout->addWidget(searchBar);
    setMenuWidget(topBar);

    QMenu *fileMenu = new QMenu("File", this);
    QMenu *editMenu = new QMenu("Edit", this);
    QMenu *selectionMenu = new QMenu("Selection", this);
    QMenu *viewMenu = new QMenu("View", this);
    QMenu *goMenu = new QMenu("Go", this);
    QMenu *runMenu = new QMenu("Run", this);
    QMenu *terminalMenu = new QMenu("Terminal", this);
    menuBar->addMenu(fileMenu);
    menuBar->addMenu(editMenu);
    menuBar->addMenu(selectionMenu);
    menuBar->addMenu(viewMenu);
    menuBar->addMenu(goMenu);
    menuBar->addMenu(runMenu);
    menuBar->addMenu(terminalMenu);

    QAction *newFileAction = fileMenu->addAction("New File");
    QAction *newWindowAction = fileMenu->addAction("New Window");
    QAction *openFolderAction = fileMenu->addAction("Open Folder");
    QAction *openFileAction = fileMenu->addAction("Open File");
    fileMenu->addSeparator();
    QAction *saveAction = fileMenu->addAction("Save");
    saveAction->setShortcut(QKeySequence::Save); // ⬅️ This line binds Ctrl+S
    QAction *saveAsAction = fileMenu->addAction("Save As...");
    saveAsAction->setShortcut(QKeySequence::SaveAs);  // ⬅️ Ctrl+Shift+S
    QAction *saveAllAction = fileMenu->addAction("Save All");
    saveAllAction->setShortcut(QKeySequence("Ctrl+Shift+S"));  // or "Alt+S"
    connect(saveAllAction, &QAction::triggered, this, &MainWindow::saveAllFiles);
    fileMenu->addSeparator();

    QAction *autosaveAction = new QAction("Enable Auto Save", this);
    autosaveAction->setCheckable(true);             // Make it a checkbox-style menu item
    autosaveAction->setChecked(true);               // Default: autosave enabled
    fileMenu->addAction(autosaveAction);

    fileMenu->addSeparator();
    fileMenu->addAction("Exit", qApp, SLOT(quit()));

    // Connect toggle
    connect(autosaveAction, &QAction::toggled, this, [=](bool enabled) {
        if (enabled) autosaveTimer->start();
        else autosaveTimer->stop();
    });

    // ✅ Explicitly start the timer if autosave is initially checked
    if (autosaveAction->isChecked()) {
        autosaveTimer->start();
    }

    editMenu->addAction("Undo");
    editMenu->addAction("Redo");
    editMenu->addSeparator();
    editMenu->addAction("Cut");
    editMenu->addAction("Copy");
    editMenu->addAction("Paste");
    editMenu->addSeparator();
    editMenu->addAction("Find");
    editMenu->addAction("Replace");

    selectionMenu->addAction("Select All");
    selectionMenu->addAction("Expand Selection");
    selectionMenu->addSeparator();
    selectionMenu->addAction("Copy Line Up");
    selectionMenu->addAction("Copy Line Down");
    selectionMenu->addAction("Move Line Up");
    selectionMenu->addAction("Move Line Down");
    selectionMenu->addAction("Duplicate Selection");

    viewMenu->addAction("Toggle Word Wrap");
    viewMenu->addSeparator();
    viewMenu->addAction("Full Window");
    viewMenu->addAction("Menu Bar Visibility");
    viewMenu->addAction("Primary Side Bar Visibility");
    viewMenu->addSeparator();
    viewMenu->addAction("Centered Layout");
    viewMenu->addSeparator();
    viewMenu->addAction("Zoom In");
    viewMenu->addAction("Zoom Out");
    viewMenu->addSeparator();
    viewMenu->addAction("Split Up");
    viewMenu->addAction("Split Down");
    viewMenu->addAction("Split Left");
    viewMenu->addAction("Split Right");

    goMenu->addAction("Back");
    goMenu->addAction("Forward");
    goMenu->addAction("Last Edit Location");
    goMenu->addSeparator();
    goMenu->addAction("Go to File");
    goMenu->addAction("Go to Definition");
    goMenu->addAction("Go to Declaration");
    goMenu->addSeparator();
    goMenu->addAction("Go to Line");

    runMenu->addAction("Start Debugging");
    runMenu->addAction("Run Without Debugging");
    QAction *runFileAction = new QAction("Run Current File", this);
    runMenu->addAction(runFileAction);
    connect(runFileAction, &QAction::triggered, this, &MainWindow::runCurrentFile);
    runMenu->addSeparator();
    runMenu->addAction("Enable all BreakPoints");
    runMenu->addAction("Disable all BreakPoints");
    runMenu->addAction("Remove all BreakPoints");

    terminalMenu->addAction("New Terminal");
    terminalMenu->addSeparator();
    QAction *runSelectedTextAction = terminalMenu->addAction("Run Selected Text");
    connect(runSelectedTextAction, &QAction::triggered, this, &MainWindow::runSelectedText);
    terminalMenu->addAction("Run task");
    terminalMenu->addAction("Run Active File");

    // === CONSOLE OUTPUT PANE ===
    consoleOutput = new QTextBrowser(this);
    consoleOutput->setReadOnly(true);
    consoleOutput->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

    // consoleOutput->setReadOnly(true);
    consoleOutput->setFixedHeight(150);
    consoleOutput->setStyleSheet("background-color: #1e1e1e; font: 12px 'Consolas'; border-top: 1px solid #444;");

    // ✅ Add these two lines just below the style settings:
    consoleOutput->setOpenLinks(false);  // This ensures anchorClicked signal is emitted
    consoleOutput->setTextInteractionFlags(Qt::TextBrowserInteraction);

    QPushButton *clearConsoleBtn = new QPushButton("🧹 Clear");
    clearConsoleBtn->setFixedSize(80, 26);
    clearConsoleBtn->setStyleSheet(
        "QPushButton { background-color: #3c3c3c; color: white; border: 1px solid #555; border-radius: 4px; font: 12px 'Segoe UI'; }"
        "QPushButton:hover { background-color: #444444; }"
        );

    toggleTerminalAction = new QAction("Toggle Terminal", this);
    toggleTerminalAction->setCheckable(true);
    toggleTerminalAction->setChecked(true);
    terminalMenu->addAction(toggleTerminalAction);

    terminalMenu->addAction("Run Selected Text");

    connect(toggleTerminalAction, &QAction::toggled, this, [=](bool visible) {
        consoleOutput->setVisible(visible);
    });

    static bool isConsoleConnected = false;
    if (!isConsoleConnected) {
        connect(consoleOutput, &QTextBrowser::anchorClicked, this, [=](const QUrl &url) {
            QString fullLink = url.toString(QUrl::None);  // e.g. file:///E:/.../main.cpp:10

            QRegularExpression rx(R"(file:///(.+):(\d+))");
            QRegularExpressionMatch match = rx.match(fullLink);

            if (match.hasMatch()) {
                QString encodedFilePath = match.captured(1).trimmed();
                QString filePath = QUrl::fromPercentEncoding(encodedFilePath.toUtf8());  // ✅ Decode

                int lineNumber = match.captured(2).toInt();

                qDebug() << "Opening:" << filePath << "at line:" << lineNumber;

                QFileInfo info(filePath);
                if (info.exists() && info.isFile()) {
                    openFileInEditor(filePath, lineNumber - 1); // 0-indexed
                } else {
                    QMessageBox::warning(this, "File Not Found", filePath);
                }
            } else {
                QMessageBox::warning(this, "Invalid Link", fullLink);
            }
        });

        isConsoleConnected = true;
    }

    consoleOutput->setVisible(false);
    toggleTerminalAction->setChecked(false);

    // === WELCOME SCREEN ===
    QWidget *welcomeScreen = new QWidget(this);
    welcomeScreen->setStyleSheet("background-color: #1e1e1e;");
    QVBoxLayout *welcomeLayout = new QVBoxLayout(welcomeScreen);
    welcomeLayout->setAlignment(Qt::AlignCenter);
    QLabel *msg = new QLabel("You have not yet opened a folder.", this);
    msg->setStyleSheet("color: #c5c5c5; font-size: 14px;");
    msg->setAlignment(Qt::AlignCenter);
    openFolderButton = new QPushButton("Open Folder", this);
    openFolderButton->setFixedSize(200, 40);
    openFolderButton->setStyleSheet("QPushButton { background-color: #0e639c; color: white; border: none; font-size: 14px; border-radius: 3px; } QPushButton:hover { background-color: #1177bb; }");
    welcomeLayout->addWidget(msg);
    welcomeLayout->addSpacing(15);
    welcomeLayout->addWidget(openFolderButton);

    // === MAIN EDITOR CONTAINER ===
    editorContainer = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(editorContainer);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // === LEFT SIDEBAR ===
    QWidget *sideBar = new QWidget();
    sideBar->setFixedWidth(50);
    sideBar->setMinimumWidth(50);
    sideBar->setStyleSheet("background-color: #333333;");
    QVBoxLayout *sideBarLayout = new QVBoxLayout(sideBar);
    sideBarLayout->setContentsMargins(10, 8, 10, 0);
    sideBarLayout->setSpacing(10);

    QPushButton *explorerBtn = new QPushButton(QIcon(":/icons/icons/file explorer.png"), "", sideBar);
    explorerBtn->setFixedSize(40, 40);
    explorerBtn->setIconSize(QSize(40, 40));
    explorerBtn->setStyleSheet("QPushButton { border: none; background-color: transparent; }"
                               "QPushButton:hover { background-color: #444444; }");

    sideBarLayout->addWidget(explorerBtn);
    sideBarLayout->addStretch();

    mainLayout->addWidget(sideBar, 0);  // Sidebar, no stretch

    // === FILE EXPLORER TREE VIEW ===
    fileModel = new CustomFileSystemModel(this);
    fileModel->setRootPath(QDir::rootPath());
    fileModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);

    treeView = new QTreeView(this);
    treeView->setModel(fileModel);
    treeView->hideColumn(1);
    treeView->hideColumn(2);
    treeView->hideColumn(3);
    treeView->setHeaderHidden(true);
    treeView->setStyleSheet(
        "QTreeView { background-color: #252526; color: #d4d4d4; font: 13px 'Segoe UI'; }"
        "QTreeView::item:selected { background-color: #094771; }"
        "QTreeView::branch:selected { background-color: #094771; }"
        );
    treeView->setRootIsDecorated(true);   // Enables expand/collapse arrows
    treeView->setAnimated(true);
    treeView->setSortingEnabled(true);                     // ENABLE SORTING
    treeView->sortByColumn(0, Qt::AscendingOrder);

    QWidget *treeContainer = new QWidget(this);
    this->treeContainer = treeContainer; // Add this line
    treeContainer->setMinimumWidth(200);
    treeContainer->setMaximumWidth(300);

    QVBoxLayout *treeLayout = new QVBoxLayout(treeContainer);
    treeLayout->setContentsMargins(0, 0, 0, 0);

    // === FOLDER HEADER + TOOLBAR ===
    QWidget *folderHeader = new QWidget(treeContainer);
    QHBoxLayout *folderLayout = new QHBoxLayout(folderHeader);
    folderLayout->setContentsMargins(5, 5, 5, 5);
    folderLayout->setSpacing(10);

    QLabel *folderNameLabel = new QLabel("Opened Folder", folderHeader);
    folderNameLabel->setStyleSheet("color: #c5c5c5; font: 13px 'Segoe UI';");
    folderLayout->addWidget(folderNameLabel);
    folderLayout->addStretch();

    QPushButton *newFileBtn = new QPushButton(QIcon(":/icons/icons/add-file-white.png"), "", folderHeader);
    QPushButton *newFolderBtn = new QPushButton(QIcon(":/icons/icons/output-onlinepngtools.png"), "", folderHeader);
    QPushButton *refreshBtn = new QPushButton(QIcon(":/icons/icons/refresh-white.png"), "", folderHeader);
    QPushButton *collapseBtn = new QPushButton(QIcon(":/icons/icons/collapse-white.png"), "", folderHeader);

    QList<QPushButton*> buttons = { newFileBtn, newFolderBtn, refreshBtn, collapseBtn };
    for (auto btn : buttons) {
        btn->setFixedSize(24, 24);
        btn->setIconSize(QSize(16, 16));
        btn->setStyleSheet("QPushButton { border: none; background-color: transparent; }"
                           "QPushButton:hover { background-color: #444444; }");
        folderLayout->addWidget(btn);
    }

    treeLayout->addWidget(folderHeader);  // 👈 Add toolbar before treeView
    treeLayout->addWidget(treeView);

    connect(newFileBtn, &QPushButton::clicked, this, &MainWindow::createNewFile);
    connect(newFolderBtn, &QPushButton::clicked, this, &MainWindow::createNewFolder);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::refreshExplorer);
    connect(collapseBtn, &QPushButton::clicked, this, &MainWindow::collapseAllFolders);

    mainLayout->addWidget(treeContainer);

    // === QSCI TAB EDITOR ===
    editorTabs = new QTabWidget(this);
    editorTabs->setTabsClosable(true);
    editorTabs->setMovable(true);
    editorTabs->setDocumentMode(true);
    editorTabs->setStyleSheet(
        "QTabWidget::pane { border: none; }"
        "QTabBar::tab { background: #2d2d30; color: #d4d4d4; padding: 5px 10px; }"
        "QTabBar::tab:selected { background: #1e1e1e; }"
        );
    mainLayout->addWidget(editorTabs, 1);

    QVBoxLayout *editorAndConsoleLayout = new QVBoxLayout();
    editorAndConsoleLayout->setContentsMargins(0, 0, 0, 0);
    editorAndConsoleLayout->setSpacing(0);

    QHBoxLayout *consoleTopLayout = new QHBoxLayout();
    consoleTopLayout->addStretch();              // Pushes the button to the right
    consoleTopLayout->addWidget(clearConsoleBtn);
    consoleTopLayout->setContentsMargins(5, 5, 5, 5);

    editorAndConsoleLayout->addWidget(editorTabs, 1);  // 👈 Editor takes all space
    editorAndConsoleLayout->addLayout(consoleTopLayout); // 👈 Add this
    editorAndConsoleLayout->addWidget(consoleOutput);

    connect(clearConsoleBtn, &QPushButton::clicked, this, [=]() {
        consoleOutput->clear();
    });

    QWidget *centerEditorWidget = new QWidget(this);
    centerEditorWidget->setLayout(editorAndConsoleLayout);
    mainLayout->addWidget(centerEditorWidget, 1);      // replaces addWidget(editorTabs, 1);

    connect(editorTabs, &QTabWidget::tabCloseRequested, this, [=](int index) {
        QWidget *widget = editorTabs->widget(index);
        editorTabs->removeTab(index);
        delete widget;
    });

    // === STACKED WIDGET ===
    stackedWidget = new QStackedWidget(this);
    stackedWidget->addWidget(welcomeScreen);
    stackedWidget->addWidget(editorContainer);
    stackedWidget->setCurrentIndex(0);

    // setCentralWidget(stackedWidget);

    QLabel *footerLabel = new QLabel("© Copyright 2025. C++ Imagine -- Made by Qadir Husain", this);
    footerLabel->setAlignment(Qt::AlignCenter);
    footerLabel->setStyleSheet("color: gray; font: 12px 'Segoe UI'; padding: 5px;");

    // Create a container widget at the bottom
    QWidget *footerContainer = new QWidget(this);
    QHBoxLayout *footerLayout = new QHBoxLayout(footerContainer);
    footerLayout->addStretch();
    footerLayout->addWidget(footerLabel);
    footerLayout->addStretch();
    footerLayout->setContentsMargins(0, 0, 0, 0);

    // Add the footer to the main layout
    QVBoxLayout *centralLayout = new QVBoxLayout();
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);
    centralLayout->addWidget(stackedWidget);     // already your main content
    centralLayout->addWidget(footerContainer);   // ⬅️ add footer at bottom

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(centralLayout);
    setCentralWidget(centralWidget);

    // === OPEN FOLDER HANDLING ===
    auto openFolderLambda = [=]() {
        QString folder = QFileDialog::getExistingDirectory(this, "Select Folder");
        if (!folder.isEmpty()) {
            openedFolderPath = folder;
            fileModel->setRootPath(folder);
            treeView->setRootIndex(fileModel->index(folder));
            stackedWidget->setCurrentIndex(1);
            folderNameLabel->setText(QFileInfo(folder).fileName()); // 👈 Update folder label

            // Set current index to root folder
            treeView->setCurrentIndex(fileModel->index(folder));
        }
    };

    connect(openFolderButton, &QPushButton::clicked, this, openFolderLambda);
    connect(openFolderAction, &QAction::triggered, this, openFolderLambda);

    connect(treeView, &QTreeView::clicked, this, [=](const QModelIndex &index) {
        QString path = fileModel->filePath(index);
        QFileInfo info(path);

        if (info.isFile()) {
            QString ext = info.suffix().toLower();
            if (ext != "cpp") {
                QMessageBox::warning(this, "Unsupported File", "Only .cpp files are supported in this IDE.");
                return;
            }

            for (int i = 0; i < editorTabs->count(); ++i) {
                if (editorTabs->tabToolTip(i) == path) {
                    editorTabs->setCurrentIndex(i);
                    return;
                }
            }

            QFile file(path);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {

                QString fileContent = file.readAll();
                file.close();

                QsciScintilla *newEditor = new QsciScintilla(this);
                QsciLexerCPP *lexer = new QsciLexerCPP(newEditor);

                QFont font("Consolas", 11);
                newEditor->setFont(font);
                lexer->setFont(font);

                QColor bg("#1e1e1e");
                QColor fg("#d4d4d4");

                newEditor->setPaper(bg);
                newEditor->setColor(fg);
                lexer->setDefaultPaper(bg);
                lexer->setDefaultColor(fg);

                for (int style = 0; style <= 127; ++style) {
                    lexer->setColor(fg, style);
                    lexer->setPaper(bg, style);
                }

                // Custom syntax colors
                lexer->setColor(QColor("#569CD6"), QsciLexerCPP::Keyword);
                lexer->setColor(QColor("#CE9178"), QsciLexerCPP::DoubleQuotedString);
                lexer->setColor(QColor("#D16969"), QsciLexerCPP::SingleQuotedString);
                lexer->setColor(QColor("#B5CEA8"), QsciLexerCPP::Comment);
                lexer->setColor(QColor("#9CDCFE"), QsciLexerCPP::Number);
                lexer->setColor(QColor("#C586C0"), QsciLexerCPP::PreProcessor);

                newEditor->setLexer(lexer);
                newEditor->setAutoIndent(true);
                newEditor->setMarginLineNumbers(1, true);
                newEditor->setBraceMatching(QsciScintilla::StrictBraceMatch);
                newEditor->setCaretLineVisible(true);
                newEditor->setCaretLineBackgroundColor(QColor("#2a2d2e"));
                newEditor->setReadOnly(false);

                QString cleanName = info.fileName();
                newEditor->setProperty("cleanName", cleanName);

                // Add tab with clean name
                /*editorTabs->addTab(newEditor, getFileIcon(cleanName), cleanName);
                editorTabs->setTabToolTip(editorTabs->count() - 1, path);
                editorTabs->setCurrentWidget(newEditor);*/

                int index = editorTabs->addTab(newEditor, getFileIcon(cleanName), cleanName);
                editorTabs->setTabToolTip(index, path);
                editorTabs->setCurrentIndex(index);

                // Custom close (X) button
                QToolButton *closeButton = new QToolButton();
                closeButton->setIcon(QIcon(":/icons/icons/close.png")); // Update with your path
                closeButton->setIconSize(QSize(6, 6));                         // ⬅️ Icon is exactly 8x8 px
                closeButton->setFixedSize(12, 12);
                closeButton->setStyleSheet("QToolButton { border: none; padding: 0px; }"
                                           "QToolButton:hover { background-color: #ff5c5c; border-radius: 2px; }");
                closeButton->setCursor(Qt::PointingHandCursor);

                // Handle click
                connect(closeButton, &QToolButton::clicked, this, [=]() {
                    QWidget *widget = editorTabs->widget(index);
                    editorTabs->removeTab(index);
                    delete widget;
                });

                // Apply close button to tab
                editorTabs->tabBar()->setTabButton(index, QTabBar::RightSide, closeButton);

                // Brace matching highlight colors
                newEditor->setMatchedBraceBackgroundColor(QColor("#007acc"));
                newEditor->setMatchedBraceForegroundColor(Qt::white);
                newEditor->setUnmatchedBraceBackgroundColor(QColor("#f44747"));
                newEditor->setUnmatchedBraceForegroundColor(Qt::white);

                connect(newEditor, SIGNAL(modificationChanged(bool)), this, SLOT(onEditorModified(bool)));

                newEditor->setText(fileContent);

                newEditor->setModified(false);

                connect(newEditor, &QsciScintilla::modificationChanged, this, [=](bool changed) {
                    qDebug() << "modificationChanged signal:" << changed;
                    updateTabTitle(newEditor);
                });
            }
        }
    });

    connect(explorerBtn, &QPushButton::clicked, this, [=]() {
        treeContainer->setVisible(!treeContainer->isVisible());
    });

    connect(saveAction, &QAction::triggered, this, &MainWindow::saveCurrentFile);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveFileAs);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::createNewFile() {
    QModelIndex currentIndex = treeView->currentIndex();
    QString path;

    if (currentIndex.isValid()) {
        path = fileModel->filePath(currentIndex);
        if (!QFileInfo(path).isDir()) {
            path = QFileInfo(path).absolutePath();
        }
    } else {
        // Use root path if nothing is selected
        path = openedFolderPath;
    }

    if (path.isEmpty()) return;

    QString fileName = QInputDialog::getText(this, "New File", "Enter file name:");
    if (!fileName.isEmpty()) {
        QFile file(path + "/" + fileName);
        if (file.open(QIODevice::WriteOnly)) {
            file.close();
            refreshExplorer();
        }
    }
}

void MainWindow::createNewFolder() {
    QModelIndex currentIndex = treeView->currentIndex();
    QString path;

    if (currentIndex.isValid()) {
        path = fileModel->filePath(currentIndex);
        if (!QFileInfo(path).isDir()) {
            path = QFileInfo(path).absolutePath();
        }
    } else {
        // Use root path if nothing is selected
        path = openedFolderPath;
    }

    if (path.isEmpty()) return;

    QString folderName = QInputDialog::getText(this, "New Folder", "Enter folder name:");
    if (!folderName.isEmpty()) {
        QDir dir(path);
        if (dir.mkdir(folderName)) {
            refreshExplorer();
        }
    }
}

void MainWindow::refreshExplorer() {
    if (!openedFolderPath.isEmpty()) {
        QModelIndex rootIndex = treeView->rootIndex();
        fileModel->refresh(rootIndex);
    }
}

void MainWindow::collapseAllFolders() {
    if (!treeView) return;

    QModelIndex rootIndex = treeView->rootIndex();
    if (!rootIndex.isValid()) return;

    // Recursively collapse all
    collapseRecursive(rootIndex);
}

void MainWindow::collapseRecursive(const QModelIndex &index) {
    if (!index.isValid()) return;

    treeView->collapse(index);

    int rowCount = fileModel->rowCount(index);
    for (int i = 0; i < rowCount; ++i) {
        QModelIndex childIndex = fileModel->index(i, 0, index);
        collapseRecursive(childIndex);
    }
}

QIcon MainWindow::getFileIcon(const QString &fileName) {
    if (fileName.endsWith(".cpp")) {
        return QIcon(":/icons/icons/vscode_icon_white.png");
    }

    return QIcon(":/icons/icons/default_icon.png");
}

void MainWindow::saveCurrentFile() {
    QWidget *widget = editorTabs->currentWidget();
    if (!widget) return;

    QsciScintilla *editor = qobject_cast<QsciScintilla *>(widget);
    if (!editor) return;

    QString filePath = editorTabs->tabToolTip(editorTabs->currentIndex());

    if (filePath.isEmpty()) {
        saveFileAs();
        return;
    }

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << editor->text();
        file.close();
        editor->setModified(false);
    }
}

void MainWindow::saveFileAs() {
    QWidget *widget = editorTabs->currentWidget();
    if (!widget) return;

    QsciScintilla *editor = qobject_cast<QsciScintilla *>(widget);
    if (!editor) return;

    QString filePath = QFileDialog::getSaveFileName(this, "Save File As");
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << editor->text();
        file.close();

        editorTabs->setTabToolTip(editorTabs->currentIndex(), filePath);
        QString cleanName = QFileInfo(filePath).fileName();
        editor->setProperty("cleanName", cleanName);
        editor->setModified(false);
    }
}

void MainWindow::updateTabTitle(QsciScintilla *editor) {
    int tabIndex = editorTabs->indexOf(editor);
    if (tabIndex == -1) return;

    QString fileName = QFileInfo(editorTabs->tabToolTip(tabIndex)).fileName();
    bool modified = editor->isModified();

    editorTabs->setTabText(tabIndex, modified ? fileName + "*" : fileName);
}

void MainWindow::onEditorModified(bool changed)
{
    Q_UNUSED(changed);  // Optional: suppress unused warning

    QsciScintilla *editor = qobject_cast<QsciScintilla *>(sender());
    if (editor) {
        updateTabTitle(editor);
    }
}

void MainWindow::saveAllFiles() {
    for (int i = 0; i < editorTabs->count(); ++i) {
        QWidget *widget = editorTabs->widget(i);
        QsciScintilla *editor = qobject_cast<QsciScintilla *>(widget);
        if (editor && editor->isModified()) {
            QString filePath = editorTabs->tabToolTip(i);
            if (!filePath.isEmpty()) {
                QFile file(filePath);
                if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream out(&file);
                    out << editor->text();
                    file.close();
                    editor->setModified(false);
                    updateTabTitle(editor);
                }
            }
        }
    }
}

void MainWindow::autosaveFiles() {
    for (int i = 0; i < editorTabs->count(); ++i) {
        QWidget *widget = editorTabs->widget(i);
        QsciScintilla *editor = qobject_cast<QsciScintilla *>(widget);
        if (editor && editor->isModified()) {
            QString filePath = editorTabs->tabToolTip(i);
            if (!filePath.isEmpty()) {
                QFile file(filePath);
                if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream out(&file);
                    out << editor->text();
                    file.close();
                    editor->setModified(false);
                    updateTabTitle(editor); // Update asterisk
                }
            }
        }
    }

    statusBar()->showMessage("Autosaved...", 2000); // Show for 2 sec
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    bool hasUnsavedChanges = false;

    for (int i = 0; i < editorTabs->count(); ++i) {
        QsciScintilla *editor = qobject_cast<QsciScintilla *>(editorTabs->widget(i));
        if (editor && editor->isModified()) {
            hasUnsavedChanges = true;
            break;
        }
    }

    // Only warn if autosave is disabled and unsaved changes exist
    if (autosaveAction && !autosaveAction->isChecked() && hasUnsavedChanges) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Unsaved Changes",
                                                                  "Some files have unsaved changes.\nAre you sure you want to exit?",
                                                                  QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::No) {
            event->ignore();
            return;
        }
    }

    event->accept(); // allow window to close
}

void MainWindow::runCurrentFile() {
    QWidget *currentWidget = editorTabs->currentWidget();
    if (!currentWidget) return;

    QsciScintilla *editor = qobject_cast<QsciScintilla *>(currentWidget);
    if (!editor) return;

    QString filePath = editorTabs->tabToolTip(editorTabs->currentIndex());
    if (!filePath.endsWith(".cpp")) {
        appendToConsole("[ERROR] Only .cpp files can be run.\n");
        return;
    }

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << editor->text();
        file.close();
    }

    QString outputBinary = QDir::tempPath() + "/output_exe";
#ifdef Q_OS_WIN
    outputBinary += ".exe";
#endif

    consoleOutput->clear();
    appendToConsole("[INFO] Compiling " + filePath);

    // Ensure console is visible when running
    if (!consoleOutput->isVisible())
        consoleOutput->setVisible(true);

    QProcess *compiler = new QProcess(this);
    compiler->start("g++", QStringList() << filePath << "-o" << outputBinary);

    connect(compiler, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int exitCode, QProcess::ExitStatus status) {
                if (status == QProcess::NormalExit && exitCode == 0) {
                    appendToConsole("[INFO] Compilation successful. Running program...\n");

                    QProcess *runner = new QProcess(this);
                    runner->start(outputBinary);

                    connect(runner, &QProcess::readyReadStandardOutput, [=]() {
                        appendToConsole(runner->readAllStandardOutput());
                    });
                    connect(runner, &QProcess::readyReadStandardError, [=]() {
                        appendToConsole(runner->readAllStandardError());
                    });
                    connect(runner, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=]() {
                        appendToConsole("\n[INFO] Program finished.");
                        runner->deleteLater();
                    });

                } else {
                    appendToConsole("[ERROR] Compilation failed.");
                }
                compiler->deleteLater();
            });

    connect(compiler, &QProcess::readyReadStandardError, [=]() {
        appendToConsole(compiler->readAllStandardError());
    });
}

void MainWindow::appendToConsole(const QString &text) {
    QTextCursor cursor = consoleOutput->textCursor();

    // Determine color based on content
    QString color = "#d4d4d4"; // Default white
    if (text.contains("[ERROR]", Qt::CaseInsensitive) || text.contains("error:", Qt::CaseInsensitive)) {
        color = "#f44747"; // Red
    } else if (text.contains("[WARNING]", Qt::CaseInsensitive) || text.contains("warning:", Qt::CaseInsensitive)) {
        color = "#ffcc00"; // Yellow
    } else if (text.contains("[INFO]", Qt::CaseInsensitive)) {
        color = "#9CDCFE"; // Blue
    }

    QString formattedText = text.toHtmlEscaped();  // Safe for HTML

    // Match multiple patterns like path/to/file.cpp:line:column
    QRegularExpression errorPattern(R"(([\w\:/\\\s\.\-]+\.cpp):(\d+):(\d+):)");
    QRegularExpressionMatchIterator it = errorPattern.globalMatch(text);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString rawFile = match.captured(1).trimmed();
        QString line = match.captured(2).trimmed();
        QString column = match.captured(3).trimmed();

        QString fullOriginal = QString("%1:%2:%3").arg(rawFile, line, column);

        // ✅ Resolve path: check if rawFile is already absolute
        QFileInfo fileInfo(rawFile);
        QString absolutePath;

        if (fileInfo.isAbsolute() && fileInfo.exists()) {
            absolutePath = fileInfo.absoluteFilePath();
        } else {
            QString cleanedRaw = rawFile;
            if (rawFile.startsWith("Files/")) {
                cleanedRaw = rawFile.mid(QString("Files/").length());
            }

            QString candidate = QDir(openedFolderPath).filePath(cleanedRaw);
            QFileInfo candidateInfo(candidate);
            if (candidateInfo.exists()) {
                absolutePath = candidateInfo.absoluteFilePath();
            } else {
                absolutePath = candidateInfo.filePath(); // fallback
            }
        }

        qDebug() << "Raw:" << rawFile;
        qDebug() << "Resolved path:" << absolutePath;

        // 🧩 Create clickable link
        QString hyperlink = QString(R"(<a href="file:///%1:%2">%3:%4</a>)")
                                .arg(absolutePath, line, QFileInfo(rawFile).fileName(), line);

        formattedText.replace(fullOriginal.toHtmlEscaped(), hyperlink);
    }

    // Final HTML line with color
    QString htmlLine = QString(R"(<pre style="font-family: Consolas; color: %1;">%2</pre>)")
                           .arg(color, formattedText);

    cursor.movePosition(QTextCursor::End);
    cursor.insertHtml(htmlLine);
    cursor.insertBlock();

    consoleOutput->setTextCursor(cursor);
    consoleOutput->ensureCursorVisible();
}

void MainWindow::openFileInEditor(const QString &filePath, int lineNumber) {
    QString normalizedTargetPath = QFileInfo(filePath).absoluteFilePath();

    for (int i = 0; i < editorTabs->count(); ++i) {
        QString tabPath = QFileInfo(editorTabs->tabToolTip(i)).absoluteFilePath();

        if (tabPath == normalizedTargetPath) {
            editorTabs->setCurrentIndex(i);
            QsciScintilla *editor = qobject_cast<QsciScintilla *>(editorTabs->widget(i));
            if (editor) {
                if (errorMarker == -1) {
                    errorMarker = editor->markerDefine(QsciScintilla::Background);
                    editor->setMarkerBackgroundColor(QColor("#FF5555"), errorMarker);
                }
                editor->markerDeleteAll(errorMarker); // clear previous
                editor->markerAdd(lineNumber, errorMarker); // highlight current
                editor->setCursorPosition(lineNumber, 0);
                editor->setFocus();
            }
            return;
        }
    }

    // If not open, open the file
    QFile file(normalizedTargetPath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString content = file.readAll();
        file.close();

        QsciScintilla *newEditor = createEditorForContent(normalizedTargetPath, content); // Your editor creation logic
        int index = editorTabs->addTab(newEditor, getFileIcon(normalizedTargetPath), QFileInfo(filePath).fileName());
        editorTabs->setTabToolTip(index, normalizedTargetPath);  // Store absolute path for consistency
        editorTabs->setCurrentIndex(index);
        newEditor->setCursorPosition(lineNumber, 0);
        newEditor->setFocus();
    }
}

QsciScintilla* MainWindow::createEditorForContent(const QString &filePath, const QString &content) {
    QsciScintilla *editor = new QsciScintilla(this);
    editor->setText(content);

    // Optional: Apply your usual editor settings here (syntax highlighting, font, etc.)
    editor->setUtf8(true);
    editor->setMarginLineNumbers(1, true);
    editor->setMarginWidth(1, "0000");

    // 🔴 Define error marker once
    if (errorMarker == -1) {
        errorMarker = editor->markerDefine(QsciScintilla::Background);
        editor->setMarkerBackgroundColor(QColor("#FF5555"), errorMarker);
    }

    return editor;
}


void MainWindow::runSelectedText() {

    QWidget *currentWidget = editorTabs->currentWidget();
    if (!currentWidget) return;

    QsciScintilla *editor = qobject_cast<QsciScintilla *>(currentWidget);
    if (!editor) return;

    QString selectedCode = editor->selectedText();
    if (selectedCode.trimmed().isEmpty()) {
        if (!consoleOutput->isVisible())
            consoleOutput->setVisible(true);
        appendToConsole("[WARNING] No text selected.");
        return;
    }

    QString wrappedCode = R"(#include<iostream>
using namespace std;

int main() {
)" + selectedCode + R"(
return 0;
}
)";

    QString tempFilePath = QDir::tempPath() + "/selected_snippet.cpp";
    QFile tempFile(tempFilePath);
    if (tempFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&tempFile);
        out << wrappedCode;
        tempFile.close();
    }

    QString outputBinary = QDir::tempPath() + "/selected_output";
#ifdef Q_OS_WIN
    outputBinary += ".exe";
#endif

    if (!consoleOutput->isVisible())
        consoleOutput->setVisible(true);

    appendToConsole("[INFO] Compiling selected snippet...");

    QProcess *compiler = new QProcess(this);
    compiler->start("g++", QStringList() << tempFilePath << "-o" << outputBinary);

    connect(compiler, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int exitCode, QProcess::ExitStatus status) {
                if (status == QProcess::NormalExit && exitCode == 0) {
                    appendToConsole("[INFO] Compilation successful. Running...\n");

                    QProcess *runner = new QProcess(this);
                    runner->start(outputBinary);

                    connect(runner, &QProcess::readyReadStandardOutput, [=]() {
                        appendToConsole(runner->readAllStandardOutput());
                    });
                    connect(runner, &QProcess::readyReadStandardError, [=]() {
                        appendToConsole(runner->readAllStandardError());
                    });
                    connect(runner, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=]() {
                        appendToConsole("\n[INFO] Snippet finished.");
                        runner->deleteLater();
                    });
                } else {
                    appendToConsole("[ERROR] Compilation failed.");
                }
                compiler->deleteLater();
            });

    connect(compiler, &QProcess::readyReadStandardError, [=]() {
        appendToConsole(compiler->readAllStandardError());
    });
}

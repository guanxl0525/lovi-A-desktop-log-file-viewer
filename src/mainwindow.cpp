#include "mainwindow.h"

#include "config.h"
#include "filelineprovider.h"
#include "logformat.h"
#include "logformatdialog.h"
#include "logformatloader.h"
#include "logmodel.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QTimer>
#include <QToolBar>
#include <QTreeView>

static const int MAX_RECENT_FILES = 10;

MainWindow::MainWindow(Config* config, QWidget* parent)
        : QMainWindow(parent)
        , mConfig(config)
        , mLogFormatLoader(std::make_unique<LogFormatLoader>())
        , mOpenLogAction(new QAction(this))
        , mSelectLogFormatAction(new QAction(this))
        , mAutoScrollAction(new QAction(this))
        , mCopyLinesAction(new QAction(this))
        , mToolBar(addToolBar(tr("Toolbar")))
        , mTreeView(new QTreeView(this)) {
    setupUi();
    setupActions();
}

MainWindow::~MainWindow() {
}

void MainWindow::loadLogFormat(const QString& filePath) {
    mLogFormatPath = filePath;
    mLogFormatLoader->load(filePath);
}

void MainWindow::loadLog(const QString& filePath) {
    mLogPath = filePath;
    setWindowTitle(QString("%1 - Lovi").arg(mLogPath));
    addLogToRecentFiles();

    auto fileLineProvider = std::make_unique<FileLineProvider>();
    fileLineProvider->setFilePath(filePath);
    mLineProvider = std::move(fileLineProvider);

    mLogModel = std::make_unique<LogModel>(mLineProvider.get());
    mLogModel->setLogFormat(mLogFormatLoader->logFormat());
    connect(mLogModel.get(), &QAbstractItemModel::rowsInserted, this, &MainWindow::onRowsInserted);

    connect(mLogFormatLoader.get(),
            &LogFormatLoader::logFormatChanged,
            mLogModel.get(),
            &LogModel::setLogFormat);

    mTreeView->setModel(mLogModel.get());

    // Must be done here because the selectionModel is (re)set by setModel()
    connect(mTreeView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &MainWindow::onSelectionChanged);
    onSelectionChanged();
}

void MainWindow::setupUi() {
    setWindowTitle("Lovi");

    mTreeView->setRootIsDecorated(false);
    mTreeView->setContextMenuPolicy(Qt::ActionsContextMenu);
    mTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mTreeView->addAction(mCopyLinesAction);
    setCentralWidget(mTreeView);

    mToolBar->addAction(mOpenLogAction);
    mToolBar->addAction(mSelectLogFormatAction);
    mToolBar->addAction(mAutoScrollAction);
    mToolBar->setMovable(false);
    mToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    resize(800, 600);
}

static void appendShortcutToToolTip(QAction* action) {
    auto shortcut = action->shortcut().toString(QKeySequence::NativeText);
    action->setToolTip(QString("%1 (%2)").arg(action->toolTip(), shortcut));
}

void MainWindow::setupActions() {
    mOpenLogAction->setText(tr("Open"));
    mOpenLogAction->setToolTip(tr("Open log file"));
    mOpenLogAction->setIcon(QIcon::fromTheme("document-open"));
    mOpenLogAction->setShortcut(QKeySequence::Open);
    connect(mOpenLogAction, &QAction::triggered, this, &MainWindow::showOpenLogDialog);

    mSelectLogFormatAction->setText(tr("Format"));
    mSelectLogFormatAction->setToolTip(tr("Select log format"));
    mSelectLogFormatAction->setShortcut(Qt::SHIFT | Qt::Key_F);
    mSelectLogFormatAction->setIcon(QIcon::fromTheme("object-columns"));
    connect(mSelectLogFormatAction, &QAction::triggered, this, &MainWindow::showLogFormatDialog);

    mAutoScrollAction->setText(tr("Auto scroll"));
    mAutoScrollAction->setShortcut(Qt::SHIFT | Qt::Key_S);
    mAutoScrollAction->setToolTip(tr("Automatically scroll down when new lines are logged"));
    mAutoScrollAction->setIcon(QIcon::fromTheme("go-bottom"));
    mAutoScrollAction->setCheckable(true);
    connect(mAutoScrollAction, &QAction::toggled, this, [this](bool toggled) {
        if (toggled) {
            mTreeView->scrollToBottom();
        }
    });

    for (auto action : {mOpenLogAction, mSelectLogFormatAction, mAutoScrollAction}) {
        appendShortcutToToolTip(action);
    }

    mCopyLinesAction->setText(tr("Copy"));
    mCopyLinesAction->setShortcut(QKeySequence::Copy);
    mCopyLinesAction->setEnabled(false);
    connect(mCopyLinesAction, &QAction::triggered, this, &MainWindow::copySelectedLines);
}

void MainWindow::onRowsInserted() {
    if (mAutoScrollAction->isChecked()) {
        // Delay the call a bit to ensure the view has created the rows
        QTimer::singleShot(0, this, [this] { mTreeView->scrollToBottom(); });
    }
}

void MainWindow::onSelectionChanged() {
    int lineCount = mTreeView->selectionModel()->selectedRows().count();
    mCopyLinesAction->setEnabled(lineCount > 0);
    mCopyLinesAction->setText(tr("Copy %n line(s)", "", lineCount));
}

void MainWindow::showOpenLogDialog() {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilters({tr("Log files (*.log *.log.* *.txt)"), tr("All files (*)")});
    dialog.setWindowTitle(tr("Open log file"));
    if (!mLogPath.isEmpty()) {
        QString logDir = QFileInfo(mLogPath).absolutePath();
        dialog.setDirectory(logDir);
    }
    if (!dialog.exec()) {
        return;
    }
    loadLog(dialog.selectedFiles().first());
}

void MainWindow::showLogFormatDialog() {
    LogFormatDialog dialog(mLogFormatPath, this);
    if (!dialog.exec()) {
        return;
    }
    loadLogFormat(dialog.logFormatPath());
}

void MainWindow::copySelectedLines() {
    auto selectedRows = mTreeView->selectionModel()->selectedRows();
    if (selectedRows.empty()) {
        return;
    }
    QStringList list;
    for (const auto& index : selectedRows) {
        list << mLineProvider->lineAt(index.row());
    }
    qApp->clipboard()->setText(list.join("\n"));
}

void MainWindow::addLogToRecentFiles() {
    QStringList files = mConfig->recentLogFiles();
    files.removeOne(mLogPath);
    files.insert(0, mLogPath);
    while (files.length() > MAX_RECENT_FILES) {
        files.takeLast();
    }
    mConfig->setRecentLogFiles(files);
}

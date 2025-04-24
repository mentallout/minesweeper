#include "cell.h"
#include "mainwindow.h"

#include <QCloseEvent>
#include <QCoreApplication>
#include <QFile>
#include <QMenuBar>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QSettings>
#include <QTimer>

MainWindow::MainWindow(bool dbg, QWidget *parent) :
    QMainWindow(parent), isDbg(dbg), gameAreaWidget(new QWidget(this)), widthInput(new QLineEdit(this)),
    heightInput(new QLineEdit(this)), minesInput(new QLineEdit(this))
{
    if (QFile::exists(getIniFilePath()))
    {
        loadGameState();
    }
    else
    {
        createMenu();
    }
}

MainWindow::~MainWindow() {}

void MainWindow::displayMessage(const QString &message1, const QString &message2)
{
    QMessageBox::information(this, message1, message2);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveGameState();
    event->accept();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QSize newSize = event->size();
    if (gameGridLayout && gameGridLayout->count() != 0)
    {
        int minSide = qMin(newSize.width(), newSize.height());
        if (minSide < 400)
            resize(400, 400);
        else
            resize(minSide, minSide);
    }
    QWidget::resizeEvent(event);
}

bool MainWindow::validateInput(int &width, int &height, int &mines)
{
    bool valid = true;
    width = widthInput->text().toInt(&valid);
    if (!valid)
        return false;
    height = heightInput->text().toInt(&valid);
    if (!valid)
        return false;
    mines = minesInput->text().toInt(&valid);
    if (!valid)
        return false;
    if (mines >= width * height || mines < 1 || height < 1 || width < 1)
        return false;
    return valid;
}

void MainWindow::cleaning()
{
    if (gameGridLayout)
    {
        QLayoutItem *item;
        while ((item = gameGridLayout->takeAt(0)) != nullptr)
        {
            if (item->widget())
            {
                item->widget()->deleteLater();
            }
            delete item;
        }
        delete gameGridLayout;
        gameGridLayout = nullptr;
    }
    menuBar()->clear();
    if (toolBar)
    {
        removeToolBar(toolBar);
        toolBar->deleteLater();
        toolBar = nullptr;
    }
    if (gameLogic)
    {
        delete gameLogic;
        gameLogic = nullptr;
    }
}

void MainWindow::startNewGame()
{
    int width, height, mines;
    if (!validateInput(width, height, mines))
    {
        QString message = "Invalid input! Please enter valid numeric values!";
        if (isRus)
        {
            message = "Неверный ввод! Пожалуйста, введите допустимые числовые значения!";
        }
        QMessageBox::warning(this, "!", message);
        return;
    }
    createGameArea(width, height, mines);
}

void MainWindow::createMenu()
{
    widthInput->setText("10");
    heightInput->setText("10");
    minesInput->setText("10");
    widthLabel = new QLabel("Width:");
    heightLabel = new QLabel("Height:");
    minesLabel = new QLabel("Mines:");
    changeEngRus = new QPushButton("Change Language to Russian");
    changeRusEng = new QPushButton("Change Language to English");
    startButton = new QPushButton("Start New Game");
    connect(
        changeEngRus,
        &QPushButton::clicked,
        this,
        [this]()
        {
            if (!isRus)
            {
                isRus = true;
                enRuMenu();
            }
        });
    connect(
        changeRusEng,
        &QPushButton::clicked,
        this,
        [this]()
        {
            if (isRus)
            {
                isRus = false;
                ruEnMenu();
            }
        });
    connect(startButton, &QPushButton::clicked, this, &MainWindow::startNewGame);
    QVBoxLayout *inputLayout = new QVBoxLayout;
    QHBoxLayout *widthLayout = new QHBoxLayout;
    QHBoxLayout *heightLayout = new QHBoxLayout;
    QHBoxLayout *minesLayout = new QHBoxLayout;
    widthLayout->addWidget(widthLabel);
    widthLayout->addWidget(widthInput);
    heightLayout->addWidget(heightLabel);
    heightLayout->addWidget(heightInput);
    minesLayout->addWidget(minesLabel);
    minesLayout->addWidget(minesInput);
    inputLayout->addLayout(widthLayout);
    inputLayout->addLayout(heightLayout);
    inputLayout->addLayout(minesLayout);
    inputLayout->addWidget(startButton);
    inputLayout->addWidget(changeEngRus);
    inputLayout->addWidget(changeRusEng);
    QWidget *menuWidget = new QWidget;
    menuWidget->setLayout(inputLayout);
    setCentralWidget(menuWidget);
}

void MainWindow::createGameArea(int width, int height, int mines)
{
    cleaning();
    gameGridLayout = new QGridLayout(gameAreaWidget);
    gameGridLayout->setSpacing(0);
    isFirstMove = true;
    remainingMines = mines;
    currentWidth = width;
    currentHeight = height;
    currentMines = mines;
    gameLogic = new GameLogic(changeDbg, isLeftHandedMode, isFirstMove, isRus, currentWidth, currentHeight, remainingMines, gameGridLayout, this);
    connect(gameLogic, &GameLogic::showMessage, this, &MainWindow::displayMessage);
    mineCounterLabel = new QLabel(QString("Mines left: %1").arg(remainingMines));
    mineCounterLabel->setAlignment(Qt::AlignCenter);
    gameGridLayout->addWidget(mineCounterLabel, 0, 0, 1, width);
    sameNewGame = new QAction("Start new game with same parameters", this);
    newNewGame = new QAction("Start new game with new parameters", this);
    leftHanded = new QAction("Left-handed mode", this);
    changeEnRu = new QAction("Change Language to Russian", this);
    changeRuEn = new QAction("Change Language to English", this);
    QMenu *menu = menuBar()->addMenu(">***<");
    menu->addAction(sameNewGame);
    menu->addAction(newNewGame);
    menu->addAction(leftHanded);
    menu->addAction(changeEnRu);
    menu->addAction(changeRuEn);
    toolBar = addToolBar("Minesweeper");
    toolBar->addAction(sameNewGame);
    toolBar->addAction(newNewGame);
    toolBar->addAction(leftHanded);
    toolBar->addAction(changeEnRu);
    toolBar->addAction(changeRuEn);
    if (isDbg)
    {
        dbgMode = new QAction("Debug mode", this);
        menu->addAction(dbgMode);
        toolBar->addAction(dbgMode);
        connect(
            dbgMode,
            &QAction::triggered,
            this,
            [this]()
            {
                changeDbg = !changeDbg;
                gameLogic->revealSilently();
            });
    }
    connect(sameNewGame, &QAction::triggered, this, &MainWindow::restartWithSameParameters);
    connect(newNewGame, &QAction::triggered, this, &MainWindow::restartWithNewParameters);
    connect(leftHanded, &QAction::triggered, this, [this]() { isLeftHandedMode = !isLeftHandedMode; });
    connect(
        changeEnRu,
        &QAction::triggered,
        this,
        [this]()
        {
            if (!isRus)
            {
                isRus = true;
                enRuGame();
            }
        });
    connect(
        changeRuEn,
        &QAction::triggered,
        this,
        [this]()
        {
            if (isRus)
            {
                isRus = false;
                ruEnGame();
            }
        });

    for (int row = 1; row <= height; ++row)
    {
        for (int col = 0; col < width; ++col)
        {
            Cell *cell = new Cell(row, col, gameAreaWidget);
            gameGridLayout->addWidget(cell, row, col);
            cell->setMinimumSize(50, 50);
            cell->setText(" ");
            connect(cell,
                    &Cell::cellClicked,
                    this,
                    [this](Cell *cell, Qt::MouseButton button) { gameLogic->handleCellClick(cell, button); });
            connect(
                cell,
                &Cell::flagChanged,
                this,
                [this](int change)
                {
                    remainingMines += change;
                    if (isRus)
                        mineCounterLabel->setText(QString("Осталось мин: %1").arg(remainingMines));
                    else
                        mineCounterLabel->setText(QString("Mines left: %1").arg(remainingMines));
                });
        }
    }
    gameLogic->placeMines(width, height, mines);
    gameLogic->calculateAdjacentMines(width, height);
    gameAreaWidget->setLayout(gameGridLayout);
    setCentralWidget(gameAreaWidget);
    if (isRus)
    {
        enRuGame();
    }
    else
    {
        ruEnGame();
    }
}

void MainWindow::saveGameState()
{
    if (!gameGridLayout || gameGridLayout->count() == 0)
    {
        return;
    }
    QSettings settings(getIniFilePath(), QSettings::IniFormat);
    settings.beginGroup("Game");
    settings.setValue("width", currentWidth);
    settings.setValue("height", currentHeight);
    settings.setValue("mines", currentMines);
    settings.setValue("remainingMines", remainingMines);
    settings.setValue("isLeftHandedMode", isLeftHandedMode);
    settings.setValue("isRus", isRus);
    settings.setValue("isFirstMove", isFirstMove);
    settings.endGroup();
    settings.beginGroup("Cells");
    for (int row = 1; row <= currentHeight; ++row)
    {
        for (int col = 0; col < currentWidth; ++col)
        {
            Cell *cell = qobject_cast< Cell * >(gameGridLayout->itemAtPosition(row, col)->widget());
            if (cell)
            {
                QString key = QString("cell_%1_%2").arg(row).arg(col);
                settings.setValue(key + "_isMine", cell->isMine());
                settings.setValue(key + "_state", cell->currentState());
                settings.setValue(key + "_adjacentMines", cell->adjacentMines());
            }
        }
    }
    settings.endGroup();
}

void MainWindow::loadGameState()
{
    QSettings settings(getIniFilePath(), QSettings::IniFormat);
    settings.beginGroup("Game");
    int width = settings.value("width", 10).toInt();
    int height = settings.value("height", 10).toInt();
    int mines = settings.value("mines", 10).toInt();
    createGameArea(width, height, mines);
    remainingMines = settings.value("remainingMines", mines).toInt();
    isLeftHandedMode = settings.value("isLeftHandedMode", false).toBool();
    isRus = settings.value("isRus").toBool();
    isFirstMove = settings.value("isFirstMove").toBool();
    settings.endGroup();
    settings.beginGroup("Cells");
    for (int row = 1; row <= height; ++row)
    {
        for (int col = 0; col < width; ++col)
        {
            Cell *cell = qobject_cast< Cell * >(gameGridLayout->itemAtPosition(row, col)->widget());
            if (cell)
            {
                QString key = QString("cell_%1_%2").arg(row).arg(col);
                bool isMine = settings.value(key + "_isMine", false).toBool();
                int state = settings.value(key + "_state", Cell::Hidden).toInt();
                int adjacentMines = settings.value(key + "_adjacentMines", 0).toInt();
                cell->setMine(isMine);
                cell->setAdjacentMines(adjacentMines);
                if (state == Cell::Opened)
                {
                    cell->open();
                }
                else if (state == Cell::Flagged)
                {
                    cell->toggleFlagQuestion();
                }
                else if (state == Cell::Question)
                {
                    cell->toggleFlagQuestion();
                    cell->toggleFlagQuestion();
                }
            }
        }
    }
    if (isRus)
    {
        enRuGame();
    }
    else
    {
        ruEnGame();
    }
    settings.endGroup();
}

void MainWindow::restartWithNewParameters()
{
    cleaning();
    isLeftHandedMode = false;
    gameAreaWidget = new QWidget(this);
    widthInput = new QLineEdit(this);
    heightInput = new QLineEdit(this);
    minesInput = new QLineEdit(this);
    createMenu();
    if (isRus)
    {
        enRuMenu();
    }
    else
    {
        ruEnMenu();
    }
}

void MainWindow::restartWithSameParameters()
{
    createGameArea(currentWidth, currentHeight, currentMines);
}

void MainWindow::enRuMenu()
{
    widthLabel->setText("Ширина:");
    heightLabel->setText("Высота:");
    minesLabel->setText("Мины:");
    startButton->setText("Начать новую игру");
    changeEngRus->setText("Поменять язык на русский");
    changeRusEng->setText("Поменять язык на английский");
}

void MainWindow::ruEnMenu()
{
    widthLabel->setText("Width:");
    heightLabel->setText("Height:");
    minesLabel->setText("Mines:");
    startButton->setText("Start New Game");
    changeEngRus->setText("Change Language to Russian");
    changeRusEng->setText("Change Language to English");
}

void MainWindow::ruEnGame()
{
    sameNewGame->setText("Start new game with same parameters");
    newNewGame->setText("Start new game with new parameters");
    leftHanded->setText("Left-handed mode");
    changeEnRu->setText("Change Language to Russian");
    changeRuEn->setText("Change Language to English");
    if (isDbg)
        dbgMode->setText("Debug mode");
    mineCounterLabel->setText(QString("Mines left: %1").arg(remainingMines));
}

void MainWindow::enRuGame()
{
    sameNewGame->setText("Начать новую игру с теми же параметрами");
    newNewGame->setText("Начать новую игру с новыми параметрами");
    leftHanded->setText("Левша");
    changeEnRu->setText("Поменять язык на русский");
    changeRuEn->setText("Поменять язык на английский");
    if (isDbg)
        dbgMode->setText("Подглядывалка");
    mineCounterLabel->setText(QString("Осталось мин: %1").arg(remainingMines));
}

QString MainWindow::getIniFilePath() const
{
    return QCoreApplication::applicationDirPath() + "/gamestate.ini";
}

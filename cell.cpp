#include "cell.h"

#include <QMouseEvent>

Cell::Cell(int row, int col, QWidget *parent) :
    QPushButton(parent), m_isMine(false), m_row(row), m_col(col), m_adjacentMines(0), m_state(Hidden)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(50, 50);
}

bool Cell::isOpened() const
{
    return m_state == Opened;
}

bool Cell::isMine() const
{
    return m_isMine;
}

int Cell::row() const
{
    return m_row;
}

int Cell::col() const
{
    return m_col;
}

int Cell::adjacentMines() const
{
    return m_adjacentMines;
}

void Cell::open()
{
    if (m_state == Flagged || m_state == Question || m_state == Opened)
        return;
    m_state = Opened;
    if (m_isMine)
    {
        setText("M");
        setStyleSheet("background-color: red");
        setEnabled(false);
    }
    else if (m_adjacentMines > 0)
    {
        setText(QString::number(m_adjacentMines));
        setStyleSheet("background-color: lightgray");
    }
    else
    {
        setText(" ");
        setStyleSheet("background-color: lightgray");
        emit openAdjacentCells(this);
        setEnabled(false);
    }
}

void Cell::setMine(bool hasMine)
{
    m_isMine = hasMine;
}

void Cell::setAdjacentMines(int count)
{
    m_adjacentMines = count;
}

void Cell::toggleFlagQuestion()
{
    if (m_state == Opened)
        return;
    if (m_state == Hidden)
    {
        m_state = Flagged;
        setText(tr("⚐"));
        setStyleSheet("background-color: blue");
        emit flagChanged(-1);
    }
    else if (m_state == Flagged)
    {
        m_state = Question;
        setText("?");
        setStyleSheet("background-color: lightblue");
        emit flagChanged(1);
    }
    else if (m_state == Question)
    {
        m_state = Hidden;
        setText(" ");
        setStyleSheet(" ");
    }
}

void Cell::removeFlagQuestion()
{
    if (m_state == Flagged)
    {
        toggleFlagQuestion();
    }
    if (m_state == Question)
    {
        toggleFlagQuestion();
    }
}

Cell::State Cell::currentState() const
{
    return m_state;
}

void Cell::mousePressEvent(QMouseEvent *event)
{
    Qt::MouseButton button = event->button();
    if (button == Qt::LeftButton || button == Qt::RightButton)
    {
        emit cellClicked(this, button);
    }
    else if (button == Qt::MiddleButton)
    {
        if (m_state == Opened && m_adjacentMines > 0)
        {
            emit cellClicked(this, button);
        }
        else
        {
            return;
        }
    }
    QPushButton::mousePressEvent(event);
}

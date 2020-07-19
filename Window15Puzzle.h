#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QDateTime>
#include <QTimer>
#include <QEasingCurve>
#include <QtMath>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QThread>
#include <QTimeLine>
#include <QCloseEvent>
#include <QList>
#include <QtDebug>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum class Mode {Game,
            GameOver,
            JustShuffled,
            PuzzleMatched };

using Tile = QPushButton*;

class Form15Puzzle : public QMainWindow
{
	Q_OBJECT

public:
	Form15Puzzle(QWidget *parent = nullptr);
	~Form15Puzzle();

private slots:
	void on_tile1_pressed();

	void on_buttonShuffle_clicked();

	void timerResizeTimer();
	void timerCreateTilesTimer();
	void timerTimeTimer();
//	void panelClientResize();
//	void buttonMenuClick();

	void on_button3x3_clicked();
	void on_button4x4_clicked();
	void on_button5x5_clicked();

	void on_buttonPlace_clicked();
	void on_buttonDisappeare_clicked();
	void on_buttonPuzzleMatched_clicked();
	void on_buttonTimeRunningOut_clicked();
	void on_buttonTimeOver_clicked();


	void on_buttonScaleForAndroid_clicked();

private:
	Ui::MainWindow *ui;


public:
	  /* Public declarations */
	int base;
	Mode mode;
	void setMode(const Mode value);
	void setMaxTime();
	void setBase(const int value);

	void createTiles();
	std::vector< Tile > tiles;
	int tileSize;
	int tileSpacing;
	int spaceX, spaceY;
	QColor tileFillNormalColor1, tileFillNormalColor2;
	QDateTime lastResizeTime;
	QDateTime lastTapTime;
	bool closingAnimation = false;

	void tileMouseDown();
	bool tryMoveTile(int tilePosition, float moveAniDuration, bool waitAnimationEnd);
	void moveTile(int oldPosition, int newPosition, float moveAniDuration, bool waitAnimationEnd);
	void animateMoveTile(Tile tile, float moveAniDuration, bool waitAnimationEnd);
	bool checkCanPuzzleMatch();
	void checkPuzzleMatched();
	void calcConsts();
	void animatePlaceTilesFast();
	void animateTilesDisappeare();
	void animatePrepareBeforePlace();
	void animateBaseNotChanged();
	void animateTimeRunningOut();
	void animatePuzzleMatched();
	void animateTimeOver();
	void animateNormalizeTilesColor();
	void showDebug();
	void on_panelClient_clicked();
//	void startBlinkShuffle();
//	void stopBlinkShuffle();

//	void resizeEvent(QResizeEvent* event);
	void closeEvent (QCloseEvent *event);

	inline int  ind(int row, int col);
	int actualPosition(Tile tile);

	QTimer *timerTime;
	QTimer *timerResize;

	int timeRemaining;
	int panelDebugMaximumHeight;
	int resizeCount = 0;


};

QPropertyAnimation* animatePropertyDelay(QObject* const target, const QByteArray &propertyName,
                  const QVariant &value, uint duration_ms = 200, uint delay_ms = 0,
                  QEasingCurve interpolation = QEasingCurve::Linear,
                  bool deleteWhenStopped = true, bool waitAnimationEnd = false);


using TGenerateStyleSheetFunc = QString (*)(const QColor &color1, const QColor &color2);

class GradientAnimation
{
public:
	GradientAnimation(QWidget *aTarget, TGenerateStyleSheetFunc AGenerateStyleSheetFunc);
    ~GradientAnimation();
	void setCurColors(QColor aCurColor1, QColor aCurColor2);
	void stop();
	void start();

    QWidget *target;
    QTimeLine *timeLine;
	TGenerateStyleSheetFunc generateStyleSheetFunc;

    QColor startColor1;
    QColor startColor2;

    QColor stopColor1;
    QColor stopColor2;

    QColor curColor1;
    QColor curColor2;

    double delay_ms = 0;
    double duration_ms = 1000;
    QEasingCurve interpolation = QEasingCurve::Linear;
    bool autoReverse;

};

class MyEventFilter : public QObject
{
    Q_OBJECT
public:
    MyEventFilter(Form15Puzzle *aparent) : QObject() {parent = aparent;};
    ~MyEventFilter() = default;
protected:
    Form15Puzzle *parent;
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // MAINWINDOW_H

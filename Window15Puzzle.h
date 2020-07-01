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

enum TMode {Game,
            GameOver,
            JustShuffled,
            PuzzleMatched };

using TTile = QPushButton*;

class TForm15Puzzle : public QMainWindow
{
	Q_OBJECT

public:
	TForm15Puzzle(QWidget *parent = nullptr);
	~TForm15Puzzle();

private slots:
	void on_Tile1_pressed();

	void on_ButtonShuffle_clicked();

	void TimerResizeTimer();
	void TimerCreateTilesTimer();
	void TimerTimeTimer();
//	void PanelClientResize();
//	void ButtonMenuClick();

	void on_Button3x3_clicked();
	void on_Button4x4_clicked();
	void on_Button5x5_clicked();

	void on_ButtonPlace_clicked();
	void on_ButtonDisappeare_clicked();
	void on_ButtonPuzzleMatched_clicked();
	void on_ButtonTimeRunningOut_clicked();
	void on_ButtonTimeOver_clicked();


	void on_ButtonScaleForAndroid_clicked();

private:
	Ui::MainWindow *ui;


public:
	  /* Public declarations */
	int Base;
	TMode Mode;
	void SetMode(const TMode Value);
	void SetMaxTime();
	void SetBase(const int Value);

	void CreateTiles();
	std::vector< TTile > Tiles;
	int TileSize;
	int TileSpacing;
	int SpaceX, SpaceY;
	QColor TileFillNormalColor1, TileFillNormalColor2;
	QDateTime LastResizeTime;
	QDateTime LastTapTime;
	bool ClosingAnimation = false;

	void TileMouseDown();
	bool TryMoveTile(int TilePosition, float MoveAniDuration, bool WaitAnimationEnd);
	void MoveTile(int OldPosition, int NewPosition, float MoveAniDuration, bool WaitAnimationEnd);
	void AnimateMoveTile(TTile ATile, float MoveAniDuration, bool WaitAnimationEnd);
	bool CheckCanPuzzleMatch();
	void CheckPuzzleMatched();
	void CalcConsts();
	void AnimatePlaceTilesFast();
	void AnimateTilesDisappeare();
	void AnimatePrepareBeforePlace();
	void AnimateBaseNotChanged();
	void AnimateTimeRunningOut();
	void AnimatePuzzleMatched();
	void AnimateTimeOver();
	void AnimateNormalizeTilesColor();
	void ShowDebug();
	void on_PanelClient_clicked();
//	void StartBlinkShuffle();
//	void StopBlinkShuffle();

//	void resizeEvent(QResizeEvent* event);
	void closeEvent (QCloseEvent *event);

	inline int  ind(int Row, int Col);
	int ActualPosition(TTile ATile);
	void DivMod(int Dividend, uint16_t Divisor, uint16_t &Result, uint16_t &Remainder);


	QTimer *TimerTime;
	QTimer *TimerResize;

	int TimeRemaining;
	int PanelDebugMaximumHeight;
	int ResizeCount = 0;


};

QPropertyAnimation* AnimatePropertyDelay(QObject* const Target, const QByteArray &PropertyName,
                  const QVariant &Value, uint Duration_ms = 200, uint Delay_ms = 0,
                  QEasingCurve AInterpolation = QEasingCurve::Linear,
                  bool DeleteWhenStopped = true, bool WaitAnimationEnd = false);


using TGenerateStyleSheetFunc = QString (*)(const QColor &color1, const QColor &color2);

class TGradientAnimation
{
public:
	TGradientAnimation(QWidget *ATarget, TGenerateStyleSheetFunc AGenerateStyleSheetFunc);
    ~TGradientAnimation();
	void SetCurColors(QColor ACurColor1, QColor ACurColor2);
	void Stop();
	void Start();

    QWidget *Target;
    QTimeLine *timeLine;
	TGenerateStyleSheetFunc GenerateStyleSheetFunc;

    QColor StartColor1;
    QColor StartColor2;

    QColor StopColor1;
    QColor StopColor2;

    QColor CurColor1;
    QColor CurColor2;

    double Delay_ms = 0;
    double Duration_ms = 1000;
    QEasingCurve AInterpolation = QEasingCurve::Linear;
    bool AutoReverse;

};

class TMyEventFilter : public QObject
{
    Q_OBJECT
public:
    TMyEventFilter(TForm15Puzzle *aparent) : QObject() {parent = aparent;};
    ~TMyEventFilter() = default;
protected:
    TForm15Puzzle *parent;
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // MAINWINDOW_H

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

    void TimerCreateTilesTimer();
    void TimerTimeTimer();
    void TimerResizeTimer();
//    void PanelClientResize();
//    void ButtonMenuClick();

    void on_Button3x3_clicked();
    void on_Button4x4_clicked();
    void on_Button5x5_clicked();

    void on_ButtonPlace_clicked();
    void on_ButtonDisappeare_clicked();
    void on_ButtonPuzzleMatched_clicked();
    void on_ButtonTimeRunningOut_clicked();
    void on_ButtonTimeOver_clicked();

    void on_ChangeBackground_clicked();

private:
    Ui::MainWindow *ui;


public:
      /* Public declarations */
    int Base;
    TMode Mode;
    void SetMode( const TMode Value );
    void SetMaxTime( );
    void SetBase( const int Value );

    void CreateTiles( );
    std::vector< TTile > Tiles;
    int TileSize;
    int TileSpacing;
    int SpaceX, SpaceY;
    QColor TileFillNormalColor1, TileFillNormalColor2;
    QDateTime LastResizeTime;
    bool ClosingAnimation;
    bool GreenTiles;

    void TileMouseDown();
    bool TryMoveTile( int TilePosition, float MoveAniDuration, bool WaitAnimationEnd );
    void AnimateMoveTile( TTile ATile, float MoveAniDuration, bool WaitAnimationEnd );
    bool CheckCanPuzzleMatch( );
    void CheckPuzzleMatched( );
    void CalcConsts( );
    void AnimatePlaceTilesFast( );
    void AnimateTilesDisappeare( );
    void AnimatePrepareBeforePlace( );
    void AnimateBaseNotChanged( );
    void AnimateTimeRunningOut( );
    void AnimatePuzzleMatched( );
    void AnimateTimeOver( );
    void AnimateNormalizeTilesColor( );
//    void StartBlinkShuffle( );
//    void StopBlinkShuffle( );

    void resizeEvent(QResizeEvent* event);
    void closeEvent (QCloseEvent *event);

    inline int  ind( int Row, int Col );
    int ActualPosition(TTile ATile);
    void DivMod(int Dividend, uint16_t Divisor, uint16_t &Result, uint16_t &Remainder);


    QTimer *TimerTime;
    QTimer *TimerResize;

    int TimeRemaining;

};

QPropertyAnimation* AnimatePropertyDelay(QObject* const Target, const QByteArray &PropertyName,
                  const QVariant &Value, uint Duration_ms = 200, uint Delay_ms = 0,
                  QEasingCurve AInterpolation = QEasingCurve::Linear,
                  bool DeleteWhenStopped = true, bool WaitAnimationEnd = false);

class TGradientAnimation
{
public:
    TGradientAnimation(QWidget *ATarget);
    ~TGradientAnimation();

    QWidget *Target;
    QTimeLine *timeLine;

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
    bool FirstStart = true;
    void Stop();
    void Start();

};


#endif // MAINWINDOW_H

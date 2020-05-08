#include "Window15Puzzle.h"
#include "ui_Window15Puzzle.h"


const double MaxMoveAniDuration = 150;
const double MinMoveAniDuration = 1;



TForm15Puzzle::TForm15Puzzle(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    TimerTime = new QTimer();
    connect(TimerTime, SIGNAL(timeout()), this, SLOT(TimerTimeTimer()));
    TimerTime->setInterval(1000);

    TimerCreateTiles = new QTimer();
    connect(TimerCreateTiles, SIGNAL(timeout()), this, SLOT(TimerCreateTilesTimer()));

    TimerResize = new QTimer();
    connect(TimerResize, SIGNAL(timeout()), this, SLOT(TimerResizeTimer()));
    TimerResize->setInterval(100);

    LastResizeTime = QDateTime::currentDateTime();   //To prevent resize on start on Android

    TileFillNormalColor1 = QColor( 255, 244, 71 );
    TileFillNormalColor2 = QColor( 175, 255, 100 );

    TGradientAnimation *GradientAni = new TGradientAnimation(ui->Tile1, this);
    GradientAni->StartColor1 = TileFillNormalColor1;
    GradientAni->StartColor2 = TileFillNormalColor2;

    ui->Tile1->setProperty("GradientAni", QVariant(uint(GradientAni)) );
    ui->Tile1->raise();

    SetBase(4);
}

TForm15Puzzle::~TForm15Puzzle()
{
    delete ui;
}



//void TForm15Puzzle::on_Tile1_clicked()
//{
//    QRect geometry = ui->Tile1->geometry();
//    geometry.adjust(20, 0, 20, 0);

//    AnimateGeometryDelay(ui->Tile1, geometry, 5000, 0, QEasingCurve::OutExpo );
//}

void TForm15Puzzle::AnimatePropertyDelay(QObject * const Target, const QByteArray &PropertyName,
                  const QVariant &Value, uint Duration_ms, uint Delay_ms,
                  QEasingCurve AInterpolation)
{
    QPropertyAnimation *ani = new QPropertyAnimation(Target, PropertyName);
    ani->setDuration(Duration_ms);

    ani->setEasingCurve(AInterpolation);

    ani->setEndValue(Value);

    if (Delay_ms == 0)
        ani->start(QAbstractAnimation::DeleteWhenStopped);
    else
        QTimer::singleShot(Delay_ms, ani, SLOT(start()));


//    QSequentialAnimationGroup group;
    if (WaitAnimationEnd)
    while (ani->state() == QPropertyAnimation::Running)
    {
            qApp->processEvents();
            QThread::msleep(1);

    }
}


QColor InterpolateColor(const QColor &Start, const QColor &Stop, double T)
{
  QColor Result;
  Result.setAlpha(  Start.alpha() + ((Stop.alpha()   - Start.alpha() ) * T) );
  Result.setRed(    Start.red()   + ((Stop.red()     - Start.red()   ) * T) );
  Result.setGreen(  Start.green() + ((Stop.green()   - Start.green() ) * T) );
  Result.setBlue(   Start.blue()  + ((Stop.blue()    - Start.blue()  ) * T) );
  return Result;
}

QString GenerateStyleSheet(const QColor &color1, const QColor &color2)
{

    return QString("border: 4px outset red;\n"
    "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, "
    "stop:0 ") + color1.name() + ", stop:1 " + color2.name() + ");";

}


TGradientAnimation::TGradientAnimation(QWidget *ATarget, QMainWindow *AMainWindow)
{
   Target = ATarget;
   MainWindow = AMainWindow;
   timeLine = new QTimeLine(Duration_ms, Target);
}

TGradientAnimation::~TGradientAnimation()
{
//    MainWindow->connect(timeLine, &QTimeLine::finished, timeLine, &QTimeLine::deleteLater);
   timeLine->stop();
   delete timeLine;
}

void TGradientAnimation::Stop()
{
    timeLine->stop();
}

void TGradientAnimation::Start()
{
    int FrameRange = 1023;
    timeLine->setFrameRange(0, FrameRange);
    timeLine->setEasingCurve(AInterpolation);
    timeLine->setDuration(Duration_ms);

    if (FirstStart)
        FirstStart = false;
    else
    {
        StartColor1 = CurColor1;
        StartColor2 = CurColor2;
    }

    if (AutoReverse)
        timeLine->setDuration(Duration_ms * 2);
    else
        timeLine->setDuration(Duration_ms);

    MainWindow->connect(timeLine, &QTimeLine::frameChanged, [=](int frame){
        double NormalizedTime = double(frame) / FrameRange;

        if (AutoReverse)
        {
            if (NormalizedTime < 0.5)
                NormalizedTime = NormalizedTime * 2;
            else
                NormalizedTime = (1 - NormalizedTime) * 2;
        }

        CurColor1 = InterpolateColor(StartColor1, StopColor1, NormalizedTime);
        CurColor2 = InterpolateColor(StartColor2, StopColor2, NormalizedTime);
        Target->setStyleSheet(GenerateStyleSheet(CurColor1, CurColor2));
    });

    timeLine->start();

}


void TForm15Puzzle::on_ChangeBackground_clicked()
{
//     ui->Tile1->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(color1.red()).arg(color1.green()).arg(color1.blue()));
//    TGradientAnimation *GradientAni = new TGradientAnimation(ui->Tile1, this);

  TGradientAnimation *GradientAni = ( TGradientAnimation* )ui->Tile1->property("GradientAni").toUInt() ;

    GradientAni->StopColor1 = QColor("olive");;
    GradientAni->StopColor2 = QColor("darkorange");
    GradientAni->Delay_ms = 0;
    GradientAni->Duration_ms = 500;
    GradientAni->AutoReverse = true;
    GradientAni->Start();
}


void  TForm15Puzzle::SetMode( const TMode Value )
{
  Mode = Value;
  if ( Mode == Game )
    TimerTime->start();
  else
    TimerTime->stop();
}



void TForm15Puzzle::on_Button3x3_clicked()
{
    QPushButton* Sender = (QPushButton*)QObject::sender();
    int LBase = QString(Sender->text().at(0)).toInt();
    SetBase(LBase);
}

void TForm15Puzzle::on_Button4x4_clicked()
{
    on_Button3x3_clicked();
}

void TForm15Puzzle::on_Button5x5_clicked()
{
   on_Button3x3_clicked();
}



void  TForm15Puzzle::SetBase( const int Value )
{
  if ( Value == Base )
  {
    AnimateBaseNotChanged();
    return;
  }
  SetMode(GameOver);
  AnimateTilesDisappeare();
  Base = Value;
  SetMaxTime();
  if ( Tiles.size() > 0 )
    TimerCreateTiles->setInterval( 520 + 30 * Tiles.size() );
  else
    TimerCreateTiles->setInterval(50);
  TimerCreateTiles->start();
}


void  TForm15Puzzle::TimerCreateTilesTimer( )
{
  TimerCreateTiles->stop();
  CreateTiles();
  AnimatePrepareBeforePlace();
  AnimatePlaceTilesFast();
}




void  TForm15Puzzle::CreateTiles( )
{

  TRectangle Tile1 = ui->Tile1;

  Tile1->setVisible(false);

  for ( uint i = 0; i < Tiles.size(); i++)
    if ( Tiles[i] != NULL )
      if ( Tiles[i] == Tile1 )
        Tiles[i] = NULL;
      else
      {
        TGradientAnimation *GradientAni = new TGradientAnimation(Tiles[i], this);
        delete GradientAni;
        delete Tiles[i];
        Tiles[i] = NULL;
      }
  Tiles.resize(Base * Base);
  Tiles[0] = Tile1;
  for ( uint i = 0; i < Tiles.size() - 1; i++)
    if ( Tiles[i] == NULL )
    {
      TRectangle NewTile;

      NewTile = new QPushButton(this);

      connect(NewTile, SIGNAL(pressed()), this, SLOT(on_Tile1_pressed()));

      NewTile->setText(QString::number(i + 1));


      TGradientAnimation *GradientAni = new TGradientAnimation(NewTile, this);
      GradientAni->StartColor1 = TileFillNormalColor1;
      GradientAni->StartColor2 = TileFillNormalColor2;
      NewTile->setProperty("GradientAni", QVariant(uint(GradientAni)) );


////      NewTile.Position.X := Tile1.Position.X;
////      NewTile.Position.Y := Tile1.Position.Y;
////      NewTile.Opacity := 1;
      NewTile->setStyleSheet(Tile1->styleSheet());
      NewTile->setParent(this);
//      NewTile.SendToBack;
      Tiles[i] = NewTile;
    }

  if ( Tiles[Tiles.size() - 1] != NULL )
    Tiles[Tiles.size() - 1] = NULL;
}


int  TForm15Puzzle::ind( int Row, int Col )
{
    return Row * Base + Col;
}

void TForm15Puzzle::DivMod(int Dividend, uint16_t Divisor, uint16_t &Result, uint16_t &Remainder)
{
    Result = Dividend / Divisor;
    Remainder = Dividend % Divisor;
}



int TForm15Puzzle::ActualPosition(TRectangle ATile)
{
    for ( uint i = 0; i < Tiles.size(); i++)
        if (Tiles[i] == ATile)
            return i;
    return 0;
}



void  TForm15Puzzle::on_Tile1_pressed( )
{
  QObject* Sender = QObject::sender();
  TRectangle SenderTile = (TRectangle) Sender;
  bool WasMoved = false;
  if ( Mode == JustShuffled )
    SetMode(Game);
  WasMoved = TryMoveTile( ActualPosition(SenderTile), MaxMoveAniDuration );
  if ( WasMoved )
    CheckPuzzleMatched();
}


bool  TForm15Puzzle::TryMoveTile( int TilePosition, float MoveAniDuration )
{
  uint16_t RowPressed = 0, ColPressed = 0;
  int RowNoTile = 0, ColNoTile = 0;
  bool WasMoved = false;
  int NewPosition = 0;
  DivMod( TilePosition, Base, RowPressed, ColPressed );
  WasMoved = false;
  for ( int Row = 0; Row < Base; Row++)
    if ( Tiles[ind( Row, ColPressed )] == NULL )
    {
      RowNoTile = Row;
      if ( RowNoTile > RowPressed )
        for ( int RowToMove = RowNoTile - 1; RowToMove >= RowPressed; RowToMove--)
        {
          NewPosition = ind( RowToMove + 1, ColPressed );
          Tiles[NewPosition] = Tiles[ind( RowToMove, ColPressed )];
//          Tiles[NewPosition].Tag = NewPosition;
          Tiles[ind( RowToMove, ColPressed )] = NULL;
          AnimateMoveTile( Tiles[NewPosition], MoveAniDuration );
          WasMoved = true;
        }
      if ( RowPressed > RowNoTile )
        for ( int RowToMove = RowNoTile + 1; RowToMove <= RowPressed; RowToMove++)
        {
          NewPosition = ind( RowToMove - 1, ColPressed );
          Tiles[NewPosition] = Tiles[ind( RowToMove, ColPressed )];
//          Tiles[NewPosition].Tag = NewPosition;
          Tiles[ind( RowToMove, ColPressed )] = NULL;
          AnimateMoveTile( Tiles[NewPosition], MoveAniDuration );
          WasMoved = true;
        }
    }
  if ( ! WasMoved )
    for ( int Col = 0; Col < Base; Col++)
      if ( Tiles[ind( RowPressed, Col )] == NULL )
      {
        ColNoTile = Col;
        if ( ColNoTile > ColPressed )
          for ( int ColToMove = ColNoTile - 1; ColToMove >= ColPressed; ColToMove--)
          {
            NewPosition = ind( RowPressed, ColToMove + 1 );
            Tiles[NewPosition] = Tiles[ind( RowPressed, ColToMove )];
//            Tiles[NewPosition].Tag = NewPosition;
            Tiles[ind( RowPressed, ColToMove )] = NULL;
            AnimateMoveTile( Tiles[NewPosition], MoveAniDuration );
            WasMoved = true;
          }
        if ( ColPressed > ColNoTile )
          for ( int ColToMove = ColNoTile + 1; ColToMove <= ColPressed; ColToMove++)
          {
            NewPosition = ind( RowPressed, ColToMove - 1 );
            Tiles[NewPosition] = Tiles[ind( RowPressed, ColToMove )];
//            Tiles[NewPosition].Tag = NewPosition;
            Tiles[ind( RowPressed, ColToMove )] = NULL;
            AnimateMoveTile( Tiles[NewPosition], MoveAniDuration );
            WasMoved = true;
          }
      }

  return WasMoved;
}


void  TForm15Puzzle::AnimateMoveTile( TRectangle ATile, float MoveAniDuration )
{
  uint16_t NewRow = 0, NewCol = 0;
  int X = 0, Y = 0;
  DivMod( ActualPosition(ATile), Base, NewRow, NewCol );

  QRect geometry = ATile->geometry();
  X = SpaceX + round( NewCol * ( TileSize + TileSpacing ) );
  Y = SpaceY + round( NewRow * ( TileSize + TileSpacing ) );

  geometry.setRect(X, Y, TileSize, TileSize);


  if ( MoveAniDuration > 0 )
  {
    AnimatePropertyDelay(ATile, "geometry", geometry, MoveAniDuration, 0, QEasingCurve::OutExpo );
  }
  else
  {
//    ATile.Position.X = X;
//    ATile.Position.Y = Y;
    ATile->setGeometry(geometry);
  }
}


void  TForm15Puzzle::CheckPuzzleMatched( )
{
  bool LPuzzleMatched = true;
  for ( uint i = 0; i < Tiles.size(); i++)
    if ( Tiles[i] != NULL )
    {
      int TextNumber = Tiles[i]->text().toInt();

      if ( (TextNumber - 1) != ActualPosition(Tiles[i]) )
      {
        LPuzzleMatched = false;
        break;
      }
    }

  if ( LPuzzleMatched && ( Mode == Game ) )
  {
    SetMode(PuzzleMatched);
    AnimatePuzzleMatched();
  }
  if ( ( ! LPuzzleMatched ) && ( ( Mode == PuzzleMatched ) || ( Mode == JustShuffled ) ) )
  {
    AnimateNormalizeTilesColor();
    if ( Mode == PuzzleMatched )
      SetMode(GameOver);
  }
}



void TForm15Puzzle::on_ButtonShuffle_clicked()
{
    int NewI = 0;
    int MoveCount = 0;
    float MoveAniDuration = 0.0;
    WaitAnimationEnd = true;
    MoveCount = Tiles.size() * Tiles.size();
    MoveAniDuration = MaxMoveAniDuration;
    for ( int i = 1; i <= MoveCount; i++)
    {
      if ( i <= 10 )
        MoveAniDuration = MinMoveAniDuration + ( MaxMoveAniDuration * ( 1 - ( double( i ) / 10 ) ) );
      if ( i >= MoveCount - 10 )
        MoveAniDuration = MinMoveAniDuration + ( ( MaxMoveAniDuration / 2 ) * ( 1 - ( double( ( MoveCount - i ) ) / 10 ) ) );
      if ( ( i > 20 ) && ( i < MoveCount - 20 ) )
        if ( ( i % 10 ) == 0 )
          MoveAniDuration = MinMoveAniDuration;
        else
          MoveAniDuration = 0;
      do
      {
        NewI = rand() % Tiles.size();
      }
      while ( ! ( TryMoveTile( NewI, MoveAniDuration ) ) );
    }
    WaitAnimationEnd = false;
    SetMaxTime();
//    StopBlinkShuffle();

    SetMode(JustShuffled);
    CheckPuzzleMatched();
}

void  TForm15Puzzle::CalcConsts( )
{
  int Height = this->height();
  int Width = this->width();
  /*# with PanelClient do */
  if ( Height > Width )
  {
    SpaceX = round( double( Width ) / 20 );
    TileSize = round( double( ( Width - SpaceX * 2 ) ) / Base );
    SpaceY = SpaceX + round( double( ( Height - Width ) ) / 2 );
  }
  else
  {
    SpaceY = round( double( Height ) / 20 );
    TileSize = round( double( ( Height - SpaceY * 2 ) ) / Base );
    SpaceX = SpaceY + round( double( ( Width - Height ) ) / 2 );
  }
  TileSpacing = round( TileSize * 0.06 );
  TileSize = round( TileSize * 0.94 );
  SpaceX = SpaceX + round( double( TileSpacing ) / 2 );
  SpaceY = SpaceY + round( double( TileSpacing ) / 2 );
}


void  TForm15Puzzle::AnimatePlaceTilesFast( )
{
  int X = 0, Y = 0;
  uint16_t Row = 0, Col = 0;
  CalcConsts();
//  for (TRectangle CurTile : Tiles)
  for ( uint i = 0; i < Tiles.size(); i++)
    if ( Tiles[i] != NULL )
    {
      DivMod( i, Base, Row, Col );

      QRect geometry = Tiles[i]->geometry();
      X = SpaceX + round( Col * ( TileSize + TileSpacing ) );
      Y = SpaceY + round( Row * ( TileSize + TileSpacing ) );

      geometry.setRect(X, Y, TileSize, TileSize);


      AnimatePropertyDelay(Tiles[i], "geometry", geometry, 200, ( 0 + 30 * i ), QEasingCurve::OutExpo );

//      AnimatePropertyDelay(Tiles[i]->font(), "pixelSize", geometry, 200, ( 0 + 30 * i ), QEasingCurve::OutExpo );
      QFont font = Tiles[i]->font();
      font.setPointSize(TileSize / 3);
      Tiles[i]->setFont(font);
    }
}

void  TForm15Puzzle::AnimateTilesDisappeare( )
{
  int X = 0, Y = 0;
  for ( uint i = 0; i < Tiles.size(); i++)
    if ( Tiles[i] != NULL )
    {
        QRect geometry = Tiles[i]->geometry();

        X = geometry.x()  + round( double( TileSize ) / 2 );
        Y = geometry.y() + TileSize;

        geometry.setRect(X, Y, 0, 0);

        AnimatePropertyDelay(Tiles[i], "geometry", geometry, 400, ( 0 + 30 * i ), QEasingCurve::/*OutExpo*/InBack );

//      TAnimator.AnimateFloatDelay( Tiles[i], "Scale.X", 0.1, 0.4 * slowdown, ( 0.03 * i ) * slowdown );
//      TAnimator.AnimateFloatDelay( Tiles[i], "Scale.Y", 0.1, 0.4 * slowdown, ( 0.03 * i ) * slowdown );
//      TAnimator.AnimateFloatDelay( Tiles[i], "RotationAngle", 45, 0.4 * slowdown, ( 0.03 * i ) * slowdown );
//      TAnimator.AnimateFloatDelay( Tiles[i], "Position.Y", Tiles[i].Position.Y + TileSize, 0.4 * slowdown, ( 0.03 * i ) * slowdown, TAnimationType.In, TInterpolationType.Back );
//      TAnimator.AnimateFloatDelay( Tiles[i], "Position.X", Tiles[i].Position.X + Round( double( TileSize ) / 2 ), 0.4 * slowdown, ( 0.03 * i ) * slowdown );
//      TAnimator.AnimateFloatDelay( Tiles[i], "Opacity", 0, 0.4 * slowdown, ( 0.1 + 0.03 * i ) * slowdown );
    }
}


void  TForm15Puzzle::AnimateBaseNotChanged( )
{
//  int i = 0;
//  for ( int stop = Tiles.Length - 1, i = 0; i <= stop; i++)
//    if ( Tiles[i] != NULL )
//    {
//      TAnimator.AnimateFloatDelay( Tiles[i], "RotationAngle", - 20, 0.1 * slowdown, 0 * slowdown, TAnimationType.InOut, TInterpolationType.Linear );
//      TAnimator.AnimateFloatDelay( Tiles[i], "RotationAngle", 20, 0.25 * slowdown, 0.1 * slowdown, TAnimationType.InOut, TInterpolationType.Exponential );
//      TAnimator.AnimateFloatDelay( Tiles[i], "RotationAngle", 0, 0.25 * slowdown, 0.35 * slowdown, TAnimationType.Out, TInterpolationType.Back );
//    }
    int X = 0, Y = 0;
    for ( uint i = 0; i < Tiles.size(); i++)
      if ( Tiles[i] != NULL )
      {
          QRect OrigGeometry = Tiles[i]->geometry();
          QRect geometry = OrigGeometry;

          int offset = round( double( TileSize ) / 4 );
          int size = round( double( TileSize ) / 2 );

          X = geometry.x()  + offset;
          Y = geometry.y() + offset;

          geometry.setRect(X, Y, size, size );
          AnimatePropertyDelay(Tiles[i], "geometry", geometry, 300, ( 0 + 30 * i ), QEasingCurve::InOutExpo );

          AnimatePropertyDelay(Tiles[i], "geometry", OrigGeometry, 300, ( 350 + 30 * i ), QEasingCurve::OutBack );
      }
}



void  TForm15Puzzle::AnimatePrepareBeforePlace( )
{
    int X = 0, Y = 0;
    uint16_t Row = 0, Col = 0;

  CalcConsts();

  for ( uint i = 0; i < Tiles.size(); i++)
    if ( Tiles[i] != NULL )
    {
        DivMod( i, Base, Row, Col );

        QRect geometry = Tiles[i]->geometry();
        X = SpaceX + round( Col * ( TileSize + TileSpacing ) );
        Y = SpaceY + round( Row * ( TileSize + TileSpacing ) );

        X = X  + round( double( TileSize ) / 2 );
        Y = Y + TileSize;

        geometry.setRect(X, Y, 0, 0);


      Tiles[i]->setGeometry(geometry);
      Tiles[i]->show();
    }
}





void  TForm15Puzzle::SetMaxTime( )
{
  uint16_t Sec = 0, Min = 0;
  TimeRemaining = ( ( Base * Base * Base * Base ) / 20 ) * 10;
  DivMod( TimeRemaining, 60, Min, Sec );
  ui->TextTime->setText(QString("%1:%2").arg(Min).arg(Sec, 2, 10, QLatin1Char('0')));
}


void TForm15Puzzle::TimerTimeTimer()
{
    uint16_t Min = 0, Sec = 0;
    TimeRemaining = TimeRemaining - 1;
    DivMod( TimeRemaining, 60, Min, Sec );
    ui->TextTime->setText(QString("%1:%2").arg(Min).arg(Sec, 2, 10, QLatin1Char('0')));

    if ( TimeRemaining == 0 )
    {
      SetMode(GameOver);
      AnimateTimeOver();
//      StartBlinkShuffle();
      return;
    }
    if ( TimeRemaining <= 10 )
      AnimateTimeRunningOut();

}



void TForm15Puzzle::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    TimerResize->stop();
    TimerResize->start();

}


void TForm15Puzzle::TimerResizeTimer()
{
    TimerResize->stop();
    uint TimeFromLastResize_ms = LastResizeTime.msecsTo(QDateTime::currentDateTime());

    if ( TimeFromLastResize_ms > 500 )
    {
      AnimatePlaceTilesFast();
      LastResizeTime = QDateTime::currentDateTime();
    }

}


void  TForm15Puzzle::AnimateTimeRunningOut( )
{
  for ( uint i = 0; i < Tiles.size(); i++)
    if ( Tiles[i] != NULL )
    {
        TGradientAnimation *GradientAni = ( TGradientAnimation* )Tiles[i]->property("GradientAni").toUInt() ;
      GradientAni->StopColor1 = TileFillNormalColor1;
      GradientAni->StopColor2 = QColor("darkorange");
      GradientAni->Delay_ms = 0;
      GradientAni->Duration_ms = 150;
      GradientAni->AutoReverse = true;
      GradientAni->Start();
    }
}


void  TForm15Puzzle::AnimateTimeOver( )
{
  for ( uint i = 0; i < Tiles.size(); i++)
    if ( Tiles[i] != NULL )
    {
        TGradientAnimation *GradientAni = ( TGradientAnimation* )Tiles[i]->property("GradientAni").toUInt() ;
      GradientAni->Stop();
      GradientAni->StopColor1 = Qt::gray;
      GradientAni->StopColor2 = Qt::red;
      GradientAni->Delay_ms = 0;
      GradientAni->Duration_ms = 600;
      GradientAni->AutoReverse = false;
      GradientAni->Start();
    }
}


void  TForm15Puzzle::AnimateNormalizeTilesColor( )
{
  for ( uint i = 0; i < Tiles.size(); i++)
    if ( Tiles[i] != NULL )
    {
        TGradientAnimation *GradientAni = ( TGradientAnimation* )Tiles[i]->property("GradientAni").toUInt() ;
      GradientAni->StopColor1 = TileFillNormalColor1;
      GradientAni->StopColor2 = TileFillNormalColor2;
      GradientAni->Delay_ms = 0;
      GradientAni->Duration_ms = 200;
      GradientAni->AutoReverse = false;
      GradientAni->Start();
    }
}


void  TForm15Puzzle::AnimatePuzzleMatched( )
{
  for ( uint i = 0; i < Tiles.size(); i++)
    if ( Tiles[i] != NULL )
    {
//      TAnimator.AnimateFloatDelay( Tiles[i], "RotationAngle", 360, 1 * slowdown, 0.35 * slowdown, TAnimationType.Out, TInterpolationType.Back );

        TGradientAnimation *GradientAni = ( TGradientAnimation* )Tiles[i]->property("GradientAni").toUInt() ;
      GradientAni->Stop();
      GradientAni->StopColor1 = QColor("lawngreen");
      GradientAni->StopColor2 = QColor("gold");
      GradientAni->Delay_ms = ( 1 + i * 0.1 );
      GradientAni->Duration_ms = 500 ;
      GradientAni->AutoReverse = false;
      GradientAni->Start();
    }
}


void TForm15Puzzle::on_ButtonPlace_clicked()
{
    if (GreenTiles)
    {
      AnimateNormalizeTilesColor();
      GreenTiles = false;
    }
    AnimatePlaceTilesFast();
}

void TForm15Puzzle::on_ButtonDisappeare_clicked()
{
    AnimateTilesDisappeare();
}



void TForm15Puzzle::on_ButtonPuzzleMatched_clicked()
{
    AnimatePuzzleMatched( );
    GreenTiles = true;

}

void TForm15Puzzle::on_ButtonTimeRunningOut_clicked()
{
    AnimateTimeRunningOut( );
}

void TForm15Puzzle::on_ButtonTimeOver_clicked()
{
    AnimateTimeOver( );
//    ui->ButtonTimeOver->setText( QColor("gold").name() );
}

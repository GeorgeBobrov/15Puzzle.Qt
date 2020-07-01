#include "Window15Puzzle.h"
#include "ui_Window15Puzzle.h"


const double MaxMoveAniDuration = 150;
const double MinMoveAniDuration = 1;

int TileBorderThickness = 4;

QString GenerateTileStyleSheet(const QColor &color1, const QColor &color2)
{
	return QString("border: " + QString::number(TileBorderThickness) + "px solid #FFE55555;\n"
	"border-radius: " + QString::number(TileBorderThickness) + "px;\n"
	"background-color: qlineargradient(spread:pad, x1:0.35, y1:0.35, x2:0.9, y2:0.9, "
	"stop:0 ") + color1.name() + ", stop:1 " + color2.name() + ");";
}


TForm15Puzzle::TForm15Puzzle(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	TMyEventFilter *MyEventFilter = new TMyEventFilter(this);
	ui->PanelClient->installEventFilter(MyEventFilter);

	TimerTime = new QTimer();
	connect(TimerTime, SIGNAL(timeout()), this, SLOT(TimerTimeTimer()));
	TimerTime->setInterval(1000);

	TimerResize = new QTimer();
	connect(TimerResize, SIGNAL(timeout()), this, SLOT(TimerResizeTimer()));
	TimerResize->setInterval(200);

	LastResizeTime = QDateTime::currentDateTime();   //To prevent resize on start on Android
//	qDebug() << QString("FormCreate  ") << QDateTime::currentDateTime().toString("mm:ss:zzz");

	TileFillNormalColor1 = QColor("bisque");
	TileFillNormalColor2 = QColor("#FFABE024");

//  PanelClient  background-color: rgb(244, 244, 244);

	srand(unsigned(time(0)));
	SetBase(4);

	PanelDebugMaximumHeight = ui->PanelDebug->height();
//	ui->PanelDebug->setMaximumHeight(0);

	ui->PanelDebug->setVisible(false);
}

TForm15Puzzle::~TForm15Puzzle()
{
	delete ui;
}


void  TForm15Puzzle::SetMode(const TMode Value)
{
	Mode = Value;
	if (Mode == Game)
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



void TForm15Puzzle::SetBase(const int Value)
{
	if (Value == Base)
	{
		AnimateBaseNotChanged();
		return;
	}
	SetMode(GameOver);
	AnimateTilesDisappeare();
	Base = Value;
	SetMaxTime();

	QTimer::singleShot(100, this, SLOT(TimerCreateTilesTimer()));
}



void TForm15Puzzle::TimerCreateTilesTimer()
{
#ifdef Q_OS_ANDROID
	on_ButtonScaleForAndroid_clicked();
#endif

	CreateTiles();
	AnimatePrepareBeforePlace();
	AnimatePlaceTilesFast();
}



void TForm15Puzzle::CreateTiles()
{
	for (uint i = 0; i < Tiles.size(); i++)
		if (Tiles[i] != NULL)
		{
			TGradientAnimation *GradientAni = (TGradientAnimation*)Tiles[i]->property("GradientAni").value<void *>() ;
			delete GradientAni;
			delete Tiles[i];
			Tiles[i] = NULL;
		}

	Tiles.resize(Base * Base);
	for (uint i = 0; i < Tiles.size() - 1; i++)
		if (Tiles[i] == NULL)
		{
			TTile NewTile;

			NewTile = new QPushButton(this);

			connect(NewTile, SIGNAL(pressed()), this, SLOT(on_Tile1_pressed()));

			NewTile->setText(QString::number(i + 1));

			TGradientAnimation *GradientAni = new TGradientAnimation(NewTile, GenerateTileStyleSheet);
			NewTile->setProperty("GradientAni", qVariantFromValue((void*) GradientAni));

			NewTile->setStyleSheet(GenerateTileStyleSheet(TileFillNormalColor1, TileFillNormalColor2));
			GradientAni->SetCurColors(TileFillNormalColor1, TileFillNormalColor2);

			NewTile->setParent(ui->PanelClient);
//			NewTile.SendToBack;
			Tiles[i] = NewTile;
		}

	if (Tiles[Tiles.size() - 1] != NULL)
		Tiles[Tiles.size() - 1] = NULL;
}


int TForm15Puzzle::ind(int Row, int Col)
{
	return Row * Base + Col;
}

void TForm15Puzzle::DivMod(int Dividend, uint16_t Divisor, uint16_t &Result, uint16_t &Remainder)
{
	Result = Dividend / Divisor;
	Remainder = Dividend % Divisor;
}

int TForm15Puzzle::ActualPosition(TTile ATile)
{
	for (uint i = 0; i < Tiles.size(); i++)
		if (Tiles[i] == ATile)
			return i;
	return 0;
}


void TForm15Puzzle::on_Tile1_pressed()
{
	TTile SenderTile = (TTile) QObject::sender();
	if (Mode == JustShuffled)
		SetMode(Game);
	bool WasMoved = TryMoveTile(ActualPosition(SenderTile), MaxMoveAniDuration, false);
	if (WasMoved)
		CheckPuzzleMatched();
}


bool TForm15Puzzle::TryMoveTile(int TilePosition, float MoveAniDuration, bool WaitAnimationEnd)
{
	bool WasMoved = false;
	
	uint16_t RowPressed = 0, ColPressed = 0;
	DivMod(TilePosition, Base, RowPressed, ColPressed);

	for (int Row = 0; Row < Base; Row++)
		if (Tiles[ind(Row, ColPressed)] == NULL)
		{
			int RowNoTile = Row;
			if (RowNoTile > RowPressed) //Move tiles down
				for (int RowToMove = RowNoTile - 1; RowToMove >= RowPressed; RowToMove--)
				{
					MoveTile(ind(RowToMove, ColPressed), ind(RowToMove + 1, ColPressed), MoveAniDuration, WaitAnimationEnd);
					WasMoved = true;
				}
			if (RowPressed > RowNoTile) //Move tiles up
				for (int RowToMove = RowNoTile + 1; RowToMove <= RowPressed; RowToMove++)
				{
					MoveTile(ind(RowToMove, ColPressed), ind(RowToMove - 1, ColPressed), MoveAniDuration, WaitAnimationEnd);
					WasMoved = true;
				}
		}
	if (! WasMoved)
		for (int Col = 0; Col < Base; Col++)
			if (Tiles[ind(RowPressed, Col)] == NULL)
			{
				int ColNoTile = Col;
				if (ColNoTile > ColPressed) //Move tiles right
					for (int ColToMove = ColNoTile - 1; ColToMove >= ColPressed; ColToMove--)
					{
						MoveTile(ind(RowPressed, ColToMove), ind(RowPressed, ColToMove + 1), MoveAniDuration, WaitAnimationEnd);
						WasMoved = true;
					}
				if (ColPressed > ColNoTile) //Move tiles left
					for (int ColToMove = ColNoTile + 1; ColToMove <= ColPressed; ColToMove++)
					{
						MoveTile(ind(RowPressed, ColToMove), ind(RowPressed, ColToMove - 1), MoveAniDuration, WaitAnimationEnd);
						WasMoved = true;
					}
			}

	return WasMoved;
}

void TForm15Puzzle::MoveTile(int OldPosition, int NewPosition, float MoveAniDuration, bool WaitAnimationEnd)
{
	TTile temp = Tiles[NewPosition];
	Tiles[NewPosition] = Tiles[OldPosition];
	Tiles[OldPosition] = temp;

	AnimateMoveTile(Tiles[NewPosition], MoveAniDuration, WaitAnimationEnd);
};


void TForm15Puzzle::AnimateMoveTile(TTile ATile, float MoveAniDuration, bool WaitAnimationEnd)
{
	uint16_t NewRow = 0, NewCol = 0;
	DivMod(ActualPosition(ATile), Base, NewRow, NewCol);

	QRect geometry = ATile->geometry();
	int X = SpaceX + round(NewCol * (TileSize + TileSpacing));
	int Y = SpaceY + round(NewRow * (TileSize + TileSpacing));

	geometry.setRect(X, Y, TileSize, TileSize);


	if (MoveAniDuration > 0)
	{
		AnimatePropertyDelay(ATile, "geometry", geometry, MoveAniDuration, 0, QEasingCurve::OutExpo, true, WaitAnimationEnd);
	}
	else
	{
//		ATile.Position.X = X;
//		ATile.Position.Y = Y;
		ATile->setGeometry(geometry);
	}
}


void TForm15Puzzle::CheckPuzzleMatched()
{
	bool LPuzzleMatched = true;
	for (uint i = 0; i < Tiles.size(); i++)
		if (Tiles[i] != NULL)
		{
			int TextNumber = Tiles[i]->text().toInt();

			if ((TextNumber - 1) != ActualPosition(Tiles[i]))
			{
				LPuzzleMatched = false;
				break;
			}
		}

	if (LPuzzleMatched && (Mode == Game))
	{
		SetMode(PuzzleMatched);
		AnimatePuzzleMatched();
	}
	if ((! LPuzzleMatched) && ((Mode == PuzzleMatched) || (Mode == JustShuffled)))
	{
		AnimateNormalizeTilesColor();
		if (Mode == PuzzleMatched)
			SetMode(GameOver);
	}
}



void TForm15Puzzle::on_ButtonShuffle_clicked()
{
	AnimateNormalizeTilesColor();

	int NewI = 0;
	int MoveCount = Tiles.size() * Tiles.size();
	float MoveAniDuration = MaxMoveAniDuration;
	for (int i = 1; i <= MoveCount; i++)
	{
		if (i <= 10)
			MoveAniDuration = MinMoveAniDuration + (MaxMoveAniDuration * (1 - (double(i) / 10)));
		if (i >= MoveCount - 10)
			MoveAniDuration = MinMoveAniDuration + ((MaxMoveAniDuration / 2) * (1 - (double((MoveCount - i)) / 10)));
		if ((i > 20) && (i < MoveCount - 20))
			if ((i % 10) == 0)
				MoveAniDuration = MinMoveAniDuration;
			else
				MoveAniDuration = 0;

		bool WasMoved;
		do
		{
			NewI = rand() % Tiles.size();
			WasMoved =  TryMoveTile(NewI, MoveAniDuration, /*WaitAnimationEnd*/true);
		}
		while (! WasMoved);
	}
	SetMaxTime();
//  StopBlinkShuffle();

	SetMode(JustShuffled);
	CheckPuzzleMatched();
}


void TForm15Puzzle::TimerTimeTimer()
{
	uint16_t Min = 0, Sec = 0;
	TimeRemaining = TimeRemaining - 1;
	DivMod(TimeRemaining, 60, Min, Sec);
	ui->TextTime->setText(QString("%1:%2").arg(Min).arg(Sec, 2, 10, QLatin1Char('0')));

	if (TimeRemaining == 0)
	{
		SetMode(GameOver);
		AnimateTimeOver();
//		StartBlinkShuffle();
		return;
	}
	if (TimeRemaining <= 10)
		AnimateTimeRunningOut();

}


void TForm15Puzzle::SetMaxTime()
{
	uint16_t Sec = 0, Min = 0;
	TimeRemaining = ((Base * Base * Base * Base) / 20) * 10;
	DivMod(TimeRemaining, 60, Min, Sec);
	ui->TextTime->setText(QString("%1:%2").arg(Min).arg(Sec, 2, 10, QLatin1Char('0')));
}

// resizeEvent was realized for PanelClient
//void TForm15Puzzle::resizeEvent(QResizeEvent *event)
//{
//		QMainWindow::resizeEvent(event);
//		TimerResize->stop();
//		TimerResize->start();
//}



void TForm15Puzzle::TimerResizeTimer()
{
	TimerResize->stop();
	uint TimeFromLastResize_ms = LastResizeTime.msecsTo(QDateTime::currentDateTime());

//	qDebug() << QString("TimerResizeTimer	") << QDateTime::currentDateTime().toString("mm:ss:zzz");

	if (TimeFromLastResize_ms > 1000)
	{
//			qDebug() << QString("AnimatePlaceTilesFast	") << TimeFromLastResize_ms;
		AnimatePlaceTilesFast();
		LastResizeTime = QDateTime::currentDateTime();
	}

}

void TForm15Puzzle::closeEvent(QCloseEvent *event)
{
	if (! ClosingAnimation)
	{
		AnimateTilesDisappeare();
		ClosingAnimation = true;
	}

}

//-------------------------------   Animations   -----------------------------

void TForm15Puzzle::CalcConsts()
{
	int Height = ui->PanelClient->height();
	int Width = ui->PanelClient->width();

	if (Height > Width)
	{
		SpaceX = round(double(Width) / 20);
		TileSize = round(double((Width - SpaceX * 2)) / Base);
		SpaceY = SpaceX + round(double((Height - Width)) / 2);
	}
	else
	{
		SpaceY = round(double(Height) / 20);
		TileSize = round(double((Height - SpaceY * 2)) / Base);
		SpaceX = SpaceY + round(double((Width - Height)) / 2);
	}
	TileSpacing = round(TileSize * 0.06);
	TileSize = round(TileSize * 0.94);
	SpaceX = SpaceX + round(double(TileSpacing) / 2);
	SpaceY = SpaceY + round(double(TileSpacing) / 2);
}


void TForm15Puzzle::AnimatePlaceTilesFast()
{
	CalcConsts();
	for (uint i = 0; i < Tiles.size(); i++)
		if (Tiles[i] != NULL)
		{
			TTile Tile = Tiles[i];
			uint16_t Row = 0, Col = 0;
			DivMod(i, Base, Row, Col);

			QRect geometry = Tile->geometry();
			int X = SpaceX + round(Col * (TileSize + TileSpacing));
			int Y = SpaceY + round(Row * (TileSize + TileSpacing));

			geometry.setRect(X, Y, TileSize, TileSize);

			AnimatePropertyDelay(Tile, "geometry", geometry, 200, (0 + 30 * i), QEasingCurve::OutExpo);

//			AnimatePropertyDelay(Tile->font(), "pixelSize", geometry, 200, (0 + 30 * i), QEasingCurve::OutExpo);

			int FontSize = round(TileSize / 3.0);
#ifdef Q_OS_ANDROID
			FontSize = round(TileSize / 4.0);
#endif
			QFont font = Tile->font();
			font.setPointSize(FontSize);
			Tile->setFont(font);

			TileBorderThickness = round(TileSize / 16.0);

			TGradientAnimation *GradientAni = (TGradientAnimation*)Tile->property("GradientAni").value<void *>() ;
			Tile->setStyleSheet(GenerateTileStyleSheet(GradientAni->CurColor1, GradientAni->CurColor2));
		}
}


void TForm15Puzzle::AnimateTilesDisappeare()
{
//	qDebug() << QString("AnimateTilesDisappeare  ") << QDateTime::currentDateTime().toString("mm:ss:zzz");
	QList<QPropertyAnimation*> AniList;

	int TileIndex = 0; // Index from not NULL, because Tiles[0] can be NULL, and then all animations will have delay and Wait end of all animations fails
	for (uint i = 0; i < Tiles.size(); i++)
		if (Tiles[i] != NULL)
		{
			TTile Tile = Tiles[i];
			QRect geometry = Tile->geometry();

			int X = geometry.x()  + round(double(TileSize) / 2);
			int Y = geometry.y() + TileSize;

			geometry.setRect(X, Y, 0, 0);

			auto ani = AnimatePropertyDelay(Tile, "geometry", geometry, 400, (0 + 30 * TileIndex), QEasingCurve::InBack, false);

			AniList.append(ani);
			TileIndex++;
		}

//Wait end of all animations;
	bool SomeAniRunning;
	while(true)
	{
		SomeAniRunning = false;
		for (int i = 0; i < AniList.size(); i++)
			if (AniList.at(i)->state() == QPropertyAnimation::Running)
				SomeAniRunning = true;

		if (SomeAniRunning)
		{
			qApp->processEvents();
			QThread::msleep(1);
		}
		else
			break;

	}

  for (int i = 0; i < AniList.size(); i++)
		delete AniList.at(i);

}


void TForm15Puzzle::AnimateBaseNotChanged()
{
//  int i = 0;
//  for (int stop = Tiles.Length - 1, i = 0; i <= stop; i++)
//    if (Tiles[i] != NULL)
//    {
//      TAnimator.AnimateFloatDelay(Tiles[i], "RotationAngle", - 20, 0.1 * slowdown, 0 * slowdown, TAnimationType.InOut, TInterpolationType.Linear);
//      TAnimator.AnimateFloatDelay(Tiles[i], "RotationAngle", 20, 0.25 * slowdown, 0.1 * slowdown, TAnimationType.InOut, TInterpolationType.Exponential);
//      TAnimator.AnimateFloatDelay(Tiles[i], "RotationAngle", 0, 0.25 * slowdown, 0.35 * slowdown, TAnimationType.Out, TInterpolationType.Back);
//    }
	for (uint i = 0; i < Tiles.size(); i++)
		if (Tiles[i] != NULL)
		{
			TTile Tile = Tiles[i];
			QRect OrigGeometry = Tile->geometry();
			QRect geometry = OrigGeometry;

			int offset = round(double(TileSize) / 4);
			int size = round(double(TileSize) / 2);

			int X = geometry.x()  + offset;
			int Y = geometry.y() + offset;

			geometry.setRect(X, Y, size, size);
			AnimatePropertyDelay(Tile, "geometry", geometry, 300, (0 + 30 * i), QEasingCurve::InBack);
			AnimatePropertyDelay(Tile, "geometry", OrigGeometry, 300, (350 + 30 * i), QEasingCurve::OutBack);
		}

}



void TForm15Puzzle::AnimatePrepareBeforePlace()
{
	CalcConsts();

	for (uint i = 0; i < Tiles.size(); i++)
		if (Tiles[i] != NULL)
		{
			TTile Tile = Tiles[i];
			uint16_t Row = 0, Col = 0;
			DivMod(i, Base, Row, Col);

			QRect geometry = Tile->geometry();
			int X = SpaceX + round(Col * (TileSize + TileSpacing));
			int Y = SpaceY + round(Row * (TileSize + TileSpacing));

			X = X  + round(double(TileSize) / 2);
			Y = Y + TileSize;

			geometry.setRect(X, Y, 0, 0);


			Tile->setGeometry(geometry);
			Tile->setStyleSheet(GenerateTileStyleSheet(TileFillNormalColor1, TileFillNormalColor2));
			Tile->show();
		}
}




void TForm15Puzzle::AnimateTimeRunningOut()
{
	for (uint i = 0; i < Tiles.size(); i++)
		if (Tiles[i] != NULL)
		{
			TTile Tile = Tiles[i];
			TGradientAnimation *GradientAni = (TGradientAnimation*)Tile->property("GradientAni").value<void *>() ;
			GradientAni->StopColor1 = TileFillNormalColor1;
			GradientAni->StopColor2 = QColor("darkorange");
			GradientAni->Delay_ms = 0;
			GradientAni->Duration_ms = 150;
			GradientAni->AutoReverse = true;
			GradientAni->Start();
		}
}


void TForm15Puzzle::AnimateTimeOver()
{
	for (uint i = 0; i < Tiles.size(); i++)
		if (Tiles[i] != NULL)
		{
			TTile Tile = Tiles[i];
			TGradientAnimation *GradientAni = (TGradientAnimation*)Tile->property("GradientAni").value<void *>() ;
			GradientAni->Stop();
			GradientAni->StopColor1 = Qt::gray;
			GradientAni->StopColor2 = Qt::red;
			GradientAni->Delay_ms = 0;
			GradientAni->Duration_ms = 600;
			GradientAni->AutoReverse = false;
			GradientAni->Start();
		}
}


void TForm15Puzzle::AnimateNormalizeTilesColor()
{
	for (uint i = 0; i < Tiles.size(); i++)
		if (Tiles[i] != NULL)
		{
			TTile Tile = Tiles[i];
			TGradientAnimation *GradientAni = (TGradientAnimation*)Tile->property("GradientAni").value<void *>() ;
			GradientAni->StopColor1 = TileFillNormalColor1;
			GradientAni->StopColor2 = TileFillNormalColor2;
			GradientAni->Delay_ms = 0;
			GradientAni->Duration_ms = 500;
			GradientAni->AutoReverse = false;
			GradientAni->Start();
		}
}




void TForm15Puzzle::AnimatePuzzleMatched()
{
	for (uint i = 0; i < Tiles.size(); i++)
		if (Tiles[i] != NULL)
		{
			TTile Tile = Tiles[i];
//			TAnimator.AnimateFloatDelay(Tile, "RotationAngle", 360, 1 * slowdown, 0.35 * slowdown, TAnimationType.Out, TInterpolationType.Back);
			TGradientAnimation *GradientAni = (TGradientAnimation*)Tile->property("GradientAni").value<void *>() ;
			GradientAni->Stop();
			GradientAni->StopColor1 = QColor("lawngreen");
			GradientAni->StopColor2 = QColor("gold");
			GradientAni->Delay_ms = (1 + i * 100);
			GradientAni->Duration_ms = 500 ;
			GradientAni->AutoReverse = false;
			GradientAni->Start();
		}
}

//-------------------------------  Test different Animations   -----------------------------

void TForm15Puzzle::on_ButtonPlace_clicked()
{
	AnimateNormalizeTilesColor();
	AnimatePlaceTilesFast();
}

void TForm15Puzzle::on_ButtonDisappeare_clicked()
{
	AnimateTilesDisappeare();
}



void TForm15Puzzle::on_ButtonPuzzleMatched_clicked()
{
	AnimatePuzzleMatched();
}

void TForm15Puzzle::on_ButtonTimeRunningOut_clicked()
{
	AnimateTimeRunningOut();
}

void TForm15Puzzle::on_ButtonTimeOver_clicked()
{
	AnimateTimeOver();
}


//---------------------------  Realization of Property Animation   -----------------------------


QPropertyAnimation* AnimatePropertyDelay(QObject * const Target, const QByteArray &PropertyName,
				  const QVariant &Value, uint Duration_ms, uint Delay_ms,
				  QEasingCurve AInterpolation, bool DeleteWhenStopped, bool WaitAnimationEnd)
{
	QPropertyAnimation *ani = new QPropertyAnimation(Target, PropertyName);
	ani->setDuration(Duration_ms);

	ani->setEasingCurve(AInterpolation);

	ani->setEndValue(Value);

//	QObject::connect(ani, &QPropertyAnimation::finished, [=](){
//		if (DeleteWhenStopped)
//				delete ani;
//	});
	if (DeleteWhenStopped)
		QObject::connect(ani, &QPropertyAnimation::finished, ani, &QPropertyAnimation::deleteLater);


	if (Delay_ms == 0)
		ani->start();
	else
		QTimer::singleShot(Delay_ms, ani, SLOT(start()));


	if (WaitAnimationEnd)
		while (ani->state() == QPropertyAnimation::Running)
		{
			qApp->processEvents();
			QThread::msleep(1);

		}

	return ani;
}

//-----------------------------   Realization of Gradient Animation    -----------------------------

QColor InterpolateColor(const QColor &Start, const QColor &Stop, double T)
{
	QColor Result;
	Result.setAlpha( Start.alpha() + ((Stop.alpha()   - Start.alpha()) * T));
	Result.setRed(   Start.red()   + ((Stop.red()     - Start.red()  ) * T));
	Result.setGreen( Start.green() + ((Stop.green()   - Start.green()) * T));
	Result.setBlue(  Start.blue()  + ((Stop.blue()    - Start.blue() ) * T));
	return Result;
}



TGradientAnimation::TGradientAnimation(QWidget *ATarget, TGenerateStyleSheetFunc AGenerateStyleSheetFunc)
{
	Target = ATarget;
	GenerateStyleSheetFunc = AGenerateStyleSheetFunc;
	timeLine = new QTimeLine(Duration_ms, Target);
  
	int FrameRange = 1023;
	timeLine->setFrameRange(0, FrameRange);
  
	QObject::connect(timeLine, &QTimeLine::frameChanged, [=](int frame){
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

		Target->setStyleSheet(GenerateStyleSheetFunc(CurColor1, CurColor2));
	});
}

TGradientAnimation::~TGradientAnimation()
{
//	QObject::connect(timeLine, &QTimeLine::finished, timeLine, &QTimeLine::deleteLater);
	timeLine->stop();
	delete timeLine;
}

void TGradientAnimation::Stop()
{
	timeLine->stop();
}

void TGradientAnimation::SetCurColors(QColor ACurColor1, QColor ACurColor2)
{
	CurColor1 = ACurColor1;
	CurColor2 = ACurColor2;
}

void TGradientAnimation::Start()
{
	timeLine->setEasingCurve(AInterpolation);

	StartColor1 = CurColor1;
	StartColor2 = CurColor2;

	if (AutoReverse)
		timeLine->setDuration(Duration_ms * 2);
	else
		timeLine->setDuration(Duration_ms);



	if (Delay_ms == 0)
		timeLine->start();
	else
		QTimer::singleShot(Delay_ms, timeLine, SLOT(start()));
}


//void TForm15Puzzle::on_ChangeBackground_clicked()
//{
//TGradientAnimation *GradientAni = (TGradientAnimation*)Tile->property("GradientAni").value<void *>() ;

//	GradientAni->StopColor1 = QColor("olive");;
//	GradientAni->StopColor2 = QColor("darkorange");
//	GradientAni->Delay_ms = 0;
//	GradientAni->Duration_ms = 500;
//	GradientAni->AutoReverse = true;
//	GradientAni->Start();
//}

//-----------------------------   -----------------------------	-----------------------------


void TForm15Puzzle::on_ButtonScaleForAndroid_clicked()
{
	QFont font = qApp->font();
	font.setPointSize(12);
	qApp->setFont(font);

	int ButtonSize =  ui->PanelClient->width() / 8;

	ui->Button3x3->setMaximumSize(ButtonSize, 16777215);
	ui->Button4x4->setMaximumSize(ButtonSize, 16777215);
	ui->Button5x5->setMaximumSize(ButtonSize, 16777215);

	ui->TextTime->setMinimumSize(ButtonSize * 1.3, 16777215);

	font = ui->TextTime->font();
	font.setPointSize(17);
	ui->TextTime->setFont(font);
}


void TForm15Puzzle::on_PanelClient_clicked()
{
	uint TimeFromLastTap_ms = LastTapTime.msecsTo(QDateTime::currentDateTime());

	if (not (LastTapTime.isNull()) and (TimeFromLastTap_ms < 300))
	  ShowDebug();

	LastTapTime = QDateTime::currentDateTime();
}

void TForm15Puzzle::ShowDebug()
{
	if (not ui->PanelDebug->isVisible())
	{
	  PanelDebugMaximumHeight = ui->TextTime->height() * 2;
	  ui->PanelDebug->setMaximumHeight(0);
	  ui->PanelDebug->setVisible(true);
	};


	if (ui->PanelDebug->maximumHeight() < 10)
		AnimatePropertyDelay(ui->PanelDebug, "maximumHeight", PanelDebugMaximumHeight, 400, 0, QEasingCurve::OutBounce);
	else
		AnimatePropertyDelay(ui->PanelDebug, "maximumHeight", 0, 400, 0, QEasingCurve::InBack);

}

bool TMyEventFilter::eventFilter(QObject *obj, QEvent *event)
{
//	qDebug("event type %d", event->type());
	if ((event->type() == QEvent::MouseButtonPress) or
		(event->type() == QEvent::MouseButtonDblClick)) {
		parent->on_PanelClient_clicked();
		return true;
	} else if (event->type() == QEvent::Resize) {
		parent->TimerResize->stop();
		parent->TimerResize->start();
//		qDebug() << QString("event Resize  ") << QDateTime::currentDateTime().toString("mm:ss:zzz");

		return true;
	}
	else {
		// standard event processing
		return QObject::eventFilter(obj, event);
	}
	return false;
}




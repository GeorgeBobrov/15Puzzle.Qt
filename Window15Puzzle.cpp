#include "Window15Puzzle.h"
#include "ui_Window15Puzzle.h"


const double maxMoveAniDuration = 150;
const double minMoveAniDuration = 1;

int tileBorderThickness = 4;

QString generateTileStyleSheet(const QColor &color1, const QColor &color2)
{
	return QString("border: " + QString::number(tileBorderThickness) + "px solid #FFE55555;\n"
	"border-radius: " + QString::number(tileBorderThickness) + "px;\n"
	"background-color: qlineargradient(spread:pad, x1:0.35, y1:0.35, x2:0.9, y2:0.9, "
	"stop:0 ") + color1.name() + ", stop:1 " + color2.name() + ");";
}


Form15Puzzle::Form15Puzzle(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	MyEventFilter *myEventFilter = new MyEventFilter(this);
	ui->panelClient->installEventFilter(myEventFilter);

	timerTime = new QTimer();
	connect(timerTime, SIGNAL(timeout()), this, SLOT(timerTimeTimer()));
	timerTime->setInterval(1000);

	timerResize = new QTimer();
	connect(timerResize, SIGNAL(timeout()), this, SLOT(timerResizeTimer()));
	timerResize->setInterval(200);

	lastResizeTime = QDateTime::currentDateTime();   //To prevent resize on start on Android
//	qDebug() << QString("FormCreate  ") << QDateTime::currentDateTime().toString("mm:ss:zzz");

	tileFillNormalColor1 = QColor("bisque");
	tileFillNormalColor2 = QColor("#FFABE024");

//  panelClient  background-color: rgb(244, 244, 244);

	srand(unsigned(time(0)));
	setBase(4);

	panelDebugMaximumHeight = ui->panelDebug->height();
//	ui->panelDebug->setMaximumHeight(0);

	ui->panelDebug->setVisible(false);
}

Form15Puzzle::~Form15Puzzle()
{
	delete ui;
}


void  Form15Puzzle::setMode(const Mode value)
{
	mode = value;
	if (mode == Mode::Game)
		timerTime->start();
	else
		timerTime->stop();
}



void Form15Puzzle::on_button3x3_clicked()
{
	QPushButton* sender = (QPushButton*)QObject::sender();
	int LBase = QString(sender->text().at(0)).toInt();
	setBase(LBase);
}

void Form15Puzzle::on_button4x4_clicked()
{
	on_button3x3_clicked();
}

void Form15Puzzle::on_button5x5_clicked()
{
	on_button3x3_clicked();
}



void Form15Puzzle::setBase(const int value)
{
	if (value == base)
	{
		animateBaseNotChanged();
		return;
	}
	setMode(Mode::GameOver);
	animateTilesDisappeare();
	base = value;
	setMaxTime();

	QTimer::singleShot(100, this, SLOT(timerCreateTilesTimer()));
}



void Form15Puzzle::timerCreateTilesTimer()
{
#ifdef Q_OS_ANDROID
	on_buttonScaleForAndroid_clicked();
#endif

	createTiles();
	animatePrepareBeforePlace();
	animatePlaceTilesFast();
}



void Form15Puzzle::createTiles()
{
	for (uint i = 0; i < tiles.size(); i++)
		if (tiles[i] != NULL)
		{
			GradientAnimation *gradientAni = (GradientAnimation*)tiles[i]->property("gradientAni").value<void *>() ;
			delete gradientAni;
			delete tiles[i];
			tiles[i] = NULL;
		}

	tiles.resize(base * base);
	for (uint i = 0; i < tiles.size() - 1; i++)
		if (tiles[i] == NULL)
		{
			Tile newTile;

			newTile = new QPushButton(this);

			connect(newTile, SIGNAL(pressed()), this, SLOT(on_tile1_pressed()));

			newTile->setText(QString::number(i + 1));

			GradientAnimation *gradientAni = new GradientAnimation(newTile, generateTileStyleSheet);
			newTile->setProperty("gradientAni", qVariantFromValue((void*) gradientAni));

			newTile->setStyleSheet(generateTileStyleSheet(tileFillNormalColor1, tileFillNormalColor2));
			gradientAni->setCurColors(tileFillNormalColor1, tileFillNormalColor2);

			newTile->setParent(ui->panelClient);
//			newTile.SendToBack;
			tiles[i] = newTile;
		}

	if (tiles[tiles.size() - 1] != NULL)
		tiles[tiles.size() - 1] = NULL;
}


int Form15Puzzle::ind(int row, int col)
{
	return row * base + col;
}


int Form15Puzzle::actualPosition(Tile tile)
{
	for (uint i = 0; i < tiles.size(); i++)
		if (tiles[i] == tile)
			return i;
	return 0;
}


void Form15Puzzle::on_tile1_pressed()
{
	Tile senderTile = (Tile) QObject::sender();
	if (mode == Mode::JustShuffled)
		setMode(Mode::Game);
	bool wasMoved = tryMoveTile(actualPosition(senderTile), maxMoveAniDuration, false);
	if (wasMoved)
		checkPuzzleMatched();
}


bool Form15Puzzle::tryMoveTile(int tilePosition, float moveAniDuration, bool waitAnimationEnd)
{
	bool wasMoved = false;
	
	int colPressed = tilePosition % base;
	int rowPressed = tilePosition / base;

	for (int row = 0; row < base; row++)
		if (tiles[ind(row, colPressed)] == NULL)
		{
			int rowNoTile = row;
			if (rowNoTile > rowPressed) //Move tiles down
				for (int rowToMove = rowNoTile - 1; rowToMove >= rowPressed; rowToMove--)
				{
					moveTile(ind(rowToMove, colPressed), ind(rowToMove + 1, colPressed), moveAniDuration, waitAnimationEnd);
					wasMoved = true;
				}
			if (rowPressed > rowNoTile) //Move tiles up
				for (int rowToMove = rowNoTile + 1; rowToMove <= rowPressed; rowToMove++)
				{
					moveTile(ind(rowToMove, colPressed), ind(rowToMove - 1, colPressed), moveAniDuration, waitAnimationEnd);
					wasMoved = true;
				}
		}
	if (! wasMoved)
		for (int col = 0; col < base; col++)
			if (tiles[ind(rowPressed, col)] == NULL)
			{
				int colNoTile = col;
				if (colNoTile > colPressed) //Move tiles right
					for (int colToMove = colNoTile - 1; colToMove >= colPressed; colToMove--)
					{
						moveTile(ind(rowPressed, colToMove), ind(rowPressed, colToMove + 1), moveAniDuration, waitAnimationEnd);
						wasMoved = true;
					}
				if (colPressed > colNoTile) //Move tiles left
					for (int colToMove = colNoTile + 1; colToMove <= colPressed; colToMove++)
					{
						moveTile(ind(rowPressed, colToMove), ind(rowPressed, colToMove - 1), moveAniDuration, waitAnimationEnd);
						wasMoved = true;
					}
			}

	return wasMoved;
}

void Form15Puzzle::moveTile(int oldPosition, int newPosition, float moveAniDuration, bool waitAnimationEnd)
{
	Tile temp = tiles[newPosition];
	tiles[newPosition] = tiles[oldPosition];
	tiles[oldPosition] = temp;

	animateMoveTile(tiles[newPosition], moveAniDuration, waitAnimationEnd);
};


void Form15Puzzle::animateMoveTile(Tile tile, float moveAniDuration, bool waitAnimationEnd)
{
	int ActPos = actualPosition(tile);
	int newCol = ActPos % base;
	int newRow = ActPos / base;

	QRect geometry = tile->geometry();
	int x = spaceX + round(newCol * (tileSize + tileSpacing));
	int y = spaceY + round(newRow * (tileSize + tileSpacing));

	geometry.setRect(x, y, tileSize, tileSize);


	if (moveAniDuration > 0)
	{
		animatePropertyDelay(tile, "geometry", geometry, moveAniDuration, 0, QEasingCurve::OutExpo, true, waitAnimationEnd);
	}
	else
	{
//		tile.Position.X = X;
//		tile.Position.Y = Y;
		tile->setGeometry(geometry);
	}
}


void Form15Puzzle::checkPuzzleMatched()
{
	bool puzzleMatched = true;
	for (uint i = 0; i < tiles.size(); i++)
		if (tiles[i] != NULL)
		{
			int TextNumber = tiles[i]->text().toInt();

			if ((TextNumber - 1) != actualPosition(tiles[i]))
			{
				puzzleMatched = false;
				break;
			}
		}

	if (puzzleMatched && (mode == Mode::Game))
	{
		setMode(Mode::PuzzleMatched);
		animatePuzzleMatched();
	}
	if ((! puzzleMatched) && ((mode == Mode::PuzzleMatched) || (mode == Mode::JustShuffled)))
	{
		animateNormalizeTilesColor();
		if (mode == Mode::PuzzleMatched)
			setMode(Mode::GameOver);
	}
}



void Form15Puzzle::on_buttonShuffle_clicked()
{
	animateNormalizeTilesColor();

	int newI = 0;
	int moveCount = tiles.size() * tiles.size();
	float moveAniDuration = maxMoveAniDuration;
	for (int i = 1; i <= moveCount; i++)
	{
		if (i <= 10)
			moveAniDuration = minMoveAniDuration + (maxMoveAniDuration * (1 - (double(i) / 10)));
		if (i >= moveCount - 10)
			moveAniDuration = minMoveAniDuration + ((maxMoveAniDuration / 2) * (1 - (double((moveCount - i)) / 10)));
		if ((i > 20) && (i < moveCount - 20))
			if ((i % 10) == 0)
				moveAniDuration = minMoveAniDuration;
			else
				moveAniDuration = 0;

		bool wasMoved;
		do
		{
			newI = rand() % tiles.size();
			wasMoved =  tryMoveTile(newI, moveAniDuration, /*waitAnimationEnd*/true);
		}
		while (! wasMoved);
	}
	setMaxTime();
//  stopBlinkShuffle();

	setMode(Mode::JustShuffled);
	checkPuzzleMatched();
}


void Form15Puzzle::timerTimeTimer()
{
	timeRemaining = timeRemaining - 1;

	int sec = timeRemaining % 60;
	int min = timeRemaining / 60;

	ui->textTime->setText(QString("%1:%2").arg(min).arg(sec, 2, 10, QLatin1Char('0')));

	if (timeRemaining == 0)
	{
		setMode(Mode::GameOver);
		animateTimeOver();
//		startBlinkShuffle();
		return;
	}
	if (timeRemaining <= 10)
		animateTimeRunningOut();

}


void Form15Puzzle::setMaxTime()
{
	timeRemaining = ((base * base * base * base) / 20) * 10;
	int sec = timeRemaining % 60;
	int min = timeRemaining / 60;
	ui->textTime->setText(QString("%1:%2").arg(min).arg(sec, 2, 10, QLatin1Char('0')));
}

// resizeEvent was realized for panelClient
//void Form15Puzzle::resizeEvent(QResizeEvent *event)
//{
//		QMainWindow::resizeEvent(event);
//		timerResize->stop();
//		timerResize->start();
//}



void Form15Puzzle::timerResizeTimer()
{
	timerResize->stop();
	uint timeFromLastResize_ms = lastResizeTime.msecsTo(QDateTime::currentDateTime());

//	qDebug() << QString("timerResizeTimer	") << QDateTime::currentDateTime().toString("mm:ss:zzz");

	if (timeFromLastResize_ms > 1000)
	{
//			qDebug() << QString("AnimatePlaceTilesFast	") << timeFromLastResize_ms;
		animatePlaceTilesFast();
		lastResizeTime = QDateTime::currentDateTime();
	}

}

void Form15Puzzle::closeEvent(QCloseEvent *event)
{
	if (! closingAnimation)
	{
		animateTilesDisappeare();
		closingAnimation = true;
	}

}

//-------------------------------   Animations   -----------------------------

void Form15Puzzle::calcConsts()
{
	int height = ui->panelClient->height();
	int width = ui->panelClient->width();

	if (height > width)
	{
		spaceX = round(double(width) / 20);
		tileSize = round(double((width - spaceX * 2)) / base);
		spaceY = spaceX + round(double((height - width)) / 2);
	}
	else
	{
		spaceY = round(double(height) / 20);
		tileSize = round(double((height - spaceY * 2)) / base);
		spaceX = spaceY + round(double((width - height)) / 2);
	}
	tileSpacing = round(tileSize * 0.06);
	tileSize = round(tileSize * 0.94);
	spaceX = spaceX + round(double(tileSpacing) / 2);
	spaceY = spaceY + round(double(tileSpacing) / 2);
}


void Form15Puzzle::animatePlaceTilesFast()
{
	calcConsts();
	for (uint i = 0; i < tiles.size(); i++)
		if (tiles[i] != NULL)
		{
			Tile tile = tiles[i];
			int col = i % base;
			int row = i / base;

			QRect geometry = tile->geometry();
			int x = spaceX + round(col * (tileSize + tileSpacing));
			int y = spaceY + round(row * (tileSize + tileSpacing));

			geometry.setRect(x, y, tileSize, tileSize);

			animatePropertyDelay(tile, "geometry", geometry, 200, (0 + 30 * i), QEasingCurve::OutExpo);

//			animatePropertyDelay(tile->font(), "pixelSize", geometry, 200, (0 + 30 * i), QEasingCurve::OutExpo);

			int fontSize = round(tileSize / 3.0);
#ifdef Q_OS_ANDROID
			fontSize = round(tileSize / 4.0);
#endif
			QFont font = tile->font();
			font.setPointSize(fontSize);
			tile->setFont(font);

			tileBorderThickness = round(tileSize / 16.0);

			GradientAnimation *gradientAni = (GradientAnimation*)tile->property("gradientAni").value<void *>() ;
			tile->setStyleSheet(generateTileStyleSheet(gradientAni->curColor1, gradientAni->curColor2));
		}
}


void Form15Puzzle::animateTilesDisappeare()
{
//	qDebug() << QString("AnimateTilesDisappeare  ") << QDateTime::currentDateTime().toString("mm:ss:zzz");
	QList<QPropertyAnimation*> aniList;

	int tileIndex = 0; // Index from not NULL, because tiles[0] can be NULL, and then all animations will have delay and Wait end of all animations fails
	for (uint i = 0; i < tiles.size(); i++)
		if (tiles[i] != NULL)
		{
			Tile tile = tiles[i];
			QRect geometry = tile->geometry();

			int x = geometry.x()  + round(double(tileSize) / 2);
			int y = geometry.y() + tileSize;

			geometry.setRect(x, y, 0, 0);

			auto ani = animatePropertyDelay(tile, "geometry", geometry, 400, (0 + 30 * tileIndex), QEasingCurve::InBack, false);

			aniList.append(ani);
			tileIndex++;
		}

//Wait end of all animations;
	bool someAniRunning;
	while(true)
	{
		someAniRunning = false;
		for (int i = 0; i < aniList.size(); i++)
			if (aniList.at(i)->state() == QPropertyAnimation::Running)
				someAniRunning = true;

		if (someAniRunning)
		{
			qApp->processEvents();
			QThread::msleep(1);
		}
		else
			break;

	}

  for (int i = 0; i < aniList.size(); i++)
		delete aniList.at(i);

}


void Form15Puzzle::animateBaseNotChanged()
{
//  int i = 0;
//  for (int stop = tiles.Length - 1, i = 0; i <= stop; i++)
//    if (tiles[i] != NULL)
//    {
//      TAnimator.AnimateFloatDelay(tiles[i], "RotationAngle", - 20, 0.1 * slowdown, 0 * slowdown, TAnimationType.InOut, TInterpolationType.Linear);
//      TAnimator.AnimateFloatDelay(tiles[i], "RotationAngle", 20, 0.25 * slowdown, 0.1 * slowdown, TAnimationType.InOut, TInterpolationType.Exponential);
//      TAnimator.AnimateFloatDelay(tiles[i], "RotationAngle", 0, 0.25 * slowdown, 0.35 * slowdown, TAnimationType.Out, TInterpolationType.Back);
//    }
	for (uint i = 0; i < tiles.size(); i++)
		if (tiles[i] != NULL)
		{
			Tile tile = tiles[i];
			QRect origGeometry = tile->geometry();
			QRect geometry = origGeometry;

			int offset = round(double(tileSize) / 4);
			int size = round(double(tileSize) / 2);

			int x = geometry.x()  + offset;
			int y = geometry.y() + offset;

			geometry.setRect(x, y, size, size);
			animatePropertyDelay(tile, "geometry", geometry, 300, (0 + 30 * i), QEasingCurve::InBack);
			animatePropertyDelay(tile, "geometry", origGeometry, 300, (350 + 30 * i), QEasingCurve::OutBack);
		}

}



void Form15Puzzle::animatePrepareBeforePlace()
{
	calcConsts();

	for (uint i = 0; i < tiles.size(); i++)
		if (tiles[i] != NULL)
		{
			Tile tile = tiles[i];
			int col = i % base;
			int row = i / base;

			QRect geometry = tile->geometry();
			int x = spaceX + round(col * (tileSize + tileSpacing));
			int y = spaceY + round(row * (tileSize + tileSpacing));

			x = x  + round(double(tileSize) / 2);
			y = y + tileSize;

			geometry.setRect(x, y, 0, 0);


			tile->setGeometry(geometry);
			tile->setStyleSheet(generateTileStyleSheet(tileFillNormalColor1, tileFillNormalColor2));
			tile->show();
		}
}




void Form15Puzzle::animateTimeRunningOut()
{
	for (uint i = 0; i < tiles.size(); i++)
		if (tiles[i] != NULL)
		{
			Tile tile = tiles[i];
			GradientAnimation *gradientAni = (GradientAnimation*)tile->property("gradientAni").value<void *>() ;
			gradientAni->stopColor1 = tileFillNormalColor1;
			gradientAni->stopColor2 = QColor("darkorange");
			gradientAni->delay_ms = 0;
			gradientAni->duration_ms = 150;
			gradientAni->autoReverse = true;
			gradientAni->start();
		}
}


void Form15Puzzle::animateTimeOver()
{
	for (uint i = 0; i < tiles.size(); i++)
		if (tiles[i] != NULL)
		{
			Tile tile = tiles[i];
			GradientAnimation *gradientAni = (GradientAnimation*)tile->property("gradientAni").value<void *>() ;
			gradientAni->stop();
			gradientAni->stopColor1 = Qt::gray;
			gradientAni->stopColor2 = Qt::red;
			gradientAni->delay_ms = 0;
			gradientAni->duration_ms = 600;
			gradientAni->autoReverse = false;
			gradientAni->start();
		}
}


void Form15Puzzle::animateNormalizeTilesColor()
{
	for (uint i = 0; i < tiles.size(); i++)
		if (tiles[i] != NULL)
		{
			Tile tile = tiles[i];
			GradientAnimation *gradientAni = (GradientAnimation*)tile->property("gradientAni").value<void *>() ;
			gradientAni->stopColor1 = tileFillNormalColor1;
			gradientAni->stopColor2 = tileFillNormalColor2;
			gradientAni->delay_ms = 0;
			gradientAni->duration_ms = 500;
			gradientAni->autoReverse = false;
			gradientAni->start();
		}
}




void Form15Puzzle::animatePuzzleMatched()
{
	for (uint i = 0; i < tiles.size(); i++)
		if (tiles[i] != NULL)
		{
			Tile tile = tiles[i];
//			TAnimator.AnimateFloatDelay(tile, "RotationAngle", 360, 1 * slowdown, 0.35 * slowdown, TAnimationType.Out, TInterpolationType.Back);
			GradientAnimation *gradientAni = (GradientAnimation*)tile->property("gradientAni").value<void *>() ;
			gradientAni->stop();
			gradientAni->stopColor1 = QColor("lawngreen");
			gradientAni->stopColor2 = QColor("gold");
			gradientAni->delay_ms = (1 + i * 100);
			gradientAni->duration_ms = 500 ;
			gradientAni->autoReverse = false;
			gradientAni->start();
		}
}

//-------------------------------  Test different Animations   -----------------------------

void Form15Puzzle::on_buttonPlace_clicked()
{
	animateNormalizeTilesColor();
	animatePlaceTilesFast();
}

void Form15Puzzle::on_buttonDisappeare_clicked()
{
	animateTilesDisappeare();
}



void Form15Puzzle::on_buttonPuzzleMatched_clicked()
{
	animatePuzzleMatched();
}

void Form15Puzzle::on_buttonTimeRunningOut_clicked()
{
	animateTimeRunningOut();
}

void Form15Puzzle::on_buttonTimeOver_clicked()
{
	animateTimeOver();
}


//---------------------------  Realization of Property Animation   -----------------------------


QPropertyAnimation* animatePropertyDelay(QObject * const target, const QByteArray &propertyName,
				  const QVariant &value, uint duration_ms, uint delay_ms,
				  QEasingCurve interpolation, bool deleteWhenStopped, bool waitAnimationEnd)
{
	QPropertyAnimation *ani = new QPropertyAnimation(target, propertyName);
	ani->setDuration(duration_ms);

	ani->setEasingCurve(interpolation);

	ani->setEndValue(value);

//	QObject::connect(ani, &QPropertyAnimation::finished, [=](){
//		if (deleteWhenStopped)
//				delete ani;
//	});
	if (deleteWhenStopped)
		QObject::connect(ani, &QPropertyAnimation::finished, ani, &QPropertyAnimation::deleteLater);


	if (delay_ms == 0)
		ani->start();
	else
		QTimer::singleShot(delay_ms, ani, SLOT(start()));


	if (waitAnimationEnd)
		while (ani->state() == QPropertyAnimation::Running)
		{
			qApp->processEvents();
			QThread::msleep(1);

		}

	return ani;
}

//-----------------------------   Realization of Gradient Animation    -----------------------------

QColor interpolateColor(const QColor &start, const QColor &stop, double T)
{
	QColor result;
	result.setAlpha( start.alpha() + ((stop.alpha()   - start.alpha()) * T));
	result.setRed(   start.red()   + ((stop.red()     - start.red()  ) * T));
	result.setGreen( start.green() + ((stop.green()   - start.green()) * T));
	result.setBlue(  start.blue()  + ((stop.blue()    - start.blue() ) * T));
	return result;
}



GradientAnimation::GradientAnimation(QWidget *aTarget, TGenerateStyleSheetFunc AGenerateStyleSheetFunc)
{
	target = aTarget;
	generateStyleSheetFunc = AGenerateStyleSheetFunc;
	timeLine = new QTimeLine(duration_ms, target);
  
	int frameRange = 1023;
	timeLine->setFrameRange(0, frameRange);
  
	QObject::connect(timeLine, &QTimeLine::frameChanged, [=](int frame){
		double normalizedTime = double(frame) / frameRange;
  
		if (autoReverse)
		{
			if (normalizedTime < 0.5)
				normalizedTime = normalizedTime * 2;
			else
				normalizedTime = (1 - normalizedTime) * 2;
		}
  
		curColor1 = interpolateColor(startColor1, stopColor1, normalizedTime);
		curColor2 = interpolateColor(startColor2, stopColor2, normalizedTime);

		target->setStyleSheet(generateStyleSheetFunc(curColor1, curColor2));
	});
}

GradientAnimation::~GradientAnimation()
{
//	QObject::connect(timeLine, &QTimeLine::finished, timeLine, &QTimeLine::deleteLater);
	timeLine->stop();
	delete timeLine;
}

void GradientAnimation::stop()
{
	timeLine->stop();
}

void GradientAnimation::setCurColors(QColor aCurColor1, QColor aCurColor2)
{
	curColor1 = aCurColor1;
	curColor2 = aCurColor2;
}

void GradientAnimation::start()
{
	timeLine->setEasingCurve(interpolation);

	startColor1 = curColor1;
	startColor2 = curColor2;

	if (autoReverse)
		timeLine->setDuration(duration_ms * 2);
	else
		timeLine->setDuration(duration_ms);



	if (delay_ms == 0)
		timeLine->start();
	else
		QTimer::singleShot(delay_ms, timeLine, SLOT(start()));
}


//void Form15Puzzle::on_ChangeBackground_clicked()
//{
//GradientAnimation *gradientAni = (GradientAnimation*)tile->property("gradientAni").value<void *>() ;

//	gradientAni->stopColor1 = QColor("olive");;
//	gradientAni->stopColor2 = QColor("darkorange");
//	gradientAni->delay_ms = 0;
//	gradientAni->duration_ms = 500;
//	gradientAni->autoReverse = true;
//	gradientAni->start();
//}

//-----------------------------   -----------------------------	-----------------------------


void Form15Puzzle::on_buttonScaleForAndroid_clicked()
{
	QFont font = qApp->font();
	font.setPointSize(12);
	qApp->setFont(font);

	int buttonSize =  ui->panelClient->width() / 8;

	ui->button3x3->setMaximumSize(buttonSize, 16777215);
	ui->button4x4->setMaximumSize(buttonSize, 16777215);
	ui->button5x5->setMaximumSize(buttonSize, 16777215);

	ui->textTime->setMinimumSize(buttonSize * 1.3, 16777215);

	font = ui->textTime->font();
	font.setPointSize(17);
	ui->textTime->setFont(font);
}


void Form15Puzzle::on_panelClient_clicked()
{
	uint timeFromLastTap_ms = lastTapTime.msecsTo(QDateTime::currentDateTime());

	if (not (lastTapTime.isNull()) and (timeFromLastTap_ms < 300))
	  showDebug();

	lastTapTime = QDateTime::currentDateTime();
}

void Form15Puzzle::showDebug()
{
	if (not ui->panelDebug->isVisible())
	{
	  panelDebugMaximumHeight = ui->textTime->height() * 2;
	  ui->panelDebug->setMaximumHeight(0);
	  ui->panelDebug->setVisible(true);
	};


	if (ui->panelDebug->maximumHeight() < 10)
		animatePropertyDelay(ui->panelDebug, "maximumHeight", panelDebugMaximumHeight, 400, 0, QEasingCurve::OutBounce);
	else
		animatePropertyDelay(ui->panelDebug, "maximumHeight", 0, 400, 0, QEasingCurve::InBack);

}

bool MyEventFilter::eventFilter(QObject *obj, QEvent *event)
{
//	qDebug("event type %d", event->type());
	if ((event->type() == QEvent::MouseButtonPress) or
		(event->type() == QEvent::MouseButtonDblClick)) {
		parent->on_panelClient_clicked();
		return true;
	} else if (event->type() == QEvent::Resize) {
		parent->timerResize->stop();
		parent->timerResize->start();
//		qDebug() << QString("event resize  ") << QDateTime::currentDateTime().toString("mm:ss:zzz");

		return true;
	}
	else {
		// standard event processing
		return QObject::eventFilter(obj, event);
	}
	return false;
}




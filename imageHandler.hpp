#ifndef IMAGEHANDLER_H
#define IMAGEHANDLER_H

#include <QObject>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <opencv.hpp>
#include <tracking.hpp>
#include <io.h>

#include "core.hpp"
#include "imgproc.hpp"
#include "highgui.hpp"

#include "HSL.hpp"

using namespace std;
using namespace cv;

class ImageHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString path READ path WRITE setPath)
public:
     string window_name;
     string image_window;
	 string image_dir;
     QString m_path;
     Mat src,dst,now;

     HSL hsl;
     int color;
     int hue;
     int saturation;
     int lightness;

    //=====亮度对比度====
     int brightness;
     int contrast;

    //RGB
     int r;
     int g;
     int b;

     explicit ImageHandler(QObject *parent = nullptr);
     //explicit ImageHandler();

     //ImageHandler(string image_dir);
     Q_INVOKABLE int handle(QString image_dir);
     QString path() const;
     Q_INVOKABLE void setPath(const QString & path);

//signals:

//public slots:
    //int adjustBrightnessContrast(Mat src, Mat dst, int brightness, int contrast);
    //int adjustRGB(Mat src, Mat dst, int r, int g, int b);
    //int adjustHSL(Mat src, Mat dst, int hue, int saturation, int lightness);
    //void callbackAdjust(int, void *);
    //void switch_trackbar(int , void *);
};

#endif // IMAGEHANDLER_H

#include<direct.h>
#include "imageHandler.hpp"

#define SWAP(a, b, t)  do { t = a; a = b; b = t; } while(0)
#define CLIP_RANGE(value, min, max)  ( (value) > (max) ? (max) : (((value) < (min)) ? (min) : (value)) )
#define COLOR_RANGE(value)  CLIP_RANGE(value, 0, 255)
#define M_PI 3.14159265358979323846

ImageHandler::ImageHandler(QObject *parent)
    : QObject(parent)
    ,m_path("")
{

}

QString ImageHandler::path()const
{
    return m_path;
}

void ImageHandler::setPath(const QString &path)
{
    m_path = path;
}

/*
ImageHandler::ImageHandler() {
    window_name = "photo";
    static Mat src,dst,now;
    image_dir = "";
    static HSL hsl;
    color = 0;
    hue = 180;
    saturation = 100;
    lightness = 100;

    brightness = 100;
    contrast = 100;

    //RGB
    r = 100;
    g = 100;
    b = 100;
}*/
/*
ImageHandler::ImageHandler(string imageDir) {
	window_name = "photo";
	static Mat src,dst,now;
	image_dir = imageDir;
	static HSL hsl;
	color = 0;
	hue = 180;
	saturation = 100;
	lightness = 100;

	brightness = 100;
	contrast = 100;

	//RGB
	r = 100;
	g = 100;
	b = 100;
}
*/
int adjustBrightnessContrast(Mat src, Mat dst, int brightness, int contrast)
{
    brightness = CLIP_RANGE(brightness, -255, 255);
    contrast = CLIP_RANGE(contrast, -255, 255);

    double B = brightness / 255.;
    double c = contrast / 255. ;
    double k = tan( (45 + 44 * c) / 180 * M_PI );

    Mat lookupTable(1, 256, CV_8U);
    uchar *p = lookupTable.data;
    for (int i = 0; i < 256; i++)
        p[i] = COLOR_RANGE( (i - 127.5 * (1 - B)) * k + 127.5 * (1 + B) );

    LUT(src, lookupTable, dst);

    return 0;
}

int adjustRGB(Mat src, Mat dst, int r, int g, int b)
{

    r = CLIP_RANGE(r,-255,255);
    g = CLIP_RANGE(g,-255,255);
    b = CLIP_RANGE(b,-255,255);

    int i, j;
    Size size = src.size();
    int chns = src.channels();

    if (src.isContinuous() && dst.isContinuous())
    {
         size.width *= size.height;
         size.height = 1;
    }

    for (  i= 0; i<size.height; ++i)
    {
        const unsigned char* psrc = (const unsigned char*)(src.data+ src.step*i);
        unsigned char* pdst = (unsigned char*)dst.data+dst.step*i;
        for (  j=0; j<size.width; ++j)
        {
            pdst[j*chns] = saturate_cast<uchar>(psrc[j*chns] +r);
            pdst[j*chns+1] = saturate_cast<uchar>(psrc[j*chns+1] +g);
            pdst[j*chns+2] = saturate_cast<uchar>(psrc[j*chns+2] +b);
        }
    }

    return 0;
}

int adjustHSL(void  *userdata,Mat src, Mat dst, int hue, int saturation, int lightness) {
	ImageHandler  * imageHandler = (ImageHandler  *)userdata;
	imageHandler->hsl.channels[0].hue = hue;
	imageHandler->hsl.channels[0].saturation = saturation;
    imageHandler->hsl.channels[0].lightness = lightness;

	imageHandler->hsl.adjust(src, dst);
    return 0;
}


void callbackAdjust(int, void *userdata)
{
	ImageHandler  * imageHandler =(ImageHandler  *) userdata;
	imageHandler->now = imageHandler->src;
    if (imageHandler->hue != 180 || imageHandler->saturation != 100 || imageHandler->lightness != 100) {
        adjustHSL(imageHandler,imageHandler->now, imageHandler->dst, imageHandler->hue - 180, imageHandler->saturation - 100, imageHandler->lightness - 100);
		imageHandler->now = imageHandler->dst;
    }
    if (imageHandler->brightness != 100 || imageHandler->contrast != 100) {
        adjustBrightnessContrast(imageHandler->now, imageHandler->dst, imageHandler->brightness - 100, imageHandler->contrast - 100);
		imageHandler->now = imageHandler->dst;
    }
    if (imageHandler->r != 100 || imageHandler->g != 100 || imageHandler->b != 100) {
        adjustRGB(imageHandler->now, imageHandler->dst, imageHandler->r - 100, imageHandler->g - 100, imageHandler->b - 100);
		imageHandler->now = imageHandler->dst;
    }
    cvResizeWindow(imageHandler->image_window.data(),600,900);
    cvResizeWindow(imageHandler->window_name.data(),400,250);
    imshow(imageHandler->image_window, imageHandler->now);
}


void switch_trackbar(int , void *userdata)
{
    ImageHandler  * imageHandler = (ImageHandler  *)userdata;

    int pos = imageHandler->image_dir.find_last_of('/');
    string path(imageHandler->image_dir.substr(0,pos+1));

    //将设置应用到文件夹下所有图片
    long long iHandler;
    struct _finddata_t rawImage;
    string imageDirTemp = path + "*.*g";
	const char* imageDir= imageDirTemp.data();

    if ((iHandler = _findfirst(imageDir, &rawImage)) == -1l) {
        cout << "read raw image fail..." << endl;
        return;
    }

    string dstImageDir = path + "new/";
     if(_access(dstImageDir.c_str(), 0)==-1){
      _mkdir(dstImageDir.c_str());
      }

    do {
        String namestr = rawImage.name;
        String imageDirString = path + namestr;
        
        Mat srcImage = imread(imageDirString);
        if (srcImage.empty()) {
            printf("could not load image...%s\n", rawImage.name);
            continue;
        }

        //对每个图片应用参数
        Mat dstImage,nowImage;
        nowImage = srcImage;
        dstImage.create(srcImage.rows, srcImage.cols, srcImage.type());
        dstImage = cv::Scalar::all(0);
        if (imageHandler->hue != 180 || imageHandler->saturation != 100 || imageHandler->lightness != 100) {
            adjustHSL(imageHandler,nowImage, dstImage, imageHandler->hue - 180, imageHandler->saturation - 100, imageHandler->lightness - 100);
            nowImage = dstImage;
        }
        if (imageHandler->brightness != 100 || imageHandler->contrast != 100) {
            adjustBrightnessContrast(nowImage, dstImage, imageHandler->brightness - 100, imageHandler->contrast - 100);
            nowImage = dstImage;
        }
        if (imageHandler->r != 100 || imageHandler->g != 100 || imageHandler->b != 100) {
            adjustRGB(nowImage, dstImage, imageHandler->r - 100, imageHandler->g - 100, imageHandler->b - 100);
            nowImage = dstImage;
        }
        String dstImageDir2 = dstImageDir + namestr;
        imwrite(dstImageDir2, nowImage);

    }while (!_findnext(iHandler, &rawImage));


    _findclose(iHandler);
}


int ImageHandler::handle(QString image_dir)
{
    this->window_name = "controller";
    this->image_window = "image";
    //static Mat src,dst,now;
    //image_dir = "";
    //static HSL hsl;
    this->color = 0;
    this->hue = 180;
    this->saturation = 100;
    this->lightness = 100;

    this->brightness = 100;
    this->contrast = 100;

    //RGB
    this->r = 100;
    this->g = 100;
    this->b = 100;

    this->image_dir = image_dir.toStdString();
    this->src = imread(this->image_dir);


    if ( !this->src.data ) {
        cout << "error read image" << endl;
        return -1;
    }
	
	this->dst.create(src.rows, src.cols, src.type());
	this->dst = cv::Scalar::all(0);
	this->now = this->dst;
    destroyAllWindows();


    namedWindow(this->image_window, WINDOW_NORMAL | WINDOW_KEEPRATIO | WINDOW_GUI_EXPANDED);
    namedWindow(this->window_name, WINDOW_NORMAL | WINDOW_KEEPRATIO | WINDOW_GUI_EXPANDED);
    createTrackbar(u8"hue", this->window_name, &this->hue, 2* this->hue, callbackAdjust,this);
    createTrackbar(u8"saturation", this->window_name, &this->saturation, 2* this->saturation, callbackAdjust, this);

    createTrackbar(u8"brightness", this->window_name, &this->brightness, 2* this->brightness, callbackAdjust, this);
    createTrackbar(u8"contrast", this->window_name, &this->contrast, 2* this->contrast, callbackAdjust, this);

    createTrackbar("R", this->window_name, &this->r, 2* this->r, callbackAdjust, this);
    createTrackbar("G", this->window_name, &this->g, 2* this->g, callbackAdjust, this);
    createTrackbar("B", this->window_name, &this->b, 2* this->b, callbackAdjust, this);
    callbackAdjust(0, this);


    int colse = 0;
    createTrackbar("save", this->window_name, &colse, 1, switch_trackbar, this);
    //createButton("save button",callbackSave,this,QT_PUSH_BUTTON);


    waitKey();
    return 0;

}

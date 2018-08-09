#include<direct.h>
#include<windows.h>
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


int adjustBrightnessContrast(Mat src, Mat dst, int brightness, int contrast)
{
    brightness = CLIP_RANGE(brightness, -255, 255);
    contrast = CLIP_RANGE(contrast, -255, 255);

    double b = brightness / 255.;
    double c = contrast / 255. ;
    //double k = tan( (45 + 44 * c) / 180 * M_PI );
    double k = tan( (45 + 44 * c) / 180 * M_PI );

    Mat lookupTable(1, 256, CV_8U);
    uchar *p = lookupTable.data;
    for (int i = 0; i < 50; i++)
        //p[i] = COLOR_RANGE( (i - 127.5 * (1 - b*i/50)) * k + 127.5 * (1 + b*i/50) );
        p[i] = COLOR_RANGE( (i - 127.5 * (1 - b/2)) * k + 127.5 * (1 + b/2) );
    for (int i = 50; i < 206; i++)
        p[i] = COLOR_RANGE( (i - 127.5 * (1 - b)) * k + 127.5 * (1 + b) );
    for (int i = 206; i < 256; i++)
        //p[i] = COLOR_RANGE( (i - 127.5 * (1 - b*(256-i)/50)) * k + 127.5 * (1 + b*(256-i)/50) );
        p[i] = COLOR_RANGE( (i - 127.5 * (1 - b/2) * k + 127.5 * (1 + b/2) );

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
            if(psrc[j*chns]<50 || psrc[j*chns]>205){
                pdst[j*chns] = saturate_cast<uchar>(psrc[j*chns] +b/2);

            }else{
                pdst[j*chns] = saturate_cast<uchar>(psrc[j*chns] +b);
            }
            if(psrc[j*chns+1]<50 || psrc[j*chns+1]>205){
                pdst[j*chns+1] = saturate_cast<uchar>(psrc[j*chns+1] +g/2);
            }else{
                pdst[j*chns+1] = saturate_cast<uchar>(psrc[j*chns+1] +g);
            }
            if(psrc[j*chns+2]<50 || psrc[j*chns+2]>205){
                pdst[j*chns+2] = saturate_cast<uchar>(psrc[j*chns+2] +r/2);
            }else{
                pdst[j*chns+2] = saturate_cast<uchar>(psrc[j*chns+2] +r);
            }
            //pdst[j*chns] = saturate_cast<uchar>(psrc[j*chns] +b*((127.5-abs(psrc[j*chns]-127.5))/127.5));
            //pdst[j*chns+1] = saturate_cast<uchar>(psrc[j*chns+1] +g*((127.5-abs(psrc[j*chns+1]-127.5))/127.5));
            //pdst[j*chns+2] = saturate_cast<uchar>(psrc[j*chns+2] +r*((127.5-abs(psrc[j*chns+2]-127.5))/127.5));
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


int adjustHS(Mat src, Mat dst, int hue, int saturation) {

    Mat temp;
    temp.create(src.rows, src.cols, src.type());

    cvtColor(src, temp, CV_RGB2HSV);

    int i, j;
    Size size = src.size();
    int chns = src.channels();

    if (temp.isContinuous())
    {
        size.width *= size.height;
        size.height = 1;
    }

    // 验证参数范围
    if ( hue<-180 )
        hue = -180;

    if ( saturation<-255)
        saturation = -255;

    if ( hue>180)
        hue = 180;

    if ( saturation>255)
        saturation = 255;



    for (  i= 0; i<size.height; ++i)
    {
        unsigned char* src1 = (unsigned char*)temp.data+temp.step*i;
        for (  j=0; j<size.width; ++j)
        {
            float val;
            if(src1[j*chns]<50 || src1[j*chns]>205){
                val = src1[j*chns]+hue/2;
            }else{
                val = src1[j*chns]+hue;
            }

            if ( val < 0) val = 0.0;
            if ( val > 180 ) val = 180;
            src1[j*chns] = val;

            if(src1[j*chns+1]<50 || src1[j*chns+1]>205){
                val = src1[j*chns+1]+saturation/2;
            }else{
                val = src1[j*chns+1]+saturation;
            }

            if ( val < 0) val = 0;
            if ( val > 255 ) val = 255;
            src1[j*chns+1] = val;

        }
    }

    cvtColor(temp, dst, CV_HSV2RGB);
    if ( temp.empty())
        temp.release();

    return 0 ;
}


void callbackAdjust2(int, void *userdata)
{
    ImageHandler  * imageHandler =(ImageHandler  *) userdata;
    imageHandler->now = imageHandler->afterHSL;

    if (imageHandler->brightness != 100 || imageHandler->contrast != 100) {
        adjustBrightnessContrast(imageHandler->now, imageHandler->afterBC, imageHandler->brightness - 100, imageHandler->contrast - 100);
        imageHandler->now = imageHandler->afterBC;
    }
    if (imageHandler->r != 100 || imageHandler->g != 100 || imageHandler->b != 100) {
        adjustRGB(imageHandler->now, imageHandler->dst, imageHandler->r - 100, imageHandler->g - 100, imageHandler->b - 100);
        imageHandler->now = imageHandler->dst;
    }

    imshow(imageHandler->image_window, imageHandler->now);
}


void callbackAdjust(int, void *userdata)
{
    ImageHandler  * imageHandler =(ImageHandler  *) userdata;
    imageHandler->now = imageHandler->src;
    if (1||imageHandler->hue != 180 || imageHandler->saturation != 100) {
        adjustHS(imageHandler->now, imageHandler->afterHSL, imageHandler->hue-180, imageHandler->saturation - 100);

        imageHandler->now = imageHandler->afterHSL;

    }
    if (imageHandler->brightness != 100 || imageHandler->contrast != 100) {
        adjustBrightnessContrast(imageHandler->now, imageHandler->afterBC, imageHandler->brightness - 100, imageHandler->contrast - 100);

        imageHandler->now = imageHandler->afterBC;
    }
    if (imageHandler->r != 100 || imageHandler->g != 100 || imageHandler->b != 100) {
        adjustRGB(imageHandler->now, imageHandler->dst, imageHandler->r - 100, imageHandler->g - 100, imageHandler->b - 100);
        imageHandler->now = imageHandler->dst;
    }

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
        Mat nowImage=srcImage;
        Mat dstImage,afterHSLImage,afterBCImage;
        dstImage.create(srcImage.rows, srcImage.cols, srcImage.type());
        dstImage = cv::Scalar::all(0);
        afterHSLImage.create(srcImage.rows, srcImage.cols, srcImage.type());
        afterHSLImage = cv::Scalar::all(0);
        afterBCImage.create(srcImage.rows, srcImage.cols, srcImage.type());
        afterBCImage = cv::Scalar::all(0);

        if (imageHandler->hue != 180 || imageHandler->saturation != 100) {
            adjustHS(nowImage, afterHSLImage, imageHandler->hue - 180, imageHandler->saturation - 100);
            nowImage = afterHSLImage;
        }
        if (imageHandler->brightness != 100 || imageHandler->contrast != 100) {
            adjustBrightnessContrast(nowImage, afterBCImage, imageHandler->brightness - 100, imageHandler->contrast - 100);
            nowImage = afterBCImage;
        }
        if (imageHandler->r != 100 || imageHandler->g != 100 || imageHandler->b != 100) {
            adjustRGB(nowImage, dstImage, imageHandler->r - 100, imageHandler->g - 100, imageHandler->b - 100);
            nowImage = dstImage;
        }
        String dstImageDir2 = dstImageDir + namestr;
        imwrite(dstImageDir2, nowImage);

        if ( dstImage.empty())
            dstImage.release();
        if ( afterHSLImage.empty())
            afterHSLImage.release();
        if ( afterBCImage.empty())
            afterBCImage.release();

    }while (!_findnext(iHandler, &rawImage));



    _findclose(iHandler);
}

string UTF8ToGBK(const char* strUTF8)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8, -1, NULL, 0);
    wchar_t* wszGBK = new wchar_t[len+1];
    memset(wszGBK, 0, len*2+2);
    MultiByteToWideChar(CP_UTF8, 0, strUTF8, -1, wszGBK, len);
    len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
    char* szGBK = new char[len+1];
    memset(szGBK, 0, len+1);
    WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
    string strTemp(szGBK);
    if(wszGBK) delete[] wszGBK;
    if(szGBK) delete[] szGBK;
    return strTemp;
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
    this->afterHSL.create(src.rows, src.cols, src.type());
    this->afterHSL = cv::Scalar::all(0);
    this->afterBC.create(src.rows, src.cols, src.type());
    this->afterBC = cv::Scalar::all(0);
    this->now = this->src;
    destroyAllWindows();

    namedWindow(this->image_window, WINDOW_NORMAL | WINDOW_KEEPRATIO | WINDOW_GUI_EXPANDED);
    namedWindow(this->window_name, WINDOW_NORMAL | WINDOW_KEEPRATIO | WINDOW_GUI_EXPANDED);
    createTrackbar(("色相"), this->window_name, &this->hue, 2* this->hue, callbackAdjust,this);
    createTrackbar(("饱和度"), this->window_name, &this->saturation, 2* this->saturation, callbackAdjust, this);


    createTrackbar(("亮度"), this->window_name, &this->brightness, 2* this->brightness, callbackAdjust2, this);
    createTrackbar(("对比度"), this->window_name, &this->contrast, 2* this->contrast, callbackAdjust2, this);
    //callbackAdjust2(0, this);

    createTrackbar("R", this->window_name, &this->r, 2* this->r, callbackAdjust2, this);
    createTrackbar("G", this->window_name, &this->g, 2* this->g, callbackAdjust2, this);
    createTrackbar("B", this->window_name, &this->b, 2* this->b, callbackAdjust2, this);

    callbackAdjust(0, this);
    callbackAdjust2(0, this);

    cvResizeWindow(this->image_window.data(),600,900);
    cvResizeWindow(this->window_name.data(),450,250);


    int colse = 0;
    createTrackbar(("保存"), this->window_name, &colse, 1, switch_trackbar, this);


    waitKey();
    return 0;

}

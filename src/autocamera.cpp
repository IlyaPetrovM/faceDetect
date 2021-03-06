#include "autocamera.h"

AutoCamera::AutoCamera(Size maxRoiSize_, double maxStepX,
                       double maxStepY,
                       double zoomSpeedMin,
                       double zoomSpeedMax,
                       double zoomThr,
                       double zoomSpeedInc_,
                       double face2shot,
                       bool bZoom_,
                       bool bMove_) :
    onePerc((double)maxRoiSize_.width/100.0),
    bZoom(bZoom_),
    bMove(bMove_),
    maxRoiSize(maxRoiSize_),
    moveX(0.0,maxStepX*(double)maxRoiSize_.width/100.0,maxRoiSize_.width),
    moveY(0.0,maxStepY*(double)maxRoiSize_.width/100.0,maxRoiSize_.height),
    zoom(zoomSpeedMin,zoomSpeedMax,maxRoiSize_,zoomThr,zoomSpeedInc_,face2shot),
    roi(Rect(Point(0,0),maxRoiSize_))
{
}


void AutoCamera::update(const Rect aim){
    static Point gp;
    register bool outOfRoi;
    if(bZoom){
        zoom.update(aim,roi);
    }

    if(bMove) {
        outOfRoi = (aim&roi.getRect()).area()<aim.area();
        gp = getGoldenPoint(roi.getRect2f(),aim);
        roi.setX(moveX.update(roi.getX(),gp.x,roi.getWidth()/15.0,outOfRoi));
        roi.setY(moveY.update(roi.getY(),gp.y,roi.getHeight()/10.0,outOfRoi));
    }


//    if(roi.x<0){
//        moveX.stop();
//        roi.x = 0;
//    }
//    if(maxRoiSize.width < roi.x+roi.width)
//    {
//        moveX.stop();
//        roi.x = maxRoiSize.width-roi.width;
//    }
//    if(roi.y<0){
//        moveY.stop();
//        roi.y = 0;
//    }
//    if(maxRoiSize.height < roi.y+roi.height)
//    {
//        moveY.stop();
//        roi.y=maxRoiSize.height-roi.height;
//    }
}


Rect2f AutoCamera::getRoi() const
{
    return roi.getRect2f();
}

AutoPan AutoCamera::getMoveX() const
{
    return moveX;
}

AutoPan AutoCamera::getMoveY() const
{
    return moveY;
}

AutoZoom AutoCamera::getZoom() const
{
    return zoom;
}

Point AutoCamera::getGoldenPoint(const Rect2f &roi, const Rect &face){
    Point target;
    if(cvRound((float)roi.width/3.0) - face.width < 0 )
        /// если лицо крупное, то держать его в центре кадра
        target = topMiddleDec(roi);
    else if(face.x+cvRound((float)face.width/2.0) < topMiddleDec(roi).x
            && face.x < roi.x+topLeftDec(roi).x)
        target = topLeftDec(roi);
    else if(face.x+face.width > roi.x+topRightDec(roi).x)
        // Камера посередине не будет реагировать
        target = topRightDec(roi);
    else
        target = topMiddleDec(roi);

    Point result = (face+topMiddleDec(face) - target).tl();
    return result;
}

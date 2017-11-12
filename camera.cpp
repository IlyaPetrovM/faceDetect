#include "camera.h"

int Camera::camCnt = 0;
int Camera::getId() const
{
    return id;
}

void Camera::update()
{
    x+=vx;
    y+=vy;
    z+=vz;
    friction(vx);
    friction(vz);
    friction(vz);
    sendFrame();
}


float Camera::getFric() const
{
    return fric;
}

void Camera::setFric(float value)
{
    fric = value;
}

void Camera::addPort(Output *_port)
{
    port.push_back(_port);
}

vector<Output> Camera::getPorts() const
{

}

void Camera::friction(float &vel)
{
    if(vel>0)vel-=fric; else if(vel<0)vel+=fric;
}

Camera::Camera(Scene *_scene)
    : id(camCnt),
      scene(_scene),
      fric(0.5)
{
    camCnt++;
}

Camera::~Camera()
{
    for(size_t i=0;i<port.size();i++){
        delete port[i];
    }
}

void Camera::moveUp()
{
    vy++;
}

void Camera::moveDown()
{
    vy--;
}

void Camera::moveLeft()
{
    vx--;
}

void Camera::moveRight()
{
    vx++;
}

void Camera::zoomIn()
{
    vz++;
}

void Camera::zoomOut()
{
    vz--;
}

void Camera::sendFrame()
{
    scene->getFrame(frame);
    clog<<"cam "<< id<<" sends frame" << endl;
    for(size_t i=0; i<port.size();i++){
        port[i]->sendFrame(frame);
    }
}


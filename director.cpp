#include "director.h"
Director::Director(Scene s, int w,int h)
    : scene(s),
      stopWork(false),
      rules(5)
{
    Log::print(DEBUG,"Director1");
    Camera* cam = new Camera(&scene,0.05,0.5);
    Frame frame;
    Log::print(DEBUG,"Director2");
    scene.getFrame(&frame);
    int channels = frame.getPixels().channels();
    Output* cvWin = new CvWindow(w,h,"out",CV_WINDOW_NORMAL);
    cam->addPort(cvWin);
//    Output* rtsp = new RtspServer(w,
//                                    h,
//                                    "rtsp://:5025/dptz.sdp",
//                                    "h264",
//                                    scene.getFps(),
//                                    channels,3);
//    cam->addPort(rtsp);
    Face* f = new Face(0,0,20,20);
    Operator* op = new Operator(&scene,cam,f);
    operators.push_back(op);
}

bool Director::findDubs()
{
    faceDub.clear();
    faceDub.resize(operators.size());

    for(int j=0;j<facesNew.size();j++){
        for(int i=0; i<operators.size();i++){
            if(rules.near(operators[i]->getFace(),facesNew[j])){
                faceDub[i] = facesNew[j];
            }
        }
    }
}

void Director::manageOperators()
{
    while(!stopWork){
        vector<thread> threads;
        for(int i=0;i<operators.size();i++){
            threads.push_back(thread(&Operator::work,ref(operators[i])));
        }
        for (auto& th : threads) th.join();
    }
}

void Director::updateScene()
{
    while(!stopWork){
        scene.update();
    }
}


void Director::help()
{
    cout << "Supported commands:\n" <<
               "\tadd\tto add an operator with camera\n"<<
               "\tdel N\tto delete operator #N with camera\n"<<
               "\texit\t ends the program\n"<<
               "\top N [m|n|w|a|s|d|q|e]\tsend command to operator N:\n\t\t m - manual control;"<<
            " n - auto control;\n \t\twasd command letters are used for moving the camera; \n\t\tq - zoom in; e - zoom out" <<endl;
}

void Director::operatorsList()
{
    cout << "Operators:\n" << endl;
    for (int i=0;i<operators.size();i++)
    {
        cout << "\tOp #" << operators[i]->getId() << endl;
    }
}



Director::~Director()
{
    for(size_t i=0;i<operators.size();i++){
        delete operators[i];
    }
}
int Director::mygetch( ) {
    struct termios oldt,
    newt;
    int ch;
    tcgetattr( STDIN_FILENO, &oldt );
    newt = oldt;
    newt.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newt );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
    return ch;
}

void Director::work()
{
    thread sceneThread(&Director::updateScene,this);
    thread operThread(&Director::manageOperators,this);
    string cmd;
    while(!stopWork){
//        manageOperators();
        cin>>cmd;
        if(cmd=="add"){
            int w,h;
            cin>>w>>h;
            Camera* cam = new Camera(&scene,w,h);
            string winname = "Window of cam "+to_string(cam->getId());
            cam->addPort(new CvWindow(w,h,winname));
            operators.push_back(new Operator(&scene,cam, new Face(0,0,10,10)));
            cout << "ok"<< endl;
        }else if(cmd=="del"){
            int ch=-1;cin >> ch;
            try {
                bool found=false;
                for (int i=0;i<operators.size();i++) {
                    if(operators[i]->getId()==ch){
                        delete operators[i];
                        operators.erase(operators.begin()+i);
                        found=true;
                        break;
                    }
                }
                if(found) cout << "ok"<< endl; else cout << "no such operator" <<endl;
            } catch(exception e) {cerr<<e.what();}
        }else if(cmd=="exit"){
            stopWork=true;
        }else if(cmd=="op"){
            //find operator
            int _id;
            cin>>_id;
            for (int i=0;i<operators.size();i++) {
                if(operators[i]->getId()==_id){
                    char _cmd=' ';
                    cout<<"Control operator "<<i<<endl;
                    while(_cmd!=27 && _cmd!='n'){
                        _cmd=mygetch();
                        operators[i]->sendCommand(_cmd);
                        cout<<"get "<<_cmd<<endl;
                    }
                    cout<<"Control operator "<<i<<endl;
                    break;
                }
            }
            cout << "ok"<< endl;
        }else{
             help();
        }
    }
    sceneThread.join();
    operThread.join();
}

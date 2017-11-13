#include "director.h"
Director::Director(Scene s)
    : scene(s),
      stopWork(false),
      rules(5)
{
    Camera* cam = new Camera(&scene);
    string winname = "Window of cam "+to_string(cam->getId());
//    cam->addPort(new CvWindow(winname));
    Mat frame;
    scene.update();
    scene.getFrame(frame);
    int channels = frame.channels();
    cam->addPort(new RtspServer("rtsp://:5025/dptz.sdp","h264",scene.getFps(),scene.getWidth(),scene.getHeight(),3));

    operators.push_back(new Operator(&scene,cam, new Face()));
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
        threads.push_back(thread(&Scene::update,ref(scene)));
        for(int i=0;i<operators.size();i++){
            threads.push_back(thread(&Operator::work,ref(operators[i])));
        }
        for (auto& th : threads) th.join();
    }
//    scene.update();
//    for(int i=0;i<operators.size();i++){
//        operators[i]->work();
//    }
//    imshow("1234",scene.frame);
//    waitKey(1);
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


void Director::work()
{
    thread cons(&Director::manageOperators,this);
    string cmd;
    while(!stopWork){
//        manageOperators();
        cin>>cmd;
        if(cmd=="add"){
            Camera* cam = new Camera(&scene);
            string winname = "Window of cam "+to_string(cam->getId());
            cam->addPort(new CvWindow(winname));
            operators.push_back(new Operator(&scene,cam, new Face()));
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
                    char _cmd;
                    cin>>_cmd;
                    operators[i]->sendCommand(_cmd);
                    break;
                }
            }
            cout << "ok"<< endl;
        }else{
             help();
        }
    }
    cons.join();
}
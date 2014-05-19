#pragma once

#include "ofMain.h"
#include "ofxLeapMotion.h"
#include "ofxStrip.h"
#include "ofxOsc.h"

class testApp : public ofBaseApp{

  public:
    void setup();
    void update();
    void draw();
    void sendFeatures();
    void sendFeatureNames();
	
    void keyPressed  (int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void exit();
		
	ofxLeapMotion leap;
    ofxOscSender mySender;
	vector <ofxLeapMotionSimpleHand> simpleHands;
    
    string screenStr;
    bool isHands;
    bool isFingers;
    
	vector <int> fingersFound; 
	ofEasyCam cam;
	ofLight l1;
	ofLight l2;
	ofMaterial m1;
	
	map <int, ofPolyline> fingerTrails;
};

#include "testApp.h"

/* Note on OS X, you must have this in the Run Script Build Phase of your project. 
where the first path ../../../addons/ofxLeapMotion/ is the path to the ofxLeapMotion addon. 

cp -f ../../../addons/ofxLeapMotion/libs/lib/osx/libLeap.dylib "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/MacOS/libLeap.dylib"; install_name_tool -change ./libLeap.dylib @executable_path/libLeap.dylib "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/MacOS/$PRODUCT_NAME";

   If you don't have this you'll see an error in the console: dyld: Library not loaded: @loader_path/libLeap.dylib
*/

//--------------------------------------------------------------
void testApp::setup(){

    ofSetFrameRate(30);
//    ofSetVerticalSync(true);
	ofSetLogLevel(OF_LOG_VERBOSE);
    
    mySender.setup("localhost", 6448);

	leap.open(); 

	l1.setPosition(200, 300, 50);
	l2.setPosition(-200, -200, 50);

	cam.setOrientation(ofPoint(-20, 0, 0));

	glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
}


//--------------------------------------------------------------
void testApp::update(){

	fingersFound.clear();
	
	//here is a simple example of getting the hands and using them to draw trails from the fingertips. 
	//the leap data is delivered in a threaded callback - so it can be easier to work with this copied hand data
	
	//if instead you want to get the data as it comes in then you can inherit ofxLeapMotion and implement the onFrame method. 
	//there you can work with the frame data directly. 



    //Option 1: Use the simple ofxLeapMotionSimpleHand - this gives you quick access to fingers and palms. 
    
    simpleHands = leap.getSimpleHands();
    
    if( leap.isFrameNew() && simpleHands.size() ){
    
        screenStr = "";
        
        leap.setMappingX(-230, 230, -ofGetWidth()/2, ofGetWidth()/2);
		leap.setMappingY(90, 490, -ofGetHeight()/2, ofGetHeight()/2);
        leap.setMappingZ(-150, 150, -200, 200);
    
        for(int i = 0; i < simpleHands.size(); i++){
        
            screenStr += "Hand " + ofToString(i) + ":: " +
            ofToString(simpleHands[i].handPos.x,3) + "," +
            ofToString(simpleHands[i].handPos.y,3) + "," +
            ofToString(simpleHands[i].handPos.z,3) + "," + " :: " +
            ofToString(simpleHands[i].handNormal.x,3) + "," +
            ofToString(simpleHands[i].handNormal.y,3) + "," +
            ofToString(simpleHands[i].handNormal.z,3) +
            "\n";
            
            for(int j = 0; j < simpleHands[i].fingers.size(); j++){
                
                
                
                int id = simpleHands[i].fingers[j].id;
                ofPolyline & polyline = fingerTrails[id];
                ofPoint pt = simpleHands[i].fingers[j].pos;
                
                screenStr += "  Finger " + ofToString(id) + ":: " +
                            ofToString(pt.x, 3) + ","
                            + ofToString(pt.y, 3) + ","
                            + ofToString(pt.z, 3) +"\n";
            
           
                
                //if the distance between the last point and the current point is too big - lets clear the line 
                //this stops us connecting to an old drawing
                if( polyline.size() && (pt-polyline[polyline.size()-1] ).length() > 50 ){
                    polyline.clear(); 
                }
                
                //add our point to our trail
                polyline.addVertex(pt); 
                
                //store fingers seen this frame for drawing
                fingersFound.push_back(id);
            }
        }
    }


    // Option 2: Work with the leap data / sdk directly - gives you access to more properties than the simple approach  
    // uncomment code below and comment the code above to use this approach. You can also inhereit ofxLeapMotion and get the data directly via the onFrame callback. 
    
	/*vector <Hand> hands = leap.getLeapHands();
	if( leap.isFrameNew() && hands.size() ){

		//leap returns data in mm - lets set a mapping to our world space. 
		//you can get back a mapped point by using ofxLeapMotion::getMappedofPoint with the Leap::Vector that tipPosition returns  
		leap.setMappingX(-230, 230, -ofGetWidth()/2, ofGetWidth()/2);
		leap.setMappingY(90, 490, -ofGetHeight()/2, ofGetHeight()/2);
		leap.setMappingZ(-150, 150, -200, 200);
				
		for(int i = 0; i < hands.size(); i++){
            for(int j = 0; j < hands[i].fingers().count(); j++){
				ofPoint pt; 
			
				const Finger & finger = hands[i].fingers()[j];
								
				//here we convert the Leap point to an ofPoint - with mapping of coordinates
				//if you just want the raw point - use ofxLeapMotion::getofPoint 
				pt = leap.getMappedofPoint( finger.tipPosition() );
                								
				//lets get the correct trail (ofPolyline) out of our map - using the finger id as the key 
				ofPolyline & polyline = fingerTrails[finger.id()]; 
				
				//if the distance between the last point and the current point is too big - lets clear the line 
				//this stops us connecting to an old drawing
				if( polyline.size() && (pt-polyline[polyline.size()-1] ).length() > 50 ){
					polyline.clear(); 
				}
				
				//add our point to our trail
				polyline.addVertex(pt); 
				
				//store fingers seen this frame for drawing
				fingersFound.push_back(finger.id());
			}
		}	
	}*/

	//IMPORTANT! - tell ofxLeapMotion that the frame is no longer new. 
	leap.markFrameAsOld();
    sendFeatures();
}

void testApp::sendFeatures(){
    
    ofxOscMessage p;
    ofxOscMessage m;
    m.setAddress("/oscCustomFeatures");
    p.setAddress("/oscCustomFeaturesNames");
    
    int numHands = min(2, (int)simpleHands.size()); //ensure no more than 2 hands

    for(int i = 0; i < 2; i++){
        p.addStringArg("hand_" + ofToString(i,0) + "_posX");
        p.addStringArg("hand_" + ofToString(i,0) + "_posY");
        p.addStringArg("hand_" + ofToString(i,0) + "_posZ");
        p.addStringArg("hand_" + ofToString(i,0) + "_normX");
        p.addStringArg("hand_" + ofToString(i,0) + "_normY");
        p.addStringArg("hand_" + ofToString(i,0) + "_normZ");
    }
    
    for(int i = 0; i < numHands; i++){
        
        //add the actual hand data
        m.addFloatArg(simpleHands[i].handPos.x);
        m.addFloatArg(simpleHands[i].handPos.y);
        m.addFloatArg(simpleHands[i].handPos.z);
        
        m.addFloatArg(simpleHands[i].handNormal.x);
        m.addFloatArg(simpleHands[i].handNormal.y);
        m.addFloatArg(simpleHands[i].handNormal.z);
        
    }
    
    for(int i = 0; i < 2 - numHands; i++){
        //fill in blanks for the missing hands
        for(int j = 0; j < 6; j++)m.addFloatArg(0);
    }
    
    
    mySender.sendMessage(m);
    mySender.sendMessage(p);

}

//--------------------------------------------------------------
void testApp::draw(){
	ofDisableLighting();
    ofBackgroundGradient(ofColor(90, 90, 90), ofColor(30, 30, 30),  OF_GRADIENT_BAR);
	
	ofSetColor(200);
	ofDrawBitmapString("ofxLeapMotion - Example App\nLeap Connected? " + ofToString(leap.isConnected()), 20, 20);

	cam.begin();	

	ofPushMatrix();
		ofRotate(90, 0, 0, 1);
		ofSetColor(20);
		ofDrawGridPlane(800, 20, false);
	ofPopMatrix();
	
	ofEnableLighting();
	l1.enable();
	l2.enable();
	
	m1.begin(); 
	m1.setShininess(0.6);
	
	for(int i = 0; i < fingersFound.size(); i++){
		ofxStrip strip;
		int id = fingersFound[i];
		
		ofPolyline & polyline = fingerTrails[id];
		strip.generate(polyline.getVertices(), 15, ofPoint(0, 0.5, 0.5) );
		
		ofSetColor(255 - id * 15, 0, id * 25);
		strip.getMesh().draw();
	}
	
    l2.disable();
    
    for(int i = 0; i < simpleHands.size(); i++){
        simpleHands[i].debugDraw();
    }

	m1.end();
	cam.end();
    
    
    ofSetColor(255);
    ofDrawBitmapString(screenStr, 20,300);
	
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
  
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){
    
}

//--------------------------------------------------------------
void testApp::exit(){
    // let's close down Leap and kill the controller
    leap.close();
}

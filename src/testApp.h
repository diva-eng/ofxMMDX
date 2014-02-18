#pragma once

#include "ofMain.h"
#include "vpvl2.h"
#include "Encoding.h"
#include "ofxBullet.h"

using namespace vpvl2;

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    //vpvl2
    Factory *factory;
    IRenderEngine *engine;
    IEncoding *encoding;
    Scene *scene;
		
};

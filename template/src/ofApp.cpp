#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
}

//--------------------------------------------------------------
void ofApp::update(){
}

//--------------------------------------------------------------
void ofApp::draw(){
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch(key){
		case 'r': case 'R':
			tempSaved = false;
			break;
		case 'v': case 'V':
			showInfo = !showInfo; break;
		case 's': case 'S':
			saveTimestamped();
			break;
		default: break;
	}
}
//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
//--------------------------------------------------------------
void ofApp::keyReleased(int key){}
void ofApp::mouseMoved(int x, int y){}
void ofApp::mouseDragged(int x, int y, int button){}
void ofApp::mousePressed(int x, int y, int button){}
void ofApp::mouseReleased(int x, int y, int button){}
void ofApp::mouseEntered(int x, int y){}
void ofApp::mouseExited(int x, int y){}
void ofApp::windowResized(int w, int h){tempSaved=false; }
void ofApp::gotMessage(ofMessage msg){}

//--------------------------------------------------------------
void ofApp::saveTimestamped(){
	if(!tempSaved){
		ofSaveScreen(tempImagePath);
		tempSaved = true;
	}
	auto now = std::time(nullptr);
	std::tm * tmPtr = std::localtime(&now);
	char buf[64];
	std::strftime(buf, sizeof(buf), "%Y-%m-%d-%H-%M-%S.png", tmPtr);
	ofFile::copyFromTo(tempImagePath, buf, true, true);
}

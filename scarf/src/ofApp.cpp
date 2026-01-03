#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	pickPalette();
	buildGradient();
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	// Draw the background gradient directly for simplicity
	ofBackgroundGradient(gradientTop, gradientBottom, OF_GRADIENT_LINEAR);

	// Save a temp frame once so S can copy it with a timestamp
	if(!tempSaved){
		ofSaveScreen(tempImagePath);
		tempSaved = true;
	}

	if(showInfo){
		std::string info = "keys: R regen gradient, P new palette, S save, V hide info";
		ofSetColor(255);
		ofDrawBitmapStringHighlight(info, 10, 20);
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch(key){
		case 'r': case 'R':
			tempSaved = false;
			buildGradient();
			break;
		case 'p': case 'P':
			pickPalette();
			buildGradient();
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
void ofApp::windowResized(int w, int h){ buildGradient(); tempSaved=false; }
void ofApp::gotMessage(ofMessage msg){}

//--------------------------------------------------------------
void ofApp::pickPalette(){
	if(!palettes.empty()){
		int idx = static_cast<int>(ofRandom(palettes.size()));
		activePalette = palettes[idx];
	}
}

//--------------------------------------------------------------
void ofApp::buildGradient(){
	// Pick top/bottom from active palette or fall back to defaults
	if(activePalette){
		gradientTop = (*activePalette)[0];
		gradientBottom = (*activePalette)[2];
	}else{
		gradientTop = ofColor::black;
		gradientBottom = ofColor::white;
	}
}

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

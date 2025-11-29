#include "ofApp.h"

#include "circle.h"

void ofApp::setup() {
	// Create circles, not positioned yet
	int total = mGridRows * mGridCols;
	mCircles.reserve(total);
	for (int i = 0; i < total; ++i) {
		Circle* circle = new Circle(new ofVec2f(0, 0), 50.0f, 60, 0.2f);
		mCircles.push_back(circle);
	}
	
	// Position them correctly
	repositionCircles();
}

void ofApp::update() {
	for (Circle* circle : mCircles) {
		circle->update(ofGetElapsedTimef());
	}
}

void ofApp::draw() {
	ofSetColor(100, 150, 200, 180); // Couleur bleu semi-transparente
	for (Circle* circle : mCircles) {
		circle->draw();
	}
}

ofApp::~ofApp() {
	// Clean up dynamically allocated circles
	for (Circle* circle : mCircles) {
		delete circle;
	}
	mCircles.clear();
}

void ofApp::keyPressed(int key) {
	switch (key) {
	case 'r':
	case 'R':
		tempSaved = false;
		break;
	case 'v':
	case 'V':
		showInfo = !showInfo;
		break;
	case 's':
	case 'S':
		saveTimestamped();
		break;
	default:
		break;
	}
}
void ofApp::dragEvent(ofDragInfo dragInfo) {
}
void ofApp::keyReleased(int key) { }
void ofApp::mouseMoved(int x, int y) { }
void ofApp::mouseDragged(int x, int y, int button) { }
void ofApp::mousePressed(int x, int y, int button) { }
void ofApp::mouseReleased(int x, int y, int button) { }
void ofApp::mouseEntered(int x, int y) { }
void ofApp::mouseExited(int x, int y) { }
void ofApp::windowResized(int w, int h) { 
	tempSaved = false; 
	repositionCircles(); // Reposition circles when window is resized
}
void ofApp::gotMessage(ofMessage msg) { }

void ofApp::repositionCircles() {
	// Calculate spacing between circles
	float spacingX = (ofGetWidth() - 2 * mMarginX) / (mGridCols - 1);
	float spacingY = (ofGetHeight() - 2 * mMarginY) / (mGridRows - 1);
	
	int index = 0;
	for (int row = 0; row < mGridRows; row++) {
		for (int col = 0; col < mGridCols; col++) {
			if (index < mCircles.size()) {
				float x = mMarginX + col * spacingX;
				float y = mMarginY + row * spacingY;
				
				// Update the position of the existing circle
				mCircles[index]->setPosition(x, y);
				index++;
			}
		}
	}
}

void ofApp::saveTimestamped() {
	if (!tempSaved) {
		ofSaveScreen(tempImagePath);
		tempSaved = true;
	}
	auto now = std::time(nullptr);
	std::tm * tmPtr = std::localtime(&now);
	char buf[64];
	std::strftime(buf, sizeof(buf), "%Y-%m-%d-%H-%M-%S.png", tmPtr);
	ofFile::copyFromTo(tempImagePath, buf, true, true);
}

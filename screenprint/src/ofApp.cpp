#include "ofApp.h"

#include "circle.h"

void ofApp::setup() {
	// Create circles, not positioned yet
	int total = mGridRows * mGridCols;
	mCircles.reserve(total);
	for (int i = 0; i < total; ++i) {
		Circle * circle = new Circle(new ofVec2f(0, 0), 30.0f, 60, 0.1f);
		mCircles.push_back(circle);
	}

	// Position them correctly
	repositionCircles();
}

void ofApp::update() {
	for (Circle * circle : mCircles) {
		circle->update(ofGetElapsedTimef());
	}
}

void ofApp::draw() {
	// Off-white background
	ofBackground(248, 246, 242); // Warm off-white background

	// Enable blending for color mixing
	ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);

	// Random hue shift applied to all colors (keeps palette coherent)
	float hueShift = ofRandom(0, 360);

	// Three printing passes with HSB colors (120° apart, plus random shift)
	ofColor colors[] = {
		ofColor::fromHsb(ofMap(fmod(0 + hueShift, 360), 0, 360, 0, 255), 255, 255),     // First color
		ofColor::fromHsb(ofMap(fmod(120 + hueShift, 360), 0, 360, 0, 255), 255, 255),   // Second color (+120°)
		ofColor::fromHsb(ofMap(fmod(240 + hueShift, 360), 0, 360, 0, 255), 255, 255)    // Third color (+240°)
	};

	for (const auto & color : colors) {
		// Random offset for each color pass
		float offsetX = ofRandom(-mPrintOffset, mPrintOffset);
		float offsetY = ofRandom(-mPrintOffset, mPrintOffset);
		float rotation = ofRandom(-1.0f, 1.0f);
		ofFill();
		ofPushMatrix();
		ofTranslate(offsetX, offsetY);
		ofRotateDeg(rotation);
		for (Circle * circle : mCircles) {
			circle->setColor(color);
			circle->draw();
		}
		ofPopMatrix();
	}

	// Restore normal blending
	ofDisableBlendMode();
}

ofApp::~ofApp() {
	// Clean up dynamically allocated circles
	for (Circle * circle : mCircles) {
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
	tempSaved = false;
}

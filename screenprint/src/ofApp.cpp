#include "ofApp.h"

#include "circle.h"
#include "stroke.h"

void ofApp::setup() {
	// Load shader first
	if (ofIsGLProgrammableRenderer()) {
		ofLogNotice("ofApp") << "Using GL Programmable Renderer (GL3)";
	} else {
		ofLogNotice("ofApp") << "Using Fixed Function Pipeline Renderer (GL2)";
	}

	mShader.load("shaders/shader");

	// Create circles using the recreateCircles function
	recreateCircles();

	// Generate initial stroke
	generateStroke();

	// Generate initial random offsets
	generateNewOffsets();
}

void ofApp::update() {
	for (Circle * circle : mCircles) {
		circle->update(ofGetElapsedTimef());
	}

	// Update stroke animation
	if (mStroke) {
		mStroke->update(ofGetElapsedTimef());
	}
}

void ofApp::draw() {
	mShader.begin();

	// Pass uniforms to shader
	mShader.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
	mShader.setUniform1f("intensity", mNoiseIntensity);
	mShader.setUniform1i("blendMode", mBlendMode);

	// Off-white background
	ofBackground(248, 246, 242); // Warm off-white background

	// Enable blending for color mixing
	ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);

	// Use fixed hue shift applied to all colors (keeps palette coherent)
	float hueShift = mHueShift;

	// Three printing passes with HSB colors (120° apart, plus random shift)
	ofColor colors[] = {
		ofColor::fromHsb(ofMap(fmod(0 + hueShift, 360), 0, 360, 0, 255), 255, 255), // First color
		ofColor::fromHsb(ofMap(fmod(120 + hueShift, 360), 0, 360, 0, 255), 255, 255), // Second color (+120°)
		ofColor::fromHsb(ofMap(fmod(240 + hueShift, 360), 0, 360, 0, 255), 255, 255) // Third color (+240°)
	};

	for (int i = 0; i < 3; i++) {
		// Use fixed offset for each color pass
		float offsetX = mPrintOffsets[i].x;
		float offsetY = mPrintOffsets[i].y;
		float rotation = mPrintOffsets[i].rotation;
		ofFill();
		ofPushMatrix();
		ofTranslate(offsetX, offsetY);
		ofRotateDeg(rotation);
		for (Circle * circle : mCircles) {
			circle->setColor(colors[i]);
			circle->draw();
		}
		ofPopMatrix();
	}

	if (mStroke) {
		mStroke->draw();
	}

	// Restore normal blending
	ofDisableBlendMode();

	mShader.end();

	// depending on showInfo, display the variables at the top in small type
	if (showInfo) {
		drawInfo();
	}
}

void ofApp::drawInfo() {
	ofPushStyle();
	ofSetColor(0);
	int infoX = 10;
	int infoY = 20;
	int lineH = 14;

	ofDrawBitmapString("showInfo: " + std::string(showInfo ? "true" : "false"), infoX, infoY);
	ofDrawBitmapString("tempImagePath: " + tempImagePath, infoX, infoY += lineH);
	ofDrawBitmapString("tempSaved: " + std::string(tempSaved ? "true" : "false"), infoX, infoY += lineH);

	ofDrawBitmapString("mCircles.size: " + ofToString(mCircles.size()), infoX, infoY += lineH);

	ofDrawBitmapString("mShader: " + std::string(mShader.isLoaded() ? "loaded" : "not loaded"), infoX, infoY += lineH);

	ofDrawBitmapString("mNoiseIntensity: " + ofToString(mNoiseIntensity, 2), infoX, infoY += lineH);
	ofDrawBitmapString("mBlendMode: " + ofToString(mBlendMode), infoX, infoY += lineH);

	for (int i = 0; i < 3; ++i) {
		ofDrawBitmapString("mPrintOffsets[" + ofToString(i) + "]: x=" + ofToString(mPrintOffsets[i].x, 2)
				+ " y=" + ofToString(mPrintOffsets[i].y, 2)
				+ " rot=" + ofToString(mPrintOffsets[i].rotation, 2),
			infoX, infoY += lineH);
	}

	ofDrawBitmapString("mHueShift: " + ofToString(mHueShift, 2), infoX, infoY += lineH);
	ofDrawBitmapString("mGridCols: " + ofToString(mGridCols), infoX, infoY += lineH);
	ofDrawBitmapString("mGridRows: " + ofToString(mGridRows), infoX, infoY += lineH);
	ofDrawBitmapString("mMarginX: " + ofToString(mMarginX, 2), infoX, infoY += lineH);
	ofDrawBitmapString("mMarginY: " + ofToString(mMarginY, 2), infoX, infoY += lineH);
	ofDrawBitmapString("mCircleDiameter: " + ofToString(mCircleDiameter, 2), infoX, infoY += lineH);
	ofDrawBitmapString("mPrintOffset: " + ofToString(mPrintOffset, 2), infoX, infoY += lineH);
	ofDrawBitmapString("mStroke: " + std::string(mStroke ? "exists" : "null"), infoX, infoY += lineH);
	if (mStroke) {
		ofDrawBitmapString("Stroke pos: (" + ofToString(mStroke->getPosition()->x, 1) + ", " + ofToString(mStroke->getPosition()->y, 1) + ")", infoX, infoY += lineH);
	}

	ofPopStyle();
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
		if (mStroke) {
			mStroke->setDebug(showInfo);
		}
		break;
	case 's':
	case 'S':
		ofLogNotice("ofApp") << "Saving timestamped screenshot.";
		saveTimestamped();
		break;
	case '+':
	case '=':
		mNoiseIntensity = ofClamp(mNoiseIntensity + 0.1f, 0.0f, 1.0f);
		ofLogNotice("ofApp") << "Noise intensity: " << mNoiseIntensity;
		break;
	case '-':
	case '_':
		mNoiseIntensity = ofClamp(mNoiseIntensity - 0.1f, 0.0f, 1.0f);
		ofLogNotice("ofApp") << "Noise intensity: " << mNoiseIntensity;
		break;
	case '0':
	case 'à':
		mNoiseIntensity = 0.0f;
		ofLogNotice("ofApp") << "Noise intensity: " << mNoiseIntensity;
		break;
	case '1':
	case '&':
		mNoiseIntensity = 1.0f;
		ofLogNotice("ofApp") << "Noise intensity: " << mNoiseIntensity;
		break;
	case 'p':
	case 'P':
		mWiggleFactor += 0.1f;
		ofLogNotice("ofApp") << "Wiggle factor: " << mWiggleFactor;
		break;
	case 'm':
	case 'M':
		mWiggleFactor = std::max(0.0f, mWiggleFactor - 0.1f);
		ofLogNotice("ofApp") << "Wiggle factor: " << mWiggleFactor;
		break;
	case OF_KEY_F1:
		mBlendMode = 0;
		ofLogNotice("ofApp") << "Blend mode: Addition";
		break;
	case OF_KEY_F2:
		mBlendMode = 1;
		ofLogNotice("ofApp") << "Blend mode: Screen";
		break;
	case OF_KEY_F3:
		mBlendMode = 2;
		ofLogNotice("ofApp") << "Blend mode: Overlay";
		break;
	case OF_KEY_F4:
		mBlendMode = 3;
		ofLogNotice("ofApp") << "Blend mode: Soft Light";
		break;
	case OF_KEY_F5:
		mBlendMode = 4;
		ofLogNotice("ofApp") << "Blend mode: Lighten-Only";
		break;
	case ' ': // Spacebar
		generateNewOffsets();
		generateStroke(); // Regenerate stroke with new parameters
		ofLogNotice("ofApp") << "Generated new print offsets and stroke";
		break;
	case OF_KEY_SHIFT: // Increase circle diameter
		mCircleDiameter = ofClamp(mCircleDiameter + 5.0f, 10.0f, 1000.0f);
		recreateCircles();
		break;
	case OF_KEY_CONTROL: // Decrease circle diameter
		mCircleDiameter = ofClamp(mCircleDiameter - 5.0f, 10.0f, 1000.0f);
		recreateCircles();
		break;
	case OF_KEY_UP: // Increase columns
		mGridCols = ofClamp(mGridCols + 1, 1, 10);
		recreateCircles();
		ofLogNotice("ofApp") << "Grid: " << mGridCols << "x" << mGridRows;
		break;
	case OF_KEY_DOWN: // Decrease columns
		mGridCols = ofClamp(mGridCols - 1, 1, 10);
		recreateCircles();
		ofLogNotice("ofApp") << "Grid: " << mGridCols << "x" << mGridRows;
		break;
	case OF_KEY_RIGHT: // Increase rows
		mGridRows = ofClamp(mGridRows + 1, 1, 15);
		recreateCircles();
		ofLogNotice("ofApp") << "Grid: " << mGridCols << "x" << mGridRows;
		break;
	case OF_KEY_LEFT: // Decrease rows
		mGridRows = ofClamp(mGridRows - 1, 1, 15);
		recreateCircles();
		ofLogNotice("ofApp") << "Grid: " << mGridCols << "x" << mGridRows;
		break;
	default:
		break;
	}
}
void ofApp::dragEvent(ofDragInfo dragInfo) { }
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

	// Reallocate shader with new dimensions
	// mShader.allocate(w, h);
}
void ofApp::gotMessage(ofMessage msg) { }

void ofApp::repositionCircles() {
	// Calculate spacing between circles, handling single row/column cases
	float spacingX = (mGridCols > 1) ? (ofGetWidth() - 2 * mMarginX) / (mGridCols - 1) : 0;
	float spacingY = (mGridRows > 1) ? (ofGetHeight() - 2 * mMarginY) / (mGridRows - 1) : 0;

	int index = 0;
	for (int row = 0; row < mGridRows; row++) {
		for (int col = 0; col < mGridCols; col++) {
			if (index < mCircles.size()) {
				float x = (mGridCols > 1) ? mMarginX + col * spacingX : ofGetWidth() / 2;
				float y = (mGridRows > 1) ? mMarginY + row * spacingY : ofGetHeight() / 2;

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

void ofApp::generateNewOffsets() {
	// Generate random offset values for each color pass
	for (int i = 0; i < 3; i++) {
		mPrintOffsets[i].x = ofRandom(-mPrintOffset, mPrintOffset);
		mPrintOffsets[i].y = ofRandom(-mPrintOffset, mPrintOffset);
		mPrintOffsets[i].rotation = ofRandom(-1.0f, 1.0f);
	}

	// Generate new random hue shift
	mHueShift = ofRandom(0, 360);
}

void ofApp::recreateCircles() {
	// Clean up existing circles
	for (Circle * circle : mCircles) {
		delete circle;
	}
	mCircles.clear();

	// Create new circles with current parameters
	int total = mGridRows * mGridCols;
	mCircles.reserve(total);
	for (int i = 0; i < total; ++i) {
		Circle * circle = new Circle(new ofVec2f(0, 0), mCircleDiameter, mCircleDiameter * 2, ofColor::white, mWiggleFactor);
		mCircles.push_back(circle);
	}

	// Position them correctly
	repositionCircles();

	// Generate new stroke with updated parameters
	generateStroke();
}

// Generates a nice big stroke across the canvas
// It goes from left to right with a random vertical position
void ofApp::generateStroke() {
	delete mStroke; // Clean up existing stroke if any

	ofColor strokeColor = ofColor::fromHsb(ofRandom(0, 255), 200, 255); // High saturation, full brightness

	ofVec2f * startPoint = new ofVec2f(ofGetWidth() * -0.2f, ofRandom((float)ofGetHeight() * 0.1f, (float)ofGetHeight() * 0.9f));
	ofVec2f * endPoint = new ofVec2f(ofGetWidth() * 1.2f, ofRandom((float)ofGetHeight() * 0.1f, (float)ofGetHeight() * 0.9f));

	mStroke = new Stroke(startPoint, endPoint, mCircleDiameter * 2.0f, strokeColor, mWiggleFactor * 500.0f);
}

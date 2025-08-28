#include "ofApp.h"

static const int MAX_SIZE = 1000;

//--------------------------------------------------------------
void ofApp::setup(){
	// IMPORTANT: Disable ARB textures for proper shader texture coordinates
	ofDisableArbTex();
	
	// Set up parameters with automatic UI
	parameters.setName("Blur Parameters");
	parameters.add(backgroundColor.set("Background", ofColor(220, 220, 240), ofColor(0), ofColor(255)));
	parameters.add(shapeSize.set("Shape Size", 60, 1, MAX_SIZE));
	parameters.add(speed.set("Speed", 2.0, 0.1, 10.0));
	parameters.add(oscillationAmplitude.set("Oscillation Amplitude", 200, 0, 400));
	parameters.add(oscillationSpeed.set("Oscillation Speed", 0.02, 0.001, 0.1));
	parameters.add(amplitudeThreshold.set("Amplitude Threshold", 0.9, 0.5, 1.0));
	parameters.add(animationEndThreshold.set("Animation End Threshold", 0.85, 0.6, 1.0));
	parameters.add(hueSpeed.set("Hue Speed", 1.5, 0.01, 5.0));
	parameters.add(colorSaturation.set("Color Saturation", 255, 0, 255));
	parameters.add(colorBrightness.set("Color Brightness", 255, 0, 255));
	parameters.add(cornerRadiusStart.set("Corner Radius Start", 0.0, 0.0, 1.0));
	parameters.add(cornerRadiusEnd.set("Corner Radius End", 0.5, 0.0, 1.0));
	parameters.add(blurAmount.set("Blur Amount", 0.8, 0.01, 1.0));
	parameters.add(showUI.set("Show UI", true));
	parameters.add(autoSave.set("Auto Save Final Frame", true));
	
	// Set up GUI
	gui.setup(parameters);
	gui.loadFromFile("settings.xml");
	
	// Initialize variables
	resetAnimation();
	tempImagePath = "final_frame_temp.png";
	tempSaved = false;
	
	// Load shaders
	if(!blurShaderX.load("blurX")) {
		ofLogError() << "Failed to load blurX shader!";
	} else {
		ofLogNotice() << "BlurX shader loaded successfully";
	}
	
	if(!blurShaderY.load("blurY")) {
		ofLogError() << "Failed to load blurY shader!";
	} else {
		ofLogNotice() << "BlurY shader loaded successfully";
	}
	
	// Set up canvas with fixed MAX_SIZExMAX_SIZE size
	setupCanvas();
}

//--------------------------------------------------------------
void ofApp::update(){
	// Only update animation if not complete
	if(!animationComplete) {
		// Update position
		yPos += speed;
		frameCounter++;
		
		// Debug output every 120 frames
		if(frameCounter % 120 == 0) {
			ofLogNotice() << "Frame " << frameCounter << ", yPos: " << yPos << ", animationComplete: " << animationComplete;
		}
		
		// Check if animation is complete using configurable threshold
		if(yPos > getThresholdPosition(animationEndThreshold)) {
			animationComplete = true;
			
			// Auto-save final frame if enabled
			if(autoSave && !tempSaved) {
				saveTempFrame();
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	// Only draw to canvas if animation is not complete
	if(!animationComplete) {
		drawToCanvas();
	}
	
	// Display the canvas
	ofSetColor(255); // Reset color to white for drawing
	canvas.draw(0, 0);
	
	// Draw UI
	if(showUI) {
		gui.draw();
		drawInfo();
	}
}

//--------------------------------------------------------------
void ofApp::setupCanvas() {
	canvas.allocate(MAX_SIZE, MAX_SIZE, GL_RGBA);
	canvas.begin();
	ofClear(backgroundColor);
	canvas.end();
	
	// Allocate blur FBOs
	fboBlurOnePass.allocate(MAX_SIZE, MAX_SIZE);
	fboBlurTwoPass.allocate(MAX_SIZE, MAX_SIZE);
}

//--------------------------------------------------------------
void ofApp::drawToCanvas() {
	canvas.begin();
	
	// Only clear background on first frame to accumulate trails
	if(firstFrame) {
		ofClear(backgroundColor);
		firstFrame = false;
		ofLogNotice() << "Cleared canvas with background color on first frame";
	}
	
	// Calculate amplitude based on vertical position (threshold parameter controls when amplitude reaches 0)
	float progress = yPos / getThresholdPosition(amplitudeThreshold);
	float currentAmplitude = oscillationAmplitude * (1.0 - ofClamp(progress, 0.0, 1.0));
	
	// Calculate vertical progress for corner radius animation (0 = start, 1 = animation end)
	float verticalProgress = yPos / getThresholdPosition(animationEndThreshold);
	verticalProgress = ofClamp(verticalProgress, 0.0, 1.0);
	
	// Calculate shape position
	float oscillation = sin(frameCounter * oscillationSpeed);
	float xPos = (MAX_SIZE / 2) + oscillation * currentAmplitude;
	
	// Calculate color (rainbow cycle) using configurable saturation and brightness
	float hue = fmod(frameCounter * hueSpeed, 255);
	ofColor hsbColor = ofColor::fromHsb(hue, colorSaturation, colorBrightness);
	
	// Draw the shape as a rounded rectangle with animated corner radius
	ofSetColor(hsbColor);
	float currentCornerRadius = ofLerp(cornerRadiusStart, cornerRadiusEnd, verticalProgress);
	float cornerRadiusPixels = shapeSize * currentCornerRadius; // Corner radius as proportion of shape size
	ofDrawRectRounded(xPos - shapeSize/2.0, yPos - shapeSize/2.0, shapeSize, shapeSize, cornerRadiusPixels);
	
	if(frameCounter % 120 == 0) {
		ofLogNotice() << "Drawing shape at " << xPos << ", " << yPos << " with color " << hsbColor << " (hue: " << hue << ")";
	}
	
	canvas.end();
	
	// Apply blur shader to the canvas (only if both shaders loaded successfully)
	if(blurShaderX.isLoaded() && blurShaderY.isLoaded()) {
		applyBlur();
	} else {
		ofLogWarning() << "Blur shaders not loaded, skipping blur effect";
	}
}

//--------------------------------------------------------------
void ofApp::applyBlur() {
	// Two-pass Gaussian blur like the OpenFrameworks example
	
	//----------------------------------------------------------
	// Pass 1: Horizontal blur
	fboBlurOnePass.begin();
	
	blurShaderX.begin();
	blurShaderX.setUniform1f("blurAmnt", blurAmount);
	blurShaderX.setUniform1f("texwidth", canvas.getWidth());

	canvas.draw(0, 0);
	
	blurShaderX.end();
	
	fboBlurOnePass.end();
	
	//----------------------------------------------------------
	// Pass 2: Vertical blur
	fboBlurTwoPass.begin();
	
	blurShaderY.begin();
	blurShaderY.setUniform1f("blurAmnt", blurAmount);
	blurShaderY.setUniform1f("texheight", canvas.getHeight());
	
	fboBlurOnePass.draw(0, 0);
	
	blurShaderY.end();
	
	fboBlurTwoPass.end();
	
	//----------------------------------------------------------
	// Copy back to canvas
	canvas.begin();
	ofClear(0, 0, 0, 0);
	fboBlurTwoPass.draw(0, 0);
	canvas.end();
}

//--------------------------------------------------------------
void ofApp::resetAnimation() {
	yPos = -shapeSize;
	frameCounter = 0;
	firstFrame = true;
	animationComplete = false;
	tempSaved = false;
	
	// Generate new seed
	seed = ofGetSystemTimeMillis();
	ofSetRandomSeed(seed);
	
	ofLogNotice() << "New seed: " << seed;
	
	// Clear canvas with background color
	if(canvas.isAllocated()) {
		canvas.begin();
		ofClear(backgroundColor);
		canvas.end();
	}
}

//--------------------------------------------------------------
void ofApp::saveTempFrame() {
	canvas.readToPixels(tempPixels);
	ofSaveImage(tempPixels, tempImagePath);
	tempSaved = true;
	ofLogNotice() << "Temporary frame saved to: " << tempImagePath;
}

//--------------------------------------------------------------
void ofApp::saveTimestamped() {
	if(!tempSaved) {
		// If no temp frame, save current canvas
		saveTempFrame();
	}
	
	// Create timestamped filename
	auto now = std::time(nullptr);
	std::tm * tmPtr = std::localtime(&now);
	char buf[64];
	std::strftime(buf, sizeof(buf), "blur_%Y-%m-%d_%H-%M-%S.png", tmPtr);
	
	// Copy temp file to timestamped name
	ofFile::copyFromTo(tempImagePath, buf, true, true);
	ofLogNotice() << "Frame saved as: " << buf;
}

//--------------------------------------------------------------
void ofApp::drawInfo() {
	// Draw info overlay
	ofSetColor(0, 0, 0, 180); // Semi-transparent black
	ofDrawRectangle(10, ofGetHeight() - 120, 250, 110);
	
	ofSetColor(255);
	string info = "Frame: " + ofToString(frameCounter) + "\n";
	info += "Seed: " + ofToString(seed) + "\n";
	info += "Status: " + string(animationComplete ? "Complete" : "Animating") + "\n";
	info += "\nControls:\n";
	info += "R - Reset animation\n";
	info += "S - Save frame\n";
	info += "G - Toggle UI\n";
	
	ofDrawBitmapString(info, 15, ofGetHeight() - 100);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch(key){
		case 'r': case 'R':
			resetAnimation();
			break;
		case 'g': case 'G':
			showUI = !showUI;
			break;
		case 's': case 'S':
			saveTimestamped();
			break;
		default: 
			break;
	}
}

//--------------------------------------------------------------
float ofApp::getThresholdPosition(float threshold) {
	return threshold * MAX_SIZE;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){}
void ofApp::mouseMoved(int x, int y){}
void ofApp::mouseDragged(int x, int y, int button){}
void ofApp::mousePressed(int x, int y, int button){}
void ofApp::mouseReleased(int x, int y, int button){}
void ofApp::mouseEntered(int x, int y){}
void ofApp::mouseExited(int x, int y){}
void ofApp::windowResized(int w, int h){ tempSaved = false; }
void ofApp::dragEvent(ofDragInfo dragInfo){}
void ofApp::gotMessage(ofMessage msg){}

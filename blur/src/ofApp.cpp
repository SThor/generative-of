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
	
	// Multi-resolution decay parameters
	parameters.add(highToMedDecayStart.set("High→Med Decay Start", 0.05, 0.0, 1.0));
	parameters.add(medToLowDecayStart.set("Med→Low Decay Start", 0.15, 0.0, 1.0));
	parameters.add(decayRate.set("Decay Rate", 0.15, 0.001, 0.5));
	parameters.add(verticalDecayMultiplier.set("Vertical Decay Multiplier", 2.0, 1.0, 5.0));
	parameters.add(highResAlpha.set("High-Res Alpha", 1.0, 0.0, 1.0));
	parameters.add(medResAlpha.set("Med-Res Alpha", 0.7, 0.0, 1.0));
	parameters.add(lowResAlpha.set("Low-Res Alpha", 0.4, 0.0, 1.0));
	parameters.add(useNearestFiltering.set("Use Nearest Filtering", false));
	
	parameters.add(showUI.set("Show UI", true));
	parameters.add(autoSave.set("Auto Save Final Frame", true));
	
	// Set up GUI
	gui.setup(parameters);
	gui.loadFromFile("settings.xml");
	
	// Initialize variables
	resetAnimation();
	debugLayer = 0; // Start with composite view
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
	// Update texture filtering if parameter changed
	static bool lastUseNearestFiltering = useNearestFiltering;
	if (lastUseNearestFiltering != useNearestFiltering) {
		updateTextureFiltering();
		lastUseNearestFiltering = useNearestFiltering;
	}
	
	// Only draw to canvas if animation is not complete
	if(!animationComplete) {
		drawToCanvas();
	}
	
	// Display the canvas or debug layer
	ofSetColor(255); // Reset color to white for drawing
	
	switch(debugLayer) {
		case 1: // High-res FBO
			highResFBO.draw(0, 0);
			break;
		case 2: // Medium-res FBO
			medResFBO.draw(0, 0, MAX_SIZE, MAX_SIZE);
			break;
		case 3: // Low-res FBO
			lowResFBO.draw(0, 0, MAX_SIZE, MAX_SIZE);
			break;
		default: // Composite (normal)
			canvas.draw(0, 0);
			break;
	}
	
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
	
	// Allocate multi-resolution FBOs
	highResFBO.allocate(MAX_SIZE, MAX_SIZE, GL_RGBA);          // 1000x1000
	medResFBO.allocate(MAX_SIZE / 2, MAX_SIZE / 2, GL_RGBA);   // 500x500
	lowResFBO.allocate(MAX_SIZE / 4, MAX_SIZE / 4, GL_RGBA);   // 250x250
	
	// Clear all multi-resolution FBOs
	highResFBO.begin();
	ofClear(0, 0, 0, 0);
	highResFBO.end();
	
	medResFBO.begin();
	ofClear(0, 0, 0, 0);
	medResFBO.end();
	
	lowResFBO.begin();
	ofClear(0, 0, 0, 0);
	lowResFBO.end();
	
	// Set initial texture filtering
	updateTextureFiltering();
}

//--------------------------------------------------------------
void ofApp::updateTextureFiltering() {
	// Set texture filtering for low-resolution FBOs
	if (useNearestFiltering) {
		// Nearest filtering for pixelated effect
		medResFBO.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
		lowResFBO.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
	} else {
		// Linear filtering for smooth scaling (default)
		medResFBO.getTexture().setTextureMinMagFilter(GL_LINEAR, GL_LINEAR);
		lowResFBO.getTexture().setTextureMinMagFilter(GL_LINEAR, GL_LINEAR);
	}
}

//--------------------------------------------------------------
void ofApp::drawToCanvas() {
	// Draw current shape to high-resolution FBO
	highResFBO.begin();
	
	// Only clear background on first frame
	if(firstFrame) {
		ofClear(backgroundColor);
		firstFrame = false;
		ofLogNotice() << "Cleared high-res FBO with background color on first frame";
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
	
	highResFBO.end();
	
	// Apply blur to high-resolution layer (only if both shaders loaded successfully)
	if(blurShaderX.isLoaded() && blurShaderY.isLoaded()) {
		applyBlur();
	} else {
		ofLogWarning() << "Blur shaders not loaded, skipping blur effect";
	}
	
	// Apply multi-resolution decay based on progress
	applyMultiResolutionDecay();
	
	// Composite all layers to final canvas
	compositeMultiResolutionLayers();
}

//--------------------------------------------------------------
void ofApp::applyBlur() {
	// Apply blur to high-resolution FBO only
	applyBlurToFBO(highResFBO);
}

//--------------------------------------------------------------
void ofApp::applyBlurToFBO(ofFbo& targetFBO) {
	// Two-pass Gaussian blur applied to any FBO
	
	// We need temporary FBOs that match the target size
	static ofFbo tempBlurPass1, tempBlurPass2;
	static int lastWidth = 0, lastHeight = 0;
	
	// Reallocate temp FBOs if size changed
	if (targetFBO.getWidth() != lastWidth || targetFBO.getHeight() != lastHeight) {
		tempBlurPass1.allocate(targetFBO.getWidth(), targetFBO.getHeight());
		tempBlurPass2.allocate(targetFBO.getWidth(), targetFBO.getHeight());
		lastWidth = targetFBO.getWidth();
		lastHeight = targetFBO.getHeight();
	}
	
	//----------------------------------------------------------
	// Pass 1: Horizontal blur
	tempBlurPass1.begin();
	
	blurShaderX.begin();
	blurShaderX.setUniform1f("blurAmnt", blurAmount);
	blurShaderX.setUniform1f("texwidth", targetFBO.getWidth());

	targetFBO.draw(0, 0);
	
	blurShaderX.end();
	
	tempBlurPass1.end();
	
	//----------------------------------------------------------
	// Pass 2: Vertical blur
	tempBlurPass2.begin();
	
	blurShaderY.begin();
	blurShaderY.setUniform1f("blurAmnt", blurAmount);
	blurShaderY.setUniform1f("texheight", targetFBO.getHeight());
	
	tempBlurPass1.draw(0, 0);
	
	blurShaderY.end();
	
	tempBlurPass2.end();
	
	//----------------------------------------------------------
	// Copy back to target FBO
	targetFBO.begin();
	ofClear(0, 0, 0, 0);
	tempBlurPass2.draw(0, 0);
	targetFBO.end();
}

//--------------------------------------------------------------
void ofApp::applyMultiResolutionDecay() {
	// Calculate animation progress
	float progress = yPos / getThresholdPosition(animationEndThreshold);
	progress = ofClamp(progress, 0.0, 1.0);
	
	// Debug logging
	if(frameCounter % 60 == 0) {
		ofLogNotice() << "Progress: " << progress << ", High→Med start: " << highToMedDecayStart << ", Med→Low start: " << medToLowDecayStart;
	}
	
	// Transfer from high-res to medium-res based on progress
	if (progress > highToMedDecayStart) {
		float decayStrength = (progress - highToMedDecayStart) / (1.0 - highToMedDecayStart);
		decayStrength = ofClamp(decayStrength, 0.0, 1.0);
		float transferAmount = decayRate * decayStrength;
		
		if(frameCounter % 60 == 0) {
			ofLogNotice() << "High→Med transfer: " << transferAmount;
		}
		
		medResFBO.begin();
		
		// Don't clear - accumulate content
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		ofSetColor(255, 255, 255, (int)(transferAmount * 255));
		
		// Draw high-res content scaled down to medium-res
		highResFBO.draw(0, 0, medResFBO.getWidth(), medResFBO.getHeight());
		
		medResFBO.end();
	}
	
	// Transfer from medium-res to low-res based on progress
	if (progress > medToLowDecayStart) {
		float decayStrength = (progress - medToLowDecayStart) / (1.0 - medToLowDecayStart);
		decayStrength = ofClamp(decayStrength, 0.0, 1.0);
		float transferAmount = decayRate * decayStrength;
		
		if(frameCounter % 60 == 0) {
			ofLogNotice() << "Med→Low transfer: " << transferAmount;
		}
		
		lowResFBO.begin();
		
		// Don't clear - accumulate content
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		ofSetColor(255, 255, 255, (int)(transferAmount * 255));
		
		// Draw medium-res content scaled down to low-res
		medResFBO.draw(0, 0, lowResFBO.getWidth(), lowResFBO.getHeight());
		
		lowResFBO.end();
	}
	
	// Reset blend mode
	ofDisableBlendMode();
	
	// Apply blur to each layer after transfers for progressive dissolution
	// Each layer gets the same blur amount, but lower resolutions get stronger relative effect
	if(blurShaderX.isLoaded() && blurShaderY.isLoaded()) {
		// Apply blur to medium-res layer (if it has content)
		if (progress > highToMedDecayStart) {
			applyBlurToFBO(medResFBO);
		}
		
		// Apply blur to low-res layer (if it has content) 
		if (progress > medToLowDecayStart) {
			applyBlurToFBO(lowResFBO);
		}
	}
}

//--------------------------------------------------------------
void ofApp::compositeMultiResolutionLayers() {
	// Composite all resolution layers to the final canvas
	canvas.begin();
	ofClear(backgroundColor);
	
	// Draw low-res layer (most dissolved, in background)
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	ofSetColor(255, 255, 255, (int)(lowResAlpha * 255));
	lowResFBO.draw(0, 0, MAX_SIZE, MAX_SIZE);
	
	// Draw medium-res layer (moderately dissolved)
	ofSetColor(255, 255, 255, (int)(medResAlpha * 255));
	medResFBO.draw(0, 0, MAX_SIZE, MAX_SIZE);
	
	// Draw high-res layer (sharp, in foreground)
	ofSetColor(255, 255, 255, (int)(highResAlpha * 255));
	highResFBO.draw(0, 0, MAX_SIZE, MAX_SIZE);
	
	ofDisableBlendMode();
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
	
	// Clear all multi-resolution FBOs
	if(highResFBO.isAllocated()) {
		highResFBO.begin();
		ofClear(0, 0, 0, 0);
		highResFBO.end();
	}
	
	if(medResFBO.isAllocated()) {
		medResFBO.begin();
		ofClear(0, 0, 0, 0);
		medResFBO.end();
	}
	
	if(lowResFBO.isAllocated()) {
		lowResFBO.begin();
		ofClear(0, 0, 0, 0);
		lowResFBO.end();
	}
}

//--------------------------------------------------------------
void ofApp::saveTempFrame() {
	// Save the currently displayed layer instead of always composite
	switch(debugLayer) {
		case 1: // High-res FBO
			highResFBO.readToPixels(tempPixels);
			ofLogNotice() << "Saving high-res layer";
			break;
		case 2: // Medium-res FBO
			medResFBO.readToPixels(tempPixels);
			ofLogNotice() << "Saving medium-res layer";
			break;
		case 3: // Low-res FBO
			lowResFBO.readToPixels(tempPixels);
			ofLogNotice() << "Saving low-res layer";
			break;
		default: // Composite (normal)
			canvas.readToPixels(tempPixels);
			ofLogNotice() << "Saving composite layer";
			break;
	}
	
	ofSaveImage(tempPixels, tempImagePath);
	tempSaved = true;
	ofLogNotice() << "Temporary frame saved to: " << tempImagePath;
}

//--------------------------------------------------------------
void ofApp::saveTimestamped() {
	// Always save the current display layer, don't use temp frame
	// Create timestamped filename with layer info
	auto now = std::time(nullptr);
	std::tm * tmPtr = std::localtime(&now);
	char buf[64];
	
	string layerName;
	switch(debugLayer) {
		case 1: layerName = "high"; break;
		case 2: layerName = "med"; break;
		case 3: layerName = "low"; break;
		default: layerName = "composite"; break;
	}
	
	std::strftime(buf, sizeof(buf), ("blur_" + layerName + "_%Y-%m-%d_%H-%M-%S.png").c_str(), tmPtr);
	
	// Save current display layer directly
	ofPixels currentPixels;
	switch(debugLayer) {
		case 1: // High-res FBO
			highResFBO.readToPixels(currentPixels);
			break;
		case 2: // Medium-res FBO
			medResFBO.readToPixels(currentPixels);
			break;
		case 3: // Low-res FBO
			lowResFBO.readToPixels(currentPixels);
			break;
		default: // Composite (normal)
			canvas.readToPixels(currentPixels);
			break;
	}
	
	ofSaveImage(currentPixels, buf);
	ofLogNotice() << "Frame saved as: " << buf << " (layer: " << layerName << ")";
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
	info += "1/2/3 - Show High/Med/Low FBO\n";
	info += "0 - Show composite (normal)\n";
	
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
		case '0':
			debugLayer = 0; // Composite
			break;
		case '1':
			debugLayer = 1; // High-res FBO
			break;
		case '2':
			debugLayer = 2; // Medium-res FBO
			break;
		case '3':
			debugLayer = 3; // Low-res FBO
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

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
	
	// Age-based blur parameters
	parameters.add(maxAge.set("Max Age (frames)", 120, 30, 2000));
	parameters.add(ageBlurMultiplier.set("Age Blur Multiplier", 2.0, 0.5, 5.0));
	parameters.add(ageFadeRate.set("Age Fade Rate", 0.02, 0.001, 0.1));
	parameters.add(debugFboDisplay.set("Debug FBO (0=final, 1=content, 2=age, 3=spread)", 0, 0, 3));
	parameters.add(ageRangeStart.set("Age Range Start (frames)", 0, 0, 2000));
	parameters.add(ageRangeEnd.set("Age Range End (frames)", 60, 10, 2000));
	parameters.add(useRelativeAge.set("Use Relative Age (0=latest, 1=oldest)", false));
	
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
	
	if(!ageCompositeShader.load("ageComposite")) {
		ofLogError() << "Failed to load ageComposite shader!";
	} else {
		ofLogNotice() << "AgeComposite shader loaded successfully";
	}
	
	if(!ageSpreadShader.load("ageSpread")) {
		ofLogError() << "Failed to load ageSpread shader!";
	} else {
		ofLogNotice() << "AgeSpread shader loaded successfully";
	}
	
	// Set up canvas with fixed MAX_SIZExMAX_SIZE size
	setupCanvas();
	setupAgeFBOs();
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
		// Draw to content FBO (non-destructive)
		drawToContentFbo();
		
		// Update age FBO (accumulative)
		updateAgeFbo();
		
		// Composite with age-based blur (non-destructive)
		compositeWithAge();
	}
	
	// Display based on debug toggle
	ofSetColor(255); // Reset color to white for drawing
	
	switch(debugFboDisplay) {
		case 0: // Final composite
			if(finalFbo.isAllocated()) {
				finalFbo.draw(0, 0);
			} else {
				ofLogWarning() << "Final FBO not allocated!";
			}
			break;
		case 1: // Content FBO
			if(contentFbo.isAllocated()) {
				contentFbo.draw(0, 0);
			} else {
				ofLogWarning() << "Content FBO not allocated!";
			}
			break;
		case 2: // Age FBO
			if(ageFbo.isAllocated()) {
				ageFbo.draw(0, 0);
			} else {
				ofLogWarning() << "Age FBO not allocated!";
			}
			break;
		case 3: // Spread FBO
			if(spreadFbo.isAllocated()) {
				spreadFbo.draw(0, 0);
			} else {
				ofLogWarning() << "Spread FBO not allocated!";
			}
			break;
		default:
			finalFbo.draw(0, 0);
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
	
	// Clear age-based FBOs
	if(ageFbo.isAllocated()) {
		ageFbo.begin();
		ofClear(0, 0, 0, 255);
		ageFbo.end();
	}
	
	if(contentFbo.isAllocated()) {
		contentFbo.begin();
		ofClear(backgroundColor);
		contentFbo.end();
	}
}

//--------------------------------------------------------------
void ofApp::saveTempFrame() {
	// Save based on current debug display mode
	switch(debugFboDisplay) {
		case 0: // Final composite
			finalFbo.readToPixels(tempPixels);
			break;
		case 1: // Content FBO
			contentFbo.readToPixels(tempPixels);
			break;
		case 2: // Age FBO
			ageFbo.readToPixels(tempPixels);
			break;
		case 3: // Spread FBO
			spreadFbo.readToPixels(tempPixels);
			break;
		default:
			finalFbo.readToPixels(tempPixels);
			break;
	}
	
	ofSaveImage(tempPixels, tempImagePath);
	tempSaved = true;
	ofLogNotice() << "Temporary frame saved to: " << tempImagePath << " (FBO mode: " << debugFboDisplay << ")";
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
	ofDrawRectangle(10, ofGetHeight() - 220, 400, 210);
	
	ofSetColor(255);
	string info = "Frame: " + ofToString(frameCounter) + "\n";
	info += "Seed: " + ofToString(seed) + "\n";
	info += "Status: " + string(animationComplete ? "Complete" : "Animating") + "\n";
	info += "yPos: " + ofToString(yPos, 1) + "\n";
	info += "Debug FBO: " + ofToString(debugFboDisplay) + " (" + 
		(debugFboDisplay == 0 ? "Final" : debugFboDisplay == 1 ? "Content" : debugFboDisplay == 2 ? "Age" : "Spread") + ")\n";
	info += "FBOs allocated: " + string(contentFbo.isAllocated() ? "✓" : "✗") + 
		string(ageFbo.isAllocated() ? "✓" : "✗") + string(finalFbo.isAllocated() ? "✓" : "✗") + "\n";
	info += "Shader loaded: " + string(ageCompositeShader.isLoaded() ? "✓" : "✗") + "\n";
	info += "Age mapping: " + string(useRelativeAge ? "Relative" : "Absolute") + "\n";
	if(!useRelativeAge) {
		info += "Age range: " + ofToString(ageRangeStart, 0) + " - " + ofToString(ageRangeEnd, 0) + " frames\n";
	}
	info += "\nControls:\n";
	info += "R - Reset animation\n";
	info += "S - Save frame\n";
	info += "G - Toggle UI\n";
	info += "D - Debug FBO toggle\n";
	
	ofDrawBitmapString(info, 15, ofGetHeight() - 200);
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
		case 'd': case 'D':
			debugFboDisplay = (debugFboDisplay + 1) % 4;
			ofLogNotice() << "Debug FBO display mode: " << debugFboDisplay;
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
void ofApp::setupAgeFBOs() {
	// Allocate age-based FBOs
	contentFbo.allocate(MAX_SIZE, MAX_SIZE, GL_RGBA);
	ageFbo.allocate(MAX_SIZE, MAX_SIZE, GL_RGBA);
	spreadFbo.allocate(MAX_SIZE, MAX_SIZE, GL_RGBA);
	finalFbo.allocate(MAX_SIZE, MAX_SIZE, GL_RGBA);
	
	ofLogNotice() << "Age-based FBOs allocated: " 
		<< "Content(" << contentFbo.getWidth() << "x" << contentFbo.getHeight() << "), "
		<< "Age(" << ageFbo.getWidth() << "x" << ageFbo.getHeight() << "), "
		<< "Spread(" << spreadFbo.getWidth() << "x" << spreadFbo.getHeight() << "), "
		<< "Final(" << finalFbo.getWidth() << "x" << finalFbo.getHeight() << ")";
	
	// Clear age FBO to black (age 0)
	ageFbo.begin();
	ofClear(0, 0, 0, 255);
	ageFbo.end();
	
	// Clear content FBO with background
	contentFbo.begin();
	ofClear(backgroundColor);
	contentFbo.end();
	
	// Clear spread FBO with background
	spreadFbo.begin();
	ofClear(backgroundColor);
	spreadFbo.end();
	
	// Clear final FBO with background
	finalFbo.begin();
	ofClear(backgroundColor);
	finalFbo.end();
	
	ofLogNotice() << "Age-based FBOs allocated and cleared successfully";
}

//--------------------------------------------------------------
void ofApp::drawToContentFbo() {
	contentFbo.begin();
	
	// Only clear with background color on first frame to allow blur to spread
	if(firstFrame) {
		ofClear(backgroundColor);
		firstFrame = false;
		ofLogNotice() << "Cleared content FBO with background color on first frame";
	}
	
	// Calculate amplitude based on vertical position
	float progress = yPos / getThresholdPosition(amplitudeThreshold);
	float currentAmplitude = oscillationAmplitude * (1.0 - ofClamp(progress, 0.0, 1.0));
	
	// Calculate vertical progress for corner radius animation
	float verticalProgress = yPos / getThresholdPosition(animationEndThreshold);
	verticalProgress = ofClamp(verticalProgress, 0.0, 1.0);
	
	// Calculate shape position
	float oscillation = sin(frameCounter * oscillationSpeed);
	float xPos = (MAX_SIZE / 2) + oscillation * currentAmplitude;
	
	// Calculate color (rainbow cycle)
	float hue = fmod(frameCounter * hueSpeed, 255);
	ofColor hsbColor = ofColor::fromHsb(hue, colorSaturation, colorBrightness);
	
	// Draw the shape as a rounded rectangle
	ofSetColor(hsbColor);
	float currentCornerRadius = ofLerp(cornerRadiusStart, cornerRadiusEnd, verticalProgress);
	float cornerRadiusPixels = shapeSize * currentCornerRadius;
	ofDrawRectRounded(xPos - shapeSize/2.0, yPos - shapeSize/2.0, shapeSize, shapeSize, cornerRadiusPixels);
	
	if(frameCounter % 120 == 0) {
		ofLogNotice() << "Drawing to content FBO: shape at " << xPos << ", " << yPos 
			<< " with color " << hsbColor << " (hue: " << hue << "), size: " << shapeSize;
	}
	
	contentFbo.end();
}

//--------------------------------------------------------------
void ofApp::updateAgeFbo() {
	ageFbo.begin();
	
	// Set blend mode to write only where new content exists
	ofEnableBlendMode(OF_BLENDMODE_DISABLED);
	
	// Draw current frame color as age marker
	ofColor frameColor = frameToColor(frameCounter);
	ofSetColor(frameColor);
	
	// Calculate shape position (same as in drawToContentFbo)
	float progress = yPos / getThresholdPosition(amplitudeThreshold);
	float currentAmplitude = oscillationAmplitude * (1.0 - ofClamp(progress, 0.0, 1.0));
	float verticalProgress = yPos / getThresholdPosition(animationEndThreshold);
	verticalProgress = ofClamp(verticalProgress, 0.0, 1.0);
	float oscillation = sin(frameCounter * oscillationSpeed);
	float xPos = (MAX_SIZE / 2) + oscillation * currentAmplitude;
	float currentCornerRadius = ofLerp(cornerRadiusStart, cornerRadiusEnd, verticalProgress);
	float cornerRadiusPixels = shapeSize * currentCornerRadius;
	
	// Draw age marker at same position as content
	ofDrawRectRounded(xPos - shapeSize/2.0, yPos - shapeSize/2.0, shapeSize, shapeSize, cornerRadiusPixels);
	
	if(frameCounter % 120 == 0) {
		ofLogNotice() << "Drawing to age FBO: frame " << frameCounter << " as color " << frameColor 
			<< " at position " << xPos << ", " << yPos;
	}
	
	ofDisableBlendMode();
	ageFbo.end();
}

//--------------------------------------------------------------
void ofApp::compositeWithAge() {
	// Pass 1: Color spreading - aged pixels "paint" their surroundings
	if(ageSpreadShader.isLoaded()) {
		spreadFbo.begin();
		ofClear(backgroundColor);
		
		ageSpreadShader.begin();
		ageSpreadShader.setUniformTexture("contentTex", contentFbo.getTexture(), 0);
		ageSpreadShader.setUniformTexture("ageTex", ageFbo.getTexture(), 1);
		ageSpreadShader.setUniform1f("currentFrame", frameCounter);
		ageSpreadShader.setUniform1f("maxAge", maxAge);
		ageSpreadShader.setUniform1f("ageBlurMultiplier", ageBlurMultiplier);
		ageSpreadShader.setUniform1f("ageFadeRate", ageFadeRate);
		ageSpreadShader.setUniform1f("blurAmount", blurAmount);
		ageSpreadShader.setUniform1f("texWidth", MAX_SIZE);
		ageSpreadShader.setUniform1f("texHeight", MAX_SIZE);
		ageSpreadShader.setUniform1f("ageRangeStart", ageRangeStart);
		ageSpreadShader.setUniform1f("ageRangeEnd", ageRangeEnd);
		ageSpreadShader.setUniform1i("useRelativeAge", useRelativeAge ? 1 : 0);
		
		contentFbo.draw(0, 0);
		
		ageSpreadShader.end();
		spreadFbo.end();
		
		// Pass 2: Apply traditional blur to the spread result for smooth dissolution
		if(blurShaderX.isLoaded() && blurShaderY.isLoaded()) {
			// Horizontal blur
			fboBlurOnePass.begin();
			blurShaderX.begin();
			blurShaderX.setUniform1f("blurAmnt", blurAmount * 0.7); // Slightly stronger blur
			blurShaderX.setUniform1f("texwidth", spreadFbo.getWidth());
			spreadFbo.draw(0, 0);
			blurShaderX.end();
			fboBlurOnePass.end();
			
			// Vertical blur to final
			finalFbo.begin();
			blurShaderY.begin();
			blurShaderY.setUniform1f("blurAmnt", blurAmount * 0.7); // Slightly stronger blur
			blurShaderY.setUniform1f("texheight", fboBlurOnePass.getHeight());
			fboBlurOnePass.draw(0, 0);
			blurShaderY.end();
			finalFbo.end();
		} else {
			// No blur available, just copy spread result
			finalFbo.begin();
			ofClear(backgroundColor);
			spreadFbo.draw(0, 0);
			finalFbo.end();
		}
	} else {
		// Fallback: just copy content to final
		finalFbo.begin();
		ofClear(backgroundColor);
		contentFbo.draw(0, 0);
		finalFbo.end();
		
		if(frameCounter % 120 == 0) {
			ofLogWarning() << "Age spread shader not loaded, using fallback";
		}
	}
	
	if(frameCounter % 120 == 0) {
		ofLogNotice() << "Age-based color spreading and blur completed";
	}
}

//--------------------------------------------------------------
ofColor ofApp::frameToColor(int frameNumber) {
	// Convert frame number to RGB (allows 16.7M frames before wrapping)
	int r = (frameNumber >> 16) & 0xFF;
	int g = (frameNumber >> 8) & 0xFF;
	int b = frameNumber & 0xFF;
	return ofColor(r, g, b, 255);
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

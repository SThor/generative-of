#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	// Initialize default palettes in parameters first
	initializeDefaultPalettes();
	
	// GUI setup
	params.setName("Controls");
	int maxIdx = std::max(0, (int)allPalettesGroup.size()-1);
	paletteIndexParam.set("Palette Index", 0, 0, maxIdx);
	params.add(paletteIndexParam);

	// Add all palettes group (already set up in initializeDefaultPalettes)
	params.add(allPalettesGroup);
	
	// Halftone effect parameters
	halftoneGroup.setName("Dither Effect");
	noiseScale.set("Grain Size", 100.0, 10.0, 500.0);
	randomSeed.set("Random Seed", 0.0, 0.0, 1000.0);
	halftoneGroup.add(noiseScale);
	halftoneGroup.add(randomSeed);
	params.add(halftoneGroup);

	// New dither parameters
	ditherGroup.setName("Post-Process Dither");
	dotRadius.set("Dot Radius", 2.0, 0.5, 5.0);
	samplesPerPixel.set("Quality", 8, 4, 16);
	enableDithering.set("Enable Dithering", true);
	ditherGroup.add(dotRadius);
	ditherGroup.add(samplesPerPixel);
	ditherGroup.add(enableDithering);
	params.add(ditherGroup);

	gui.setup(params);
	// place GUI at top-right
	gui.setPosition(ofGetWidth() - gui.getWidth() - 10, 10);

	// Load settings from XML (this will override our defaults)
	gui.loadFromFile("settings.xml");
	
	// Initialize shader and FBO for halftone effect
	shaderLoaded = halftoneShader.load("shaders/halftone");
	if(shaderLoaded) {
		ofLogNotice("ofApp") << "Halftone shader loaded successfully";
	} else {
		ofLogError("ofApp") << "Failed to load halftone shader";
	}
	
	// Load dither post-processing shader
	ditherShaderLoaded = ditherShader.load("shaders/dither_postprocess");
	if(ditherShaderLoaded) {
		ofLogNotice("ofApp") << "Dither post-processing shader loaded successfully";
	} else {
		ofLogError("ofApp") << "Failed to load dither post-processing shader";
	}
	
	// Initialize FBOs to match window size
	gradientFbo.allocate(ofGetWidth(), ofGetHeight());
	originalGradientFbo.allocate(ofGetWidth(), ofGetHeight());

	// After loading, sync parameters to core data structures
	syncParametersToCore();
	
	// ONLY NOW add the palette index listener (so it doesn't trigger during setup)
	paletteIndexParam.addListener(this, &ofApp::onPaletteIndexChanged);
	
	// Add listeners for dither parameters to trigger refresh
	noiseScale.addListener(this, &ofApp::onHalftoneParamChanged);
	randomSeed.addListener(this, &ofApp::onHalftoneParamChanged);
	
	// Initial gradient
	buildGradient();
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	// Step 1: Render original gradient to FBO
	originalGradientFbo.begin();
	ofClear(0, 0, 0, 255);
	
	// Simple gradient without any effects
	ofBackgroundGradient(gradientTop, gradientBottom, OF_GRADIENT_LINEAR);
	
	originalGradientFbo.end();
	
	// Step 2: Apply dithering if enabled and shader loaded
	if(enableDithering && ditherShaderLoaded) {
		// Render dithered version to screen
		ditherShader.begin();
		ditherShader.setUniformTexture("gradientTexture", originalGradientFbo.getTexture(), 0);
		ditherShader.setUniform3f("topColor", gradientTop.r/255.0f, gradientTop.g/255.0f, gradientTop.b/255.0f);
		ditherShader.setUniform3f("bottomColor", gradientBottom.r/255.0f, gradientBottom.g/255.0f, gradientBottom.b/255.0f);
		ditherShader.setUniform1f("dotRadius", dotRadius.get());
		ditherShader.setUniform1f("grainSize", noiseScale.get() / 10.0f); // Convert to pixel size
		ditherShader.setUniform1f("randomSeed", randomSeed.get());
		ditherShader.setUniform1i("samplesPerPixel", samplesPerPixel.get());
		ditherShader.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
		
		// Draw full-screen quad
		originalGradientFbo.draw(0, 0);
		
		ditherShader.end();
	} 
	else if(shaderLoaded && useHalftoneEffect) {
		// Fallback to old halftone shader
		halftoneShader.begin();
		halftoneShader.setUniform3f("topColor", gradientTop.r/255.0f, gradientTop.g/255.0f, gradientTop.b/255.0f);
		halftoneShader.setUniform3f("bottomColor", gradientBottom.r/255.0f, gradientBottom.g/255.0f, gradientBottom.b/255.0f);
		halftoneShader.setUniform1f("noiseScale", noiseScale.get());
		halftoneShader.setUniform1f("randomSeed", randomSeed.get());
		halftoneShader.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
		
		ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
		halftoneShader.end();
	} 
	else {
		// No dithering - show original gradient
		originalGradientFbo.draw(0, 0);
	}

	// Save a temp frame once so S can copy it with a timestamp
	if(!tempSaved){
		ofSaveScreen(tempImagePath);
		tempSaved = true;
	}

	if(mShowInfo){
		std::string info = "keys: R regen, P random palette, S save, V hide info, G toggle GUI, H toggle halftone, D toggle dithering";
		ofSetColor(255);
		ofDrawBitmapStringHighlight(info, 10, 20);
	}

	if(mShowGui){
		gui.draw();
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch(key){
		case 'r': case 'R':
			tempSaved = false;
			randomSeed = ofRandom(1000.0);
			buildGradient();
			break;
		case 'p': case 'P':
			pickPalette(); // pick random
			paletteIndexParam = activePaletteIndex; // sync GUI
			buildGradient();
			tempSaved = false;
			break;
		case 'v': case 'V':
			mShowInfo = !mShowInfo; break;
			case 'g': case 'G':
				mShowGui = !mShowGui; break;
		case 's': case 'S':
			saveTimestamped();
			break;
		case 'h': case 'H':
			useHalftoneEffect = !useHalftoneEffect;
			tempSaved = false;
			break;
		case 'd': case 'D':
			enableDithering = !enableDithering;
			tempSaved = false;
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
void ofApp::windowResized(int w, int h){ 
	buildGradient(); 
	tempSaved=false; 
	if(mShowGui){ 
		gui.setPosition(ofGetWidth() - gui.getWidth() - 10, 10);
	}
	// Resize FBOs to match new window size
	if(gradientFbo.isAllocated()) {
		gradientFbo.allocate(w, h);
	}
	if(originalGradientFbo.isAllocated()) {
		originalGradientFbo.allocate(w, h);
	}
}
void ofApp::gotMessage(ofMessage msg){}

//--------------------------------------------------------------
void ofApp::pickPalette(){
	if(!palettes.empty()){
		int idx = static_cast<int>(ofRandom(palettes.size()));
		activePaletteIndex = idx;
	}
}

//--------------------------------------------------------------
void ofApp::buildGradient(){
	// Get current palette from parameters
	auto currentPalette = getCurrentPalette();
	gradientTop = currentPalette[0];
	gradientBottom = currentPalette[2];
}

//--------------------------------------------------------------
void ofApp::onPaletteIndexChanged(int& idx){
	activePaletteIndex = ofClamp(idx, 0, std::max(0, (int)palettes.size()-1));
	buildGradient();
	tempSaved = false;
}

//--------------------------------------------------------------
void ofApp::onAnyAllPaletteColorChanged(ofColor& c){
	// Find which parameter changed and update core data
	for(size_t i=0; i<allPaletteColorParams.size(); ++i){
		for(int j=0; j<3; ++j){
			if(&allPaletteColorParams[i][j].get() == &c){
				if(i < palettes.size()){
					palettes[i][j] = c;
					// If this is the active palette, rebuild gradient
					if((int)i == activePaletteIndex){
						buildGradient();
						tempSaved = false;
					}
				}
				return;
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::onHalftoneParamChanged(float& param){
	// Mark temp as not saved when halftone parameters change
	tempSaved = false;
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

//--------------------------------------------------------------
void ofApp::initializeDefaultPalettes(){
	// Set up default palettes in parameters
	allPalettesGroup.setName("All Palettes");
	allPaletteColorParams.clear();
	
	// Pink palette
	std::array<ofParameter<ofColor>,3> pinkParams;
	ofParameterGroup pinkGroup;
	pinkGroup.setName("Pink");
	pinkParams[0].set("Color 0", ofColor(202,49,98), ofColor(0), ofColor(255));
	pinkParams[1].set("Color 1", ofColor(202,118,154), ofColor(0), ofColor(255));
	pinkParams[2].set("Color 2", ofColor(187,141,159), ofColor(0), ofColor(255));
	for(int j=0; j<3; ++j){
		pinkGroup.add(pinkParams[j]);
		pinkParams[j].addListener(this, &ofApp::onAnyAllPaletteColorChanged);
	}
	allPalettesGroup.add(pinkGroup);
	allPaletteColorParams.push_back(pinkParams);
	
	// Green palette
	std::array<ofParameter<ofColor>,3> greenParams;
	ofParameterGroup greenGroup;
	greenGroup.setName("Green");
	greenParams[0].set("Color 0", ofColor(14,96,82), ofColor(0), ofColor(255));
	greenParams[1].set("Color 1", ofColor(94,142,80), ofColor(0), ofColor(255));
	greenParams[2].set("Color 2", ofColor(168,172,158), ofColor(0), ofColor(255));
	for(int j=0; j<3; ++j){
		greenGroup.add(greenParams[j]);
		greenParams[j].addListener(this, &ofApp::onAnyAllPaletteColorChanged);
	}
	allPalettesGroup.add(greenGroup);
	allPaletteColorParams.push_back(greenParams);
}

//--------------------------------------------------------------
void ofApp::syncParametersToCore(){
	// Sync all palettes from parameters to core data
	palettes.clear();
	paletteNames.clear();
	
	ofLogNotice("syncParametersToCore") << "allPaletteColorParams.size(): " << allPaletteColorParams.size();
	
	for(size_t i=0; i<allPaletteColorParams.size(); ++i){
		std::array<ofColor,3> palette;
		for(int j=0; j<3; ++j){
			palette[j] = allPaletteColorParams[i][j].get();
		}
		palettes.push_back(palette);
		
		// Extract palette name from parameter group
		std::string name = "Palette " + ofToString(i);
		if(i == 0) name = "Pink";
		else if(i == 1) name = "Green";
		paletteNames.push_back(name);
	}
	
	ofLogNotice("syncParametersToCore") << "palettes.size(): " << palettes.size();
	
	// Sync active palette index
	paletteIndexParam.setMax(palettes.size()-1);
	paletteIndexParam.set(ofClamp(paletteIndexParam.get(), 0, paletteIndexParam.getMax()));

	ofLogNotice("syncParametersToCore") << "activePaletteIndex: " << activePaletteIndex;
}

//--------------------------------------------------------------
std::array<ofColor,3> ofApp::getCurrentPalette(){
	// Return current palette colors from parameters, with fallback
	if(activePaletteIndex >= 0 && activePaletteIndex < (int)allPaletteColorParams.size()){
		return {
			allPaletteColorParams[activePaletteIndex][0].get(),
			allPaletteColorParams[activePaletteIndex][1].get(),
			allPaletteColorParams[activePaletteIndex][2].get()
		};
	}
	// Fallback to default pink palette
	return {
		ofColor(202,49,98),
		ofColor(202,118,154), 
		ofColor(187,141,159)
	};
}

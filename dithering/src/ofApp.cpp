#include "ofApp.h"

void ofApp::setup() {
	// Allocate pixel buffer and image once
	pixels.allocate(ofGetWidth(), ofGetHeight(), OF_IMAGE_GRAYSCALE);
	
	// Load dither patterns
	loadDitherPatterns();
}

void ofApp::update() {
}

void ofApp::draw() {
	generateNoise();
	ditherAndDraw();
	
	if(showPatternPreview) drawPatternPreview();
	if(showInfo) drawInfo();
}

void ofApp::generateNoise() {
	// Use paused time if animation is paused, otherwise use current time
	float timeOffset = animationPaused ? pausedTime : ofGetElapsedTimef() * 0.05;
	
	for (int y = 0; y < ofGetHeight(); y++) {
		for (int x = 0; x < ofGetWidth(); x++) {
			float noise = ofNoise(x * 0.002, y * 0.002, timeOffset);
			pixels.setColor(x, y, ofColor(noise * 255));
		}
	}
}

void ofApp::ditherAndDraw() {
	if (ditherPatterns.empty()) {
		ofLogWarning() << "No dither patterns loaded.";
		
		image.setFromPixels(pixels);
		image.draw(0, 0);
		return;
	}

	if (gpuDithering) {
		gpuDither();
	} else {
		cpuDither();
	}
	return;
}

void ofApp::gpuDither() {
	// Placeholder for GPU dithering implementation
	// Currently just draws the original image

	image.setFromPixels(pixels);
	image.draw(0, 0);
}

void ofApp::cpuDither() {
	// load the current dither pattern
	ofImage &pattern = ditherPatterns[currentPatternIndex].image;
	int patternWidth = pattern.getWidth();
	int patternHeight = pattern.getHeight();
	ofPixels &patternPixels = pattern.getPixels();

	// iterate over each pixel and apply dithering
	for (int y = 0; y < ofGetHeight(); y++) {
		for (int x = 0; x < ofGetWidth(); x++) {
			ofColor pixelColor = pixels.getColor(x, y);
			int brightness = pixelColor.r; // Grayscale, so r=g=b

			// Get corresponding pattern pixel
			int patternX = x % patternWidth;
			int patternY = y % patternHeight;
			ofColor patternColor = patternPixels.getColor(patternX, patternY);
			int threshold = patternColor.r; // Assuming pattern is grayscale

			// Apply dithering
			if (brightness > threshold) {
				pixels.setColor(x, y, ofColor(255));
			} else {
				pixels.setColor(x, y, ofColor(0));
			}
		}
	}
	
	image.setFromPixels(pixels);
	image.draw(0, 0);
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
	case OF_KEY_UP: // Previous dither pattern
		if(!ditherPatterns.empty()) {
			currentPatternIndex = (currentPatternIndex - 1 + ditherPatterns.size()) % ditherPatterns.size();
			tempSaved = false;
		}
		break;
	case OF_KEY_DOWN: // Next dither pattern
		if(!ditherPatterns.empty()) {
			currentPatternIndex = (currentPatternIndex + 1) % ditherPatterns.size();
			tempSaved = false;
		}
		break;
	case 'g':
	case 'G':
		gpuDithering = !gpuDithering;
		ofLogNotice() << "GPU Dithering: " << (gpuDithering ? "ON" : "OFF");
		break;
	case 'p':
	case 'P':
		showPatternPreview = !showPatternPreview;
		break;
	case ' ': // Spacebar
		animationPaused = !animationPaused;
		if(animationPaused) {
			// Store current time when pausing
			pausedTime = ofGetElapsedTimef() * 0.05;
		}
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
void ofApp::windowResized(int w, int h) { tempSaved = false; }
void ofApp::gotMessage(ofMessage msg) { }

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

void ofApp::loadDitherPatterns() {
	std::string patternsPath = "patterns/";
	ofDirectory dir(patternsPath);
	
	// Allow only png files
	dir.allowExt("png");
	dir.listDir();
	
	if(dir.size() == 0) {
		ofLogWarning() << "No PNG files found in " << patternsPath;
		return;
	}
	
	// Load all pattern files
	ditherPatterns.clear();
	
	for(int i = 0; i < dir.size(); i++) {
		std::string filename = dir.getName(i);
		
		ofImage pattern;
		if(pattern.load(patternsPath + filename)) {
			DitherPattern ditherPattern;
			ditherPattern.filename = filename;
			ditherPattern.image = pattern;
			ditherPattern.area = pattern.getWidth() * pattern.getHeight();
			ditherPatterns.push_back(ditherPattern);
			ofLogNotice() << "Loaded dither pattern: " << filename;
		} else {
			ofLogError() << "Failed to load: " << filename;
		}
	}
	
	// Sort by image size (area)
	std::sort(ditherPatterns.begin(), ditherPatterns.end(), [](const DitherPattern& a, const DitherPattern& b) {
		return a.area < b.area;
	});
	
	ofLogNotice() << "Loaded " << ditherPatterns.size() << " dither patterns (sorted by size)";
}

void ofApp::drawInfo() {
	// Build info lines
	std::vector<std::string> infoLines;
	infoLines.push_back("Dither Pattern Tester");
	infoLines.push_back("");
	
	if(!ditherPatterns.empty()) {
		infoLines.push_back("Pattern: " + ofToString(currentPatternIndex + 1) + "/" + ofToString(ditherPatterns.size()));
		
		int w = ditherPatterns[currentPatternIndex].image.getWidth();
		int h = ditherPatterns[currentPatternIndex].image.getHeight();
		infoLines.push_back("Size: " + ofToString(w) + "x" + ofToString(h));
		
		infoLines.push_back("");
		infoLines.push_back("Available Patterns:");
		
		// List all pattern files with star for current one
		for(int i = 0; i < ditherPatterns.size(); i++) {
			std::string marker = (i == currentPatternIndex) ? " * " : "   ";
			infoLines.push_back(marker + ditherPatterns[i].filename);
		}
		
	} else {
		infoLines.push_back("No patterns found in patterns/");
	}
	infoLines.push_back("");
	infoLines.push_back("Dithering Mode: " + std::string(gpuDithering ? "GPU" : "CPU"));
	infoLines.push_back("Pattern Preview: " + std::string(showPatternPreview ? "ON" : "OFF"));
	infoLines.push_back("Animation: " + std::string(animationPaused ? "PAUSED" : "PLAYING"));
	
	infoLines.push_back("");
	infoLines.push_back("Controls:");
	infoLines.push_back("up/down - Change pattern");
	infoLines.push_back("SPACE - Pause/play animation");
	infoLines.push_back("V - Toggle help");
	infoLines.push_back("P - Toggle pattern preview");
	infoLines.push_back("S - Save screenshot");
	infoLines.push_back("G - Toggle GPU/CPU dithering");
	
	// Calculate dynamic size
	int lineHeight = 14;
	int padding = 20;
	int rectHeight = infoLines.size() * lineHeight + padding;
	int rectWidth = 300; // Increased width for longer filenames
	
	// Semi-transparent background
	ofSetColor(0, 0, 0, 180);
	ofDrawRectangle(10, 10, rectWidth, rectHeight);
	
	// Draw each line
	ofSetColor(255);
	for(int i = 0; i < infoLines.size(); i++) {
		ofDrawBitmapString(infoLines[i], 20, 30 + i * lineHeight);
	}
}

void ofApp::drawPatternPreview() {
	if(ditherPatterns.empty()) return;
	
	const int previewSize = 64;
	const int margin = 10;
	
	// Position in top-right corner
	int x = ofGetWidth() - previewSize - margin;
	int y = margin;
	
	// Draw dark background
	ofSetColor(40);
	ofDrawRectangle(x - 2, y - 2, previewSize + 4, previewSize + 4);
	
	// Draw pattern preview scaled to 64x64 using nearest neighbor
	ofSetColor(255);  // Full white tint
	
	// Set texture filtering to nearest neighbor for crisp pixels
	ditherPatterns[currentPatternIndex].image.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
	ditherPatterns[currentPatternIndex].image.draw(x, y, previewSize, previewSize);
	
	// Draw border
	ofSetColor(120);
	ofNoFill();
	ofDrawRectangle(x - 1, y - 1, previewSize + 2, previewSize + 2);
	ofFill();
}

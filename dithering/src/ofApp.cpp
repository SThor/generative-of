#include "ofApp.h"

void ofApp::setup() {
	// Allocate pixel buffers once
	pixels.allocate(ofGetWidth(), ofGetHeight(), OF_IMAGE_GRAYSCALE);
	basePixels.allocate(ofGetWidth(), ofGetHeight(), OF_IMAGE_GRAYSCALE);
	
	// Load dither patterns
	loadDitherPatterns();
}

void ofApp::update() {
}

void ofApp::draw() {
	ofSetColor(255);
	if (!hasBaseImage) {
		basePixels = generateNoise();
	}
	ditherAndDraw();
	
	if(showPatternPreview) drawPatternPreview();
	if(showInfo) drawInfo();
}

ofPixels ofApp::generateNoise() {
	auto noisePixels = ofPixels();
	noisePixels.allocate(ofGetWidth(), ofGetHeight(), OF_IMAGE_GRAYSCALE);

	// Use paused time if animation is paused, otherwise use current time
	float timeOffset = animationPaused ? pausedTime : ofGetElapsedTimef() * 0.05;
	
	for (int y = 0; y < ofGetHeight(); y++) {
		for (int x = 0; x < ofGetWidth(); x++) {
			float noise = ofNoise(x * 0.001, y * 0.001, timeOffset);
			noisePixels.setColor(x, y, ofColor(noise * 255));
		}
	}

	return noisePixels;
}

void ofApp::ditherAndDraw() {
	if (ditherPatterns.empty()) {
		ofLogWarning() << "No dither patterns loaded.";
		
		image.setFromPixels(basePixels);
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
	pixels = basePixels; // start with base image

	// Placeholder for GPU dithering implementation
	// Currently just draws the original image
	image.setFromPixels(pixels);
	image.draw(0, 0);
}

void ofApp::cpuDither() {
	pixels = basePixels; // start with base image

	// Apply dithering using current pattern
	applyDitherToPixels(pixels, ditherPatterns[currentPatternIndex]);
	
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
	case 'e':
	case 'E':
		exportAllPatterns();
		break;
	default:
		break;
	}
}

void ofApp::dragEvent(ofDragInfo dragInfo){
	if (!dragInfo.files.size()) return;

	// Check if drop is in pattern preview area (top-right corner)
	int previewX = ofGetWidth() - PREVIEW_SIZE - PREVIEW_MARGIN;
	int previewY = PREVIEW_MARGIN;
	
	bool isInPreviewArea = (dragInfo.position.x >= previewX - PREVIEW_EXTRA_MARGIN && 
	                        dragInfo.position.x <= previewX + PREVIEW_SIZE + PREVIEW_EXTRA_MARGIN &&
	                        dragInfo.position.y >= previewY - PREVIEW_EXTRA_MARGIN && 
	                        dragInfo.position.y <= previewY + PREVIEW_SIZE + PREVIEW_EXTRA_MARGIN &&
	                        showPatternPreview);

	// Handle file drops
	for(int i = 0; i < dragInfo.files.size(); i++) {
		ofLogNotice() << "File dropped: " << dragInfo.files[i];
		
		// You can check file extensions and handle different file types
		ofFile file(dragInfo.files[i]);
		string extension = ofToLower(file.getExtension());
		
		if(extension == "png" || extension == "jpg" || extension == "jpeg" || extension == "bmp" || extension == "gif" || extension == "webp") {
			ofLogNotice() << "Image file detected: " << dragInfo.files[i];
			ofImage loadedImage;
			
			// Load the image
			if(loadedImage.load(dragInfo.files[i])) {
				ofLogNotice() << "Image loaded successfully: " << loadedImage.getWidth() << "x" << loadedImage.getHeight();
				
				if(isInPreviewArea) {
					// Drop in pattern preview area - add as new dither pattern
					ofLogNotice() << "Adding as new dither pattern";
					addDitherPattern(file.getFileName(), loadedImage);
				} else {
					// Drop elsewhere - replace base image
					ofLogNotice() << "Replacing base image";
					hasBaseImage = true;
					
					// Resize window to match image dimensions
					// int newWidth = loadedImage.getWidth();
					// int newHeight = loadedImage.getHeight();
					// ofSetWindowShape(newWidth, newHeight);

					basePixels = loadedImage.getPixels();
				}
				
				tempSaved = false; // mark for new screenshot
			} else {
				ofLogError() << "Failed to load image: " << dragInfo.files[i];
			}
		}
	}
}
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

void ofApp::addDitherPattern(const std::string& filename, const ofImage& image) {
	DitherPattern newPattern;
	newPattern.filename = filename;
	newPattern.image = image;
	newPattern.area = image.getWidth() * image.getHeight();
	
	// Add the new pattern
	ditherPatterns.push_back(newPattern);
	
	// Re-sort by size
	std::sort(ditherPatterns.begin(), ditherPatterns.end(), [](const DitherPattern& a, const DitherPattern& b) {
		return a.area < b.area;
	});
	
	// Switch to the new pattern
	for(int i = 0; i < ditherPatterns.size(); i++) {
		if(ditherPatterns[i].filename == filename) {
			currentPatternIndex = i;
			break;
		}
	}
	
	ofLogNotice() << "Added new dither pattern: " << filename << " (" << image.getWidth() << "x" << image.getHeight() << ")";
	ofLogNotice() << "Total patterns: " << ditherPatterns.size();
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
	infoLines.push_back("E - Export all patterns");
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
	
	// Position in top-right corner
	int x = ofGetWidth() - PREVIEW_SIZE - PREVIEW_MARGIN;
	int y = PREVIEW_MARGIN;
	
	// Draw dark background
	ofSetColor(40);
	ofDrawRectangle(x - 2, y - 2, PREVIEW_SIZE + 4, PREVIEW_SIZE + 4);
	
	// Draw pattern preview scaled to 64x64 using nearest neighbor
	ofSetColor(255);  // Full white tint
	
	// Set texture filtering to nearest neighbor for crisp pixels
	ditherPatterns[currentPatternIndex].image.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
	ditherPatterns[currentPatternIndex].image.draw(x, y, PREVIEW_SIZE, PREVIEW_SIZE);
	
	// Draw border
	ofSetColor(120);
	ofNoFill();
	ofDrawRectangle(x - 1, y - 1, PREVIEW_SIZE + 2, PREVIEW_SIZE + 2);
	ofFill();
}

void ofApp::exportAllPatterns() {
	if(ditherPatterns.empty()) {
		ofLogWarning() << "No dither patterns to export with";
		return;
	}
	
	// Generate timestamp for this export batch
	auto now = std::time(nullptr);
	std::tm * tmPtr = std::localtime(&now);
	char timeStr[32];
	std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d_%H-%M-%S", tmPtr);
	
	ofLogNotice() << "Exporting current noise with all " << ditherPatterns.size() << " dither patterns...";
	
	// Export with each pattern
	for(int i = 0; i < ditherPatterns.size(); i++) {
		// Start with clean noise pixels
		ofPixels exportPixels = basePixels;
		
		// Apply current pattern
		applyDitherToPixels(exportPixels, ditherPatterns[i]);
		
		// Save this dithered result
		ofImage exportImg;
		exportImg.setFromPixels(exportPixels);
		
		// Create filename: timestamp_patternname.png
		std::string filename = std::string(timeStr) + "_" + ditherPatterns[i].filename;
		exportImg.save(filename);
		
		ofLogNotice() << "Exported: " << filename;
	}
	
	ofLogNotice() << "Export complete! " << ditherPatterns.size() << " images saved.";
}

void ofApp::applyDitherToPixels(ofPixels& targetPixels, const DitherPattern& pattern) {
	// Get pattern properties
	int patternWidth = pattern.image.getWidth();
	int patternHeight = pattern.image.getHeight();
	const ofPixels& patternPixels = pattern.image.getPixels();

	// Apply dithering to all pixels - use actual pixel buffer dimensions
	for (int y = 0; y < targetPixels.getHeight(); y++) {
		for (int x = 0; x < targetPixels.getWidth(); x++) {
			ofColor pixelColor = targetPixels.getColor(x, y);
			int brightness = pixelColor.r; // Grayscale

			// Get corresponding pattern pixel
			int patternX = x % patternWidth;
			int patternY = y % patternHeight;
			ofColor patternColor = patternPixels.getColor(patternX, patternY);
			int threshold = patternColor.r; // Assuming pattern is grayscale

			// Apply dithering
			if (brightness > threshold) {
				targetPixels.setColor(x, y, ofColor(255));
			} else {
				targetPixels.setColor(x, y, ofColor(0));
			}
		}
	}
}

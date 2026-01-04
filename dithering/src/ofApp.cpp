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
	if (!hasPatterns()) {
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
	applyDitherToPixels(pixels, getCurrentPattern());
	
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
	case OF_KEY_UP: // Previous dither pattern (or group with shift)
		if(!hasPatterns()) break;
		if(ofGetKeyPressed(OF_KEY_SHIFT)) {
			// Shift+Up: Previous group
			if(currentGroupIndex > 0) {
				currentGroupIndex--;
				// Keep same index in group if possible, otherwise clamp
				currentPatternIndexInGroup = std::min(currentPatternIndexInGroup, 
					(int)patternGroups[currentGroupIndex].patterns.size() - 1);
			}
		} else {
			// Normal Up: Previous pattern within current group, or go to end of previous group
			if(currentPatternIndexInGroup > 0) {
				currentPatternIndexInGroup--;
			} else if(currentGroupIndex > 0) {
				// Go to end of previous group
				currentGroupIndex--;
				currentPatternIndexInGroup = patternGroups[currentGroupIndex].patterns.size() - 1;
			}
		}
		tempSaved = false;
		break;
	case OF_KEY_DOWN: // Next dither pattern (or group with shift)
		if(!hasPatterns()) break;
		if(ofGetKeyPressed(OF_KEY_SHIFT)) {
			// Shift+Down: Next group
			if(currentGroupIndex < patternGroups.size() - 1) {
				currentGroupIndex++;
				// Keep same index in group if possible, otherwise clamp
				currentPatternIndexInGroup = std::min(currentPatternIndexInGroup, 
					(int)patternGroups[currentGroupIndex].patterns.size() - 1);
			}
		} else {
			// Normal Down: Next pattern within current group, or go to start of next group
			if(currentPatternIndexInGroup < patternGroups[currentGroupIndex].patterns.size() - 1) {
				currentPatternIndexInGroup++;
			} else if(currentGroupIndex < patternGroups.size() - 1) {
				// Go to start of next group
				currentGroupIndex++;
				currentPatternIndexInGroup = 0;
			}
		}
		tempSaved = false;
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
	
	// Clear existing patterns
	patternGroups.clear();
	
	// Load all pattern files
	for(int i = 0; i < dir.size(); i++) {
		std::string filename = dir.getName(i);
		
		ofImage pattern;
		if(pattern.load(patternsPath + filename)) {
			DitherPattern ditherPattern;
			ditherPattern.filename = filename;
			ditherPattern.image = pattern;
			ditherPattern.area = pattern.getWidth() * pattern.getHeight();
			
			// Insert into appropriate group
			insertPatternIntoGroups(ditherPattern);
			ofLogNotice() << "Loaded dither pattern: " << filename;
		} else {
			ofLogError() << "Failed to load: " << filename;
		}
	}
	
	int totalPatterns = 0;
	for(const auto& group : patternGroups) {
		totalPatterns += group.patterns.size();
	}
	
	ofLogNotice() << "Loaded " << totalPatterns << " dither patterns in " << patternGroups.size() << " size groups";
}

void ofApp::addDitherPattern(const std::string& filename, const ofImage& image) {
	DitherPattern newPattern;
	newPattern.filename = filename;
	newPattern.image = image;
	newPattern.area = image.getWidth() * image.getHeight();
	
	// Insert into appropriate group and get the final position
	auto [groupIndex, patternIndex] = insertPatternIntoGroups(newPattern);
	
	// Switch to the new pattern
	currentGroupIndex = groupIndex;
	currentPatternIndexInGroup = patternIndex;
	
	int totalPatterns = 0;
	for(const auto& group : patternGroups) {
		totalPatterns += group.patterns.size();
	}
	
	ofLogNotice() << "Added new dither pattern: " << filename << " (" << image.getWidth() << "x" << image.getHeight() << ")";
	ofLogNotice() << "Total patterns: " << totalPatterns << " in " << patternGroups.size() << " groups";
}

void ofApp::drawInfo() {
	// Build info lines
	std::vector<std::string> infoLines;
	infoLines.push_back("Dither Pattern Tester");
	infoLines.push_back("");
	
	if(hasPatterns()) {
		// Current pattern info
		int totalPatterns = 0;
		for(const auto& group : patternGroups) {
			totalPatterns += group.patterns.size();
		}
		
		int currentPatternGlobal = 1;
		for(int g = 0; g < currentGroupIndex; g++) {
			currentPatternGlobal += patternGroups[g].patterns.size();
		}
		currentPatternGlobal += currentPatternIndexInGroup;
		
		infoLines.push_back("Pattern: " + ofToString(currentPatternGlobal + 1) + "/" + ofToString(totalPatterns));
		
		DitherPattern& current = getCurrentPattern();
		int w = current.image.getWidth();
		int h = current.image.getHeight();
		infoLines.push_back("Size: " + ofToString(w) + "x" + ofToString(h));
		infoLines.push_back("File: " + current.filename);
		
		infoLines.push_back("");
		infoLines.push_back("Pattern Groups (" + ofToString(patternGroups.size()) + " sizes):");
		
		// List all groups in tree format
		for(int g = 0; g < patternGroups.size(); g++) {
			const auto& group = patternGroups[g];
			std::string groupMarker = (g == currentGroupIndex) ? "+" : "-";
			std::string groupLine = groupMarker + " " + ofToString(group.width) + "x" + ofToString(group.height) + " (" + ofToString(group.patterns.size()) + ")";
			infoLines.push_back(groupLine);
			
			// Show patterns in current group
			if(g == currentGroupIndex) {
				for(int p = 0; p < group.patterns.size(); p++) {
					std::string patternMarker = (p == currentPatternIndexInGroup) ? "  * " : "    ";
					infoLines.push_back(patternMarker + group.patterns[p].filename);
				}
			}
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
	infoLines.push_back("shift+up/down - Change group");
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
	int rectWidth = 350; // Increased width for tree structure
	
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
	if(!hasPatterns()) return;
	
	// Position in top-right corner
	int x = ofGetWidth() - PREVIEW_SIZE - PREVIEW_MARGIN;
	int y = PREVIEW_MARGIN;
	
	// Draw dark background
	ofSetColor(40);
	ofDrawRectangle(x - 2, y - 2, PREVIEW_SIZE + 4, PREVIEW_SIZE + 4);
	
	// Draw pattern preview scaled to 64x64 using nearest neighbor
	ofSetColor(255);  // Full white tint
	
	// Set texture filtering to nearest neighbor for crisp pixels
	getCurrentPattern().image.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
	getCurrentPattern().image.draw(x, y, PREVIEW_SIZE, PREVIEW_SIZE);
	
	// Draw border
	ofSetColor(120);
	ofNoFill();
	ofDrawRectangle(x - 1, y - 1, PREVIEW_SIZE + 2, PREVIEW_SIZE + 2);
	ofFill();
}

void ofApp::exportAllPatterns() {
	if(!hasPatterns()) {
		ofLogWarning() << "No dither patterns to export with";
		return;
	}
	
	// Generate timestamp for this export batch
	auto now = std::time(nullptr);
	std::tm * tmPtr = std::localtime(&now);
	char timeStr[32];
	std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d_%H-%M-%S", tmPtr);
	
	int totalPatterns = 0;
	for(const auto& group : patternGroups) {
		totalPatterns += group.patterns.size();
	}
	
	ofLogNotice() << "Exporting current noise with all " << totalPatterns << " dither patterns...";
	
	// Export with each pattern
	for(const auto& group : patternGroups) {
		for(const auto& pattern : group.patterns) {
			// Start with clean noise pixels
			ofPixels exportPixels = basePixels;
			
			// Apply current pattern
			applyDitherToPixels(exportPixels, pattern);
			
			// Save this dithered result
			ofImage exportImg;
			exportImg.setFromPixels(exportPixels);
			
			// Create filename: timestamp_patternname.png
			std::string filename = std::string(timeStr) + "_" + pattern.filename;
			exportImg.save(filename);
			
			ofLogNotice() << "Exported: " << filename;
		}
	}
	
	ofLogNotice() << "Export complete! " << totalPatterns << " images saved.";
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

std::pair<int, int> ofApp::insertPatternIntoGroups(const DitherPattern& pattern) {
	int width = pattern.image.getWidth();
	int height = pattern.image.getHeight();
	
	// Find existing group or create new one
	bool foundGroup = false;
	int groupIndex = -1;
	
	for(int i = 0; i < patternGroups.size(); i++) {
		if(patternGroups[i].width == width && patternGroups[i].height == height) {
			patternGroups[i].patterns.push_back(pattern);
			// Sort patterns within group by filename
			std::sort(patternGroups[i].patterns.begin(), patternGroups[i].patterns.end(), 
				[](const DitherPattern& a, const DitherPattern& b) {
					return a.filename < b.filename;
				});
			groupIndex = i;
			foundGroup = true;
			break;
		}
	}
	
	if(!foundGroup) {
		// Create new group
		PatternGroup newGroup;
		newGroup.width = width;
		newGroup.height = height;
		newGroup.patterns.push_back(pattern);
		patternGroups.push_back(newGroup);
		
		// Re-sort groups by area
		std::sort(patternGroups.begin(), patternGroups.end(), [](const PatternGroup& a, const PatternGroup& b) {
			return (a.width * a.height) < (b.width * b.height);
		});
		
		// Find where the new group ended up
		for(int i = 0; i < patternGroups.size(); i++) {
			if(patternGroups[i].width == width && patternGroups[i].height == height) {
				groupIndex = i;
				break;
			}
		}
	}
	
	// Find the pattern index within the group
	int patternIndex = -1;
	if(groupIndex >= 0) {
		for(int i = 0; i < patternGroups[groupIndex].patterns.size(); i++) {
			if(patternGroups[groupIndex].patterns[i].filename == pattern.filename) {
				patternIndex = i;
				break;
			}
		}
	}
	
	return {groupIndex, patternIndex};
}

DitherPattern& ofApp::getCurrentPattern() {
	return patternGroups[currentGroupIndex].patterns[currentPatternIndexInGroup];
}

bool ofApp::hasPatterns() const {
	return !patternGroups.empty();
}

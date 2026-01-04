#include "ofApp.h"

void ofApp::setup(){
	ofSetWindowTitle("Bayer Dither Matrix Visualizer");
	ofBackground(50);
	initializeBaseMatrices();
	generateBayerMatrix(matrixSize);
}

void ofApp::update(){
}

void ofApp::draw(){
	if(currentMode == CUSTOM_MODE) {
		drawCustomMatrix();
		drawPalette();
	} else if(currentMode == GRADIENT_MODE) {
		drawGradientMatrix();
	} else {
		drawMatrix();
	}
	if(showInfo) drawInfo();
}

void ofApp::keyPressed(int key){
	switch(key){
		case 'r': case 'R':
			tempSaved = false;
			break;
		case 'v': case 'V':
			showInfo = !showInfo;
			tempSaved = false;
			break;
		case 's': case 'S':
			saveTimestamped();
			break;
		case 'e': case 'E':
			saveMatrixImage();
			break;
		case 'i': case 'I':
			showIndices = !showIndices;
			if(showIndices) showValues = false;
			tempSaved = false;
			break;
		case 'n': case 'N':
			showValues = !showValues;
			if(showValues) showIndices = false;
			tempSaved = false;
			break;
		case 'a': case 'A':
			if(currentMode == CUSTOM_MODE) {
				autoMode = !autoMode;
				tempSaved = false;
			}
			break;
		case 'g': case 'G':
			if(currentMode == GRADIENT_MODE) {
				gradientInverted = !gradientInverted;
				generateCircularGradient();
				tempSaved = false;
			}
			break;
		case OF_KEY_F1:
			currentMode = MATRIX_MODE;
			tempSaved = false;
			break;
		case OF_KEY_F2:
			currentMode = CUSTOM_MODE;
			initializeCustomMode();
			tempSaved = false;
			break;
		case OF_KEY_F3:
			currentMode = GRADIENT_MODE;
			initializeGradientMode();
			tempSaved = false;
			break;
		case OF_KEY_UP:
			if(currentMode == MATRIX_MODE && matrixSize < 64) {
				matrixSize *= 2;
				generateBayerMatrix(matrixSize);
				tempSaved = false;
			} else if((currentMode == CUSTOM_MODE || currentMode == GRADIENT_MODE) && matrixSize < 64) {
				matrixSize += 1;
				if(currentMode == CUSTOM_MODE) initializeCustomMode();
				if(currentMode == GRADIENT_MODE) initializeGradientMode();
				tempSaved = false;
			}
			break;
		case OF_KEY_DOWN:
			if(currentMode == MATRIX_MODE && matrixSize > 2) {
				matrixSize /= 2;
				generateBayerMatrix(matrixSize);
				tempSaved = false;
			} else if((currentMode == CUSTOM_MODE || currentMode == GRADIENT_MODE) && matrixSize > 1) {
				matrixSize -= 1;
				if(currentMode == CUSTOM_MODE) initializeCustomMode();
				if(currentMode == GRADIENT_MODE) initializeGradientMode();
				tempSaved = false;
			}
			break;
		case OF_KEY_LEFT:
			if(currentMode == MATRIX_MODE) {
				currentBaseIndex = (currentBaseIndex - 1 + baseMatrices.size()) % baseMatrices.size();
				generateBayerMatrix(matrixSize);
				tempSaved = false;
			}
			break;
		case OF_KEY_RIGHT:
			if(currentMode == MATRIX_MODE) {
				currentBaseIndex = (currentBaseIndex + 1) % baseMatrices.size();
				generateBayerMatrix(matrixSize);
				tempSaved = false;
			}
			break;
		default: break;
	}
}

void ofApp::dragEvent(ofDragInfo dragInfo){}
void ofApp::keyReleased(int key){}
void ofApp::mouseMoved(int x, int y){}
void ofApp::mouseDragged(int x, int y, int button){
	if(currentMode == CUSTOM_MODE && button == 0) { // Left mouse button
		mouseWasReleased = false; // Mark that we're dragging
		handlePatternClick(x, y);
	}
}
void ofApp::mousePressed(int x, int y, int button){
	if (button != 0) return; // Only respond to left mouse button
	mouseWasReleased = true; // Reset on new mouse press
	if(currentMode == MATRIX_MODE) {
		// Switch to custom mode on first mouse down
		currentMode = CUSTOM_MODE;
		initializeCustomMode();
		tempSaved = false;
	} else if(currentMode == CUSTOM_MODE) {
		if(isInPaletteArea(x, y)) {
			handlePaletteClick(x, y);
		} else if(isInPatternArea(x, y)) {
			handlePatternClick(x, y);
		}
	}
}
void ofApp::mouseReleased(int x, int y, int button){
	if (button == 0) { // Left mouse button
		mouseWasReleased = true;
	}
}
void ofApp::mouseEntered(int x, int y){}
void ofApp::mouseExited(int x, int y){}
void ofApp::windowResized(int w, int h){tempSaved=false; }
void ofApp::gotMessage(ofMessage msg){}

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

void ofApp::saveMatrixImage(){
	// Create image with exact matrix dimensions
	ofImage matrixImg;
	matrixImg.allocate(matrixSize, matrixSize, OF_IMAGE_GRAYSCALE);
	
	int totalCells = matrixSize * matrixSize;
	
	for(int y = 0; y < matrixSize; y++) {
		for(int x = 0; x < matrixSize; x++) {
			// Get value from appropriate matrix based on mode
			int value;
			if(currentMode == CUSTOM_MODE) {
				value = customMatrix[y][x];
			} else if(currentMode == GRADIENT_MODE) {
				value = gradientMatrix[y][x];
			} else {
				value = bayerMatrix[y][x];
			}
			
			// Normalize value to 0-255 range
			float normalizedValue = (float)value / (float)(totalCells - 1);
			unsigned char grayValue = (unsigned char)(normalizedValue * 255);
			matrixImg.setColor(x, y, ofColor(grayValue));
		}
	}
	
	// Create filename based on mode
	char buf[128];
	if(currentMode == CUSTOM_MODE) {
		std::snprintf(buf, sizeof(buf), "custom_%dx%d.png", matrixSize, matrixSize);
	} else if(currentMode == GRADIENT_MODE) {
		std::snprintf(buf, sizeof(buf), "gradient_%dx%d%s.png", 
				 matrixSize, matrixSize, gradientInverted ? "_inverted" : "");
	} else {
		// Create safe filename from base matrix name
		std::string baseName = baseMatrices[currentBaseIndex].name;
		std::string safeBaseName;
		for(char c : baseName) {
			if(std::isalnum(c)) {
				safeBaseName += std::tolower(c);
			} else if(c == ' ' || c == '(' || c == ')' || c == '-') {
				safeBaseName += '_';
			}
		}
		std::snprintf(buf, sizeof(buf), "%s_%dx%d.png", 
				 safeBaseName.c_str(), matrixSize, matrixSize);
	}
	
	matrixImg.save(buf);
	ofLogNotice() << "Matrix image saved as: " << buf;
}

void ofApp::generateBayerMatrix(int size) {
	// Resize the matrix
	bayerMatrix.resize(size);
	for(int i = 0; i < size; i++) {
		bayerMatrix[i].resize(size);
	}
	
	// Base case: use selected base matrix
	if(size == 2) {
		const auto& baseMatrix = baseMatrices[currentBaseIndex].matrix;
		for(int i = 0; i < 2; i++) {
			for(int j = 0; j < 2; j++) {
				bayerMatrix[i][j] = baseMatrix[i][j];
			}
		}
		return;
	}
	
	// Recursive case: build from smaller matrix
	int halfSize = size / 2;
	
	// Generate the smaller matrix first
	std::vector<std::vector<int>> smallMatrix(halfSize);
	for(int i = 0; i < halfSize; i++) {
		smallMatrix[i].resize(halfSize);
	}
	
	// Temporarily store current matrix and generate smaller one
	int oldSize = matrixSize;
	matrixSize = halfSize;
	generateBayerMatrix(halfSize);
	
	// Copy the smaller matrix
	for(int i = 0; i < halfSize; i++) {
		for(int j = 0; j < halfSize; j++) {
			smallMatrix[i][j] = bayerMatrix[i][j];
		}
	}
	
	// Restore matrix size
	matrixSize = oldSize;
	bayerMatrix.resize(size);
	for(int i = 0; i < size; i++) {
		bayerMatrix[i].resize(size);
	}
	
	// Build the full matrix using the recursive pattern based on the selected base matrix
	const auto& baseMatrix = baseMatrices[currentBaseIndex].matrix;
	for(int i = 0; i < halfSize; i++) {
		for(int j = 0; j < halfSize; j++) {
			int baseValue = smallMatrix[i][j] * 4;
			// Use the base matrix pattern to determine quadrant arrangement
			// Upper-left quadrant: 4*M + baseMatrix[0][0]
			bayerMatrix[i][j] = baseValue + baseMatrix[0][0];
			// Upper-right quadrant: 4*M + baseMatrix[0][1]
			bayerMatrix[i][j + halfSize] = baseValue + baseMatrix[0][1];
			// Lower-left quadrant: 4*M + baseMatrix[1][0]
			bayerMatrix[i + halfSize][j] = baseValue + baseMatrix[1][0];
			// Lower-right quadrant: 4*M + baseMatrix[1][1]
			bayerMatrix[i + halfSize][j + halfSize] = baseValue + baseMatrix[1][1];
		}
	}
}

void ofApp::drawMatrix() {
	int totalCells = matrixSize * matrixSize;
	
	// Calculate centering offset
	int totalWidth = matrixSize * cellSize;
	int totalHeight = matrixSize * cellSize;
	int offsetX = (ofGetWidth() - totalWidth) / 2;
	int offsetY = (ofGetHeight() - totalHeight) / 2;
	
	// Draw each cell
	for(int y = 0; y < matrixSize; y++) {
		for(int x = 0; x < matrixSize; x++) {
			int cellX = offsetX + x * cellSize;
			int cellY = offsetY + y * cellSize;
			
			// Calculate gray value (0-255)
			float normalizedValue = (float)bayerMatrix[y][x] / (float)totalCells;
			int grayValue = (int)(normalizedValue * 255);
			
			// Draw cell background
			ofSetColor(grayValue);
			ofDrawRectangle(cellX, cellY, cellSize, cellSize);
			
			// Draw text overlay if enabled
			if(showIndices || showValues) {
				ofSetColor(grayValue > 127 ? 0 : 255);
				
				std::string text;
				if(showIndices) {
					text = ofToString(bayerMatrix[y][x]);
				} else if(showValues) {
					text = ofToString(normalizedValue, 3);
				}
				
				// Approximate text centering (bitmap font is roughly 8x11 pixels per character)
				float textWidth = text.length() * 8;
				float textHeight = 11;
				float textX = cellX + (cellSize - textWidth) / 2;
				float textY = cellY + (cellSize + textHeight) / 2;
				
				ofDrawBitmapString(text, textX, textY);
			}
		}
	}
}

void ofApp::drawInfo() {
	// Build info lines
	std::vector<std::string> infoLines;
	infoLines.push_back("Bayer Matrix Visualizer");
	infoLines.push_back("");
	
	if(currentMode == CUSTOM_MODE) {
		infoLines.push_back("Mode: F2 - CUSTOM DRAWING");
		infoLines.push_back("Matrix Size: " + ofToString(matrixSize) + "x" + ofToString(matrixSize));
		infoLines.push_back("Palette Size: " + ofToString(palette.size()) + " grays");
		infoLines.push_back("Auto Mode: " + std::string(autoMode ? "ON" : "OFF"));
		infoLines.push_back("");
		infoLines.push_back("Controls:");
		infoLines.push_back("Click pattern - Draw with selected/auto color");
		infoLines.push_back("Click palette - Select color");
		infoLines.push_back("Drag on pattern - Continuous drawing");
		infoLines.push_back("A - Toggle auto mode");
		infoLines.push_back("up/down - Change matrix size (+/-1)");
	} else if(currentMode == GRADIENT_MODE) {
		infoLines.push_back("Mode: F3 - CIRCULAR GRADIENT");
		infoLines.push_back("Matrix Size: " + ofToString(matrixSize) + "x" + ofToString(matrixSize));
		infoLines.push_back("Gradient: " + std::string(gradientInverted ? "Center=1, Edge=0" : "Center=0, Edge=1"));
		infoLines.push_back("");
		infoLines.push_back("Controls:");
		infoLines.push_back("G - Toggle gradient direction");
		infoLines.push_back("up/down - Change matrix size (+/-1)");
	} else {
		infoLines.push_back("Mode: F1 - PRESET PATTERNS");
		infoLines.push_back("Base Pattern: " + baseMatrices[currentBaseIndex].name);
		infoLines.push_back("Matrix Size: " + ofToString(matrixSize) + "x" + ofToString(matrixSize));
		infoLines.push_back("Total Values: " + ofToString(matrixSize * matrixSize));
		infoLines.push_back("");
		infoLines.push_back("Controls:");
		infoLines.push_back("left/right - Change base pattern");
		infoLines.push_back("up/down - Change matrix size (*2, /2)");
		infoLines.push_back("Click anywhere - Switch to custom drawing");
	}
	
	infoLines.push_back("");
	infoLines.push_back("F1 - Matrix mode | F2 - Custom mode | F3 - Gradient mode");
	infoLines.push_back("I - Toggle indices " + std::string(showIndices ? "[ON]" : "[OFF]"));
	infoLines.push_back("N - Toggle values " + std::string(showValues ? "[ON]" : "[OFF]"));
	infoLines.push_back("S - Save screenshot | E - Export matrix image");
	infoLines.push_back("V - Toggle this help");
	
	// Calculate dynamic size
	int lineHeight = 14; // approximate line height for bitmap font
	int padding = 20;
	int rectHeight = infoLines.size() * lineHeight + padding;
	int rectWidth = 380; // increase width for longer lines
	
	// Semi-transparent background
	ofSetColor(0, 0, 0, 180);
	ofDrawRectangle(10, 10, rectWidth, rectHeight);
	
	// Draw each line
	ofSetColor(255);
	for(int i = 0; i < infoLines.size(); i++) {
		ofDrawBitmapString(infoLines[i], 20, 30 + i * lineHeight);
	}
}

void ofApp::initializeBaseMatrices() {
	// Clear any existing base matrices
	baseMatrices.clear();
	
	// Classic Bayer cross pattern
	BaseMatrix classicBayer;
	classicBayer.name = "Classic Bayer (Cross)";
	classicBayer.matrix = {
		{0, 2},
		{3, 1}
	};
	baseMatrices.push_back(classicBayer);
	
	// Horizontal pattern - increases left to right
	BaseMatrix horizontalBayer;
	horizontalBayer.name = "Horizontal Lines";
	horizontalBayer.matrix = {
		{0, 1},
		{2, 3}
	};
	baseMatrices.push_back(horizontalBayer);
	
	// Vertical pattern - increases top to bottom
	BaseMatrix verticalBayer;
	verticalBayer.name = "Vertical Lines";
	verticalBayer.matrix = {
		{0, 2},
		{1, 3}
	};
	baseMatrices.push_back(verticalBayer);
	
	// Diagonal pattern - diagonal stripe
	BaseMatrix diagonalBayer;
	diagonalBayer.name = "Diagonal Stripe";
	diagonalBayer.matrix = {
		{0, 3},
		{1, 2}
	};
	baseMatrices.push_back(diagonalBayer);
	
	// Anti-diagonal pattern
	BaseMatrix antiDiagonalBayer;
	antiDiagonalBayer.name = "Anti-Diagonal";
	antiDiagonalBayer.matrix = {
		{2, 1},
		{3, 0}
	};
	baseMatrices.push_back(antiDiagonalBayer);
	
	// Make sure we have a valid current index
	currentBaseIndex = 0;
}

void ofApp::initializeCustomMode() {
	// Initialize custom matrix with current matrix size
	customMatrix.resize(matrixSize);
	for(int i = 0; i < matrixSize; i++) {
		customMatrix[i].resize(matrixSize, 0); // Initialize with 0s
	}
	
	// Create palette indices (0 to totalCells-1)
	int totalCells = matrixSize * matrixSize;
	palette.clear();
	for(int i = 0; i < totalCells; i++) {
		palette.push_back(i);
	}
	
	selectedPaletteIndex = 0;
	autoMode = false;
	autoModeIndex = 0;
	
	// Reset tracking variables
	lastPaintedX = -1;
	lastPaintedY = -1;
	mouseWasReleased = true;
}

void ofApp::drawCustomMatrix() {
	int totalCells = matrixSize * matrixSize;
	
	// Calculate centering offset for pattern
	int totalWidth = matrixSize * cellSize;
	int totalHeight = matrixSize * cellSize;
	int offsetX = (ofGetWidth() - totalWidth) / 2;
	int offsetY = (ofGetHeight() - totalHeight) / 2 - 50; // Move up to make room for palette
	
	// Draw each cell
	for(int y = 0; y < matrixSize; y++) {
		for(int x = 0; x < matrixSize; x++) {
			int cellX = offsetX + x * cellSize;
			int cellY = offsetY + y * cellSize;
			
			// Calculate gray value from index (0-255)
			float normalizedValue = (float)customMatrix[y][x] / (float)(totalCells - 1);
			int grayValue = (int)(normalizedValue * 255);
			
			// Draw cell background
			ofSetColor(grayValue);
			ofDrawRectangle(cellX, cellY, cellSize, cellSize);
			
			// Draw border for better visibility
			if(x == lastPaintedX && y == lastPaintedY) {
				ofSetColor(0, 255, 0); // Green highlight
			} else {
				ofSetColor(128);
			}
			ofNoFill();
			ofDrawRectangle(cellX, cellY, cellSize, cellSize);
			ofFill();
			
			// Draw text overlay if enabled
			if(showIndices || showValues) {
				ofSetColor(grayValue > 127 ? 0 : 255);
				
				std::string text;
				if(showIndices) {
					text = ofToString(customMatrix[y][x]);
				} else if(showValues) {
					text = ofToString(normalizedValue, 3);
				}
				
				// Approximate text centering
				float textWidth = text.length() * 8;
				float textHeight = 11;
				float textX = cellX + (cellSize - textWidth) / 2;
				float textY = cellY + (cellSize + textHeight) / 2;
				
				ofDrawBitmapString(text, textX, textY);
			}
		}
	}
}

void ofApp::drawPalette() {
	int totalCells = matrixSize * matrixSize;
	int paletteSize = palette.size();
	
	// Calculate palette layout
	int maxPaletteWidth = ofGetWidth() - 40; // Leave margins
	int paletteCellSize = std::min(20, maxPaletteWidth / paletteSize);
	int paletteWidth = paletteSize * paletteCellSize;
	
	// Center palette horizontally at bottom of screen
	int paletteX = (ofGetWidth() - paletteWidth) / 2;
	int paletteY = ofGetHeight() - 80;
	
	// Draw palette background
	ofSetColor(40);
	ofDrawRectangle(paletteX - 10, paletteY - 10, paletteWidth + 20, paletteCellSize + 20);
	
	// Draw palette cells
	for(int i = 0; i < paletteSize; i++) {
		int cellX = paletteX + i * paletteCellSize;
		int cellY = paletteY;
		
		// Calculate gray value for this palette index
		float normalizedValue = (float)palette[i] / (float)(totalCells - 1);
		int grayValue = (int)(normalizedValue * 255);
		
		// Draw cell
		ofSetColor(grayValue);
		ofDrawRectangle(cellX, cellY, paletteCellSize, paletteCellSize);
		
		// Highlight selected cell or auto mode current index
		if(autoMode && i == autoModeIndex) {
			// Blue border for auto mode current index
			ofSetColor(0, 100, 255); // Blue
			ofNoFill();
			ofSetLineWidth(3);
			ofDrawRectangle(cellX, cellY, paletteCellSize, paletteCellSize);
			ofFill();
			ofSetLineWidth(1);
		} else if(!autoMode && i == selectedPaletteIndex) {
			// Red border for manually selected
			ofSetColor(255, 0, 0); // Red border for selected
			ofNoFill();
			ofSetLineWidth(2);
			ofDrawRectangle(cellX, cellY, paletteCellSize, paletteCellSize);
			ofFill();
			ofSetLineWidth(1);
		} else {
			// Regular border
			ofSetColor(128);
			ofNoFill();
			ofDrawRectangle(cellX, cellY, paletteCellSize, paletteCellSize);
			ofFill();
		}
	}
	
	// Draw palette info
	ofSetColor(255);
	std::string paletteInfo = "Palette - Selected: " + ofToString(selectedPaletteIndex) + 
							 " (" + ofToString((int)((float)palette[selectedPaletteIndex] / (float)(totalCells - 1) * 100)) + "% gray)";
	if(autoMode) paletteInfo += " - AUTO MODE";
	ofDrawBitmapString(paletteInfo, paletteX, paletteY + paletteCellSize + 25);
}

void ofApp::handlePaletteClick(int x, int y) {
	int totalCells = matrixSize * matrixSize;
	int paletteSize = palette.size();
	
	// Calculate palette layout (same as in drawPalette)
	int maxPaletteWidth = ofGetWidth() - 40;
	int paletteCellSize = std::min(20, maxPaletteWidth / paletteSize);
	int paletteWidth = paletteSize * paletteCellSize;
	int paletteX = (ofGetWidth() - paletteWidth) / 2;
	int paletteY = ofGetHeight() - 80;
	
	// Check if click is within palette area
	if(x >= paletteX && x < paletteX + paletteWidth && 
	   y >= paletteY && y < paletteY + paletteCellSize) {
		
		// Calculate which palette cell was clicked
		int paletteIndex = (x - paletteX) / paletteCellSize;
		if(paletteIndex >= 0 && paletteIndex < paletteSize) {
			selectedPaletteIndex = paletteIndex;
			tempSaved = false;
		}
	}
}

void ofApp::handlePatternClick(int x, int y) {
	// Calculate pattern area (same as in drawCustomMatrix)
	int totalWidth = matrixSize * cellSize;
	int totalHeight = matrixSize * cellSize;
	int offsetX = (ofGetWidth() - totalWidth) / 2;
	int offsetY = (ofGetHeight() - totalHeight) / 2 - 50;
	
	// Check if click is within pattern area
	if(x >= offsetX && x < offsetX + totalWidth && 
	   y >= offsetY && y < offsetY + totalHeight) {
		
		// Calculate which matrix cell was clicked
		int matrixX = (x - offsetX) / cellSize;
		int matrixY = (y - offsetY) / cellSize;
		
		if(matrixX >= 0 && matrixX < matrixSize && matrixY >= 0 && matrixY < matrixSize) {
			// Check if we should prevent duplicate painting in auto mode
			if(autoMode && !mouseWasReleased && 
			   matrixX == lastPaintedX && matrixY == lastPaintedY) {
				return; // Don't paint again in the same cell while dragging
			}
			
			// Paint the cell
			if(autoMode) {
				// Auto mode: cycle through palette
				customMatrix[matrixY][matrixX] = palette[autoModeIndex];
				autoModeIndex = (autoModeIndex + 1) % palette.size();
			} else {
				// Manual mode: use selected palette color
				customMatrix[matrixY][matrixX] = palette[selectedPaletteIndex];
			}
			
			// Track the last painted cell for highlighting and duplicate prevention
			lastPaintedX = matrixX;
			lastPaintedY = matrixY;
			mouseWasReleased = false; // Mark that we've painted while dragging
			
			tempSaved = false;
		}
	}
}

bool ofApp::isInPaletteArea(int x, int y) {
	int totalCells = matrixSize * matrixSize;
	int paletteSize = palette.size();
	int maxPaletteWidth = ofGetWidth() - 40;
	int paletteCellSize = std::min(20, maxPaletteWidth / paletteSize);
	int paletteWidth = paletteSize * paletteCellSize;
	int paletteX = (ofGetWidth() - paletteWidth) / 2;
	int paletteY = ofGetHeight() - 80;
	
	return (x >= paletteX && x < paletteX + paletteWidth && 
			y >= paletteY && y < paletteY + paletteCellSize);
}

bool ofApp::isInPatternArea(int x, int y) {
	int totalWidth = matrixSize * cellSize;
	int totalHeight = matrixSize * cellSize;
	int offsetX = (ofGetWidth() - totalWidth) / 2;
	int offsetY = (ofGetHeight() - totalHeight) / 2 - 50;
	
	return (x >= offsetX && x < offsetX + totalWidth && 
			y >= offsetY && y < offsetY + totalHeight);
}

void ofApp::initializeGradientMode() {
	// Initialize gradient matrix with current matrix size
	gradientMatrix.resize(matrixSize);
	for(int i = 0; i < matrixSize; i++) {
		gradientMatrix[i].resize(matrixSize);
	}
	
	// Generate the circular gradient
	generateCircularGradient();
	
	// Reset tracking variables (in case we came from custom mode)
	lastPaintedX = -1;
	lastPaintedY = -1;
	mouseWasReleased = true;
}

void ofApp::generateCircularGradient() {
	int totalCells = matrixSize * matrixSize;
	float centerX = (matrixSize - 1) / 2.0f;
	float centerY = (matrixSize - 1) / 2.0f;
	
	// Calculate the maximum distance from center to corner
	float maxDistance = sqrt(centerX * centerX + centerY * centerY);
	
	for(int y = 0; y < matrixSize; y++) {
		for(int x = 0; x < matrixSize; x++) {
			// Calculate distance from center
			float dx = x - centerX;
			float dy = y - centerY;
			float distance = sqrt(dx * dx + dy * dy);
			
			// Normalize distance to 0-1 range
			float normalizedDistance = distance / maxDistance;
			
			// Apply inversion if needed
			if(gradientInverted) {
				normalizedDistance = 1.0f - normalizedDistance;
			}
			
			// Convert to index (0 to totalCells-1)
			int index = (int)(normalizedDistance * (totalCells - 1));
			gradientMatrix[y][x] = index;
		}
	}
}

void ofApp::drawGradientMatrix() {
	int totalCells = matrixSize * matrixSize;
	
	// Calculate centering offset for pattern
	int totalWidth = matrixSize * cellSize;
	int totalHeight = matrixSize * cellSize;
	int offsetX = (ofGetWidth() - totalWidth) / 2;
	int offsetY = (ofGetHeight() - totalHeight) / 2;
	
	// Draw each cell
	for(int y = 0; y < matrixSize; y++) {
		for(int x = 0; x < matrixSize; x++) {
			int cellX = offsetX + x * cellSize;
			int cellY = offsetY + y * cellSize;
			
			// Calculate gray value from index (0-255)
			float normalizedValue = (float)gradientMatrix[y][x] / (float)(totalCells - 1);
			int grayValue = (int)(normalizedValue * 255);
			
			// Draw cell background
			ofSetColor(grayValue);
			ofDrawRectangle(cellX, cellY, cellSize, cellSize);
			
			// Draw border for better visibility
			ofSetColor(128);
			ofNoFill();
			ofDrawRectangle(cellX, cellY, cellSize, cellSize);
			ofFill();
			
			// Draw text overlay if enabled
			if(showIndices || showValues) {
				ofSetColor(grayValue > 127 ? 0 : 255);
				
				std::string text;
				if(showIndices) {
					text = ofToString(gradientMatrix[y][x]);
				} else if(showValues) {
					text = ofToString(normalizedValue, 3);
				}
				
				// Approximate text centering
				float textWidth = text.length() * 8;
				float textHeight = 11;
				float textX = cellX + (cellSize - textWidth) / 2;
				float textY = cellY + (cellSize + textHeight) / 2;
				
				ofDrawBitmapString(text, textX, textY);
			}
		}
	}
}

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
	drawMatrix();
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
		case OF_KEY_UP:
			if(matrixSize < 64) {
				matrixSize *= 2;
				generateBayerMatrix(matrixSize);
				tempSaved = false;
			}
			break;
		case OF_KEY_DOWN:
			if(matrixSize > 2) {
				matrixSize /= 2;
				generateBayerMatrix(matrixSize);
				tempSaved = false;
			}
			break;
		case OF_KEY_LEFT:
			currentBaseIndex = (currentBaseIndex - 1 + baseMatrices.size()) % baseMatrices.size();
			generateBayerMatrix(matrixSize);
			tempSaved = false;
			break;
		case OF_KEY_RIGHT:
			currentBaseIndex = (currentBaseIndex + 1) % baseMatrices.size();
			generateBayerMatrix(matrixSize);
			tempSaved = false;
			break;
		default: break;
	}
}

void ofApp::dragEvent(ofDragInfo dragInfo){}
void ofApp::keyReleased(int key){}
void ofApp::mouseMoved(int x, int y){}
void ofApp::mouseDragged(int x, int y, int button){}
void ofApp::mousePressed(int x, int y, int button){}
void ofApp::mouseReleased(int x, int y, int button){}
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
			// Normalize value to 0-255 range
			float normalizedValue = (float)bayerMatrix[y][x] / (float)totalCells;
			unsigned char grayValue = (unsigned char)(normalizedValue * 255);
			matrixImg.setColor(x, y, ofColor(grayValue));
		}
	}
	
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
	
	// Save image with descriptive filename including base matrix name
	char buf[128];
	std::snprintf(buf, sizeof(buf), "%s_%dx%d.png", 
			 safeBaseName.c_str(), matrixSize, matrixSize);
	
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
	infoLines.push_back("Base Pattern: " + baseMatrices[currentBaseIndex].name);
	infoLines.push_back("Matrix Size: " + ofToString(matrixSize) + "x" + ofToString(matrixSize));
	infoLines.push_back("Total Values: " + ofToString(matrixSize * matrixSize));
	infoLines.push_back("");
	infoLines.push_back("Controls:");
	infoLines.push_back("left/right - Change base pattern");
	infoLines.push_back("up/down - Change matrix size");
	infoLines.push_back("I - Toggle indices " + std::string(showIndices ? "[ON]" : "[OFF]"));
	infoLines.push_back("N - Toggle values " + std::string(showValues ? "[ON]" : "[OFF]"));
	infoLines.push_back("S - Save screenshot");
	infoLines.push_back("E - Export matrix image");
	infoLines.push_back("V - Toggle this help");
	
	// Calculate dynamic size
	int lineHeight = 14; // approximate line height for bitmap font
	int padding = 20;
	int rectHeight = infoLines.size() * lineHeight + padding;
	int rectWidth = 300; // keep width fixed or could calculate from longest line
	
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

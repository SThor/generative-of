#pragma once

#include "ofMain.h"

class ofApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

	private:
		bool showInfo = true;           // draw small help overlay
		std::string tempImagePath = "final_frame_temp.png";
		bool tempSaved = false;
		
		// Mode system
		enum Mode {
			MATRIX_MODE = 0,    // F1 - Matrix-based patterns
			CUSTOM_MODE = 1,    // F2 - Custom painting
			GRADIENT_MODE = 2   // F3 - Circular gradient
		};
		Mode currentMode = MATRIX_MODE;
		
		// Custom drawing mode
		std::vector<std::vector<int>> customMatrix;
		std::vector<int> palette;        // palette indices (0 to paletteSize-1)
		int selectedPaletteIndex = 0;
		bool autoMode = false;
		int autoModeIndex = 0;
		
		// Circular gradient mode
		std::vector<std::vector<int>> gradientMatrix;
		bool gradientInverted = false;   // false: 0 at center, true: 1 at center
		
		// Mouse state tracking for drag prevention
		int lastPaintedX = -1;
		int lastPaintedY = -1;
		bool mouseWasReleased = true;

		void saveTimestamped();
		void saveMatrixImage();
		
		// Base matrix patterns
		struct BaseMatrix {
			std::string name;
			std::vector<std::vector<int>> matrix;
		};
		std::vector<BaseMatrix> baseMatrices;
		int currentBaseIndex = 0;
		void initializeBaseMatrices();
		
		// Bayer matrix parameters
		int matrixSize = 2;           // n for n×n matrix (powers of 2)
		std::vector<std::vector<int>> bayerMatrix;
		
		// Display options
		bool showIndices = false;      // show matrix indices
		bool showValues = false;       // show normalized values (0-1)
		int cellSize = 40;            // size of each cell in pixels
		
		// Helper functions
		void generateBayerMatrix(int size);
		void drawMatrix();
		void drawInfo();
		
		// Custom drawing functions
		void initializeCustomMode();
		void drawPalette();
		void drawCustomMatrix();
		void handlePaletteClick(int x, int y);
		void handlePatternClick(int x, int y);
		bool isInPaletteArea(int x, int y);
		bool isInPatternArea(int x, int y);
		
		// Gradient mode functions
		void initializeGradientMode();
		void generateCircularGradient();
		void drawGradientMatrix();
};

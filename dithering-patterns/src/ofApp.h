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

		void saveTimestamped();
		void saveMatrixImage();
		
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
};

#pragma once

#include "ofMain.h"
#include "ofxGui.h"

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
		bool mShowInfo = true;           // draw small help overlay
		bool mShowGui = true;            // show GUI panel
		std::string tempImagePath = "final_frame_temp.png";
		bool tempSaved = false;

		void buildGradient();
		void pickPalette();
		void saveTimestamped();
		// Core data structures synced FROM parameters
		std::vector<std::array<ofColor,3>> palettes;
		std::vector<std::string> paletteNames;
		int activePaletteIndex = -1;
		// Simple gradient endpoints (top and bottom)
		ofColor gradientTop {0,0,0};
		ofColor gradientBottom {0,0,0};
		
		// Shader for halftone gradient effect
		ofShader halftoneShader;
		ofFbo gradientFbo;
		bool shaderLoaded = false;
		bool useHalftoneEffect = true;

		// Additional FBO and shader for post-processing dither approach
		ofFbo originalGradientFbo;  // Store original gradient
		ofShader ditherShader;      // New post-processing shader
		bool ditherShaderLoaded = false;

		// GUI parameters
		ofParameterGroup params;
		ofParameter<int> paletteIndexParam; // index into palettes
		ofParameterGroup allPalettesGroup; // shows all palettes' colors
		std::vector<std::array<ofParameter<ofColor>,3>> allPaletteColorParams;
		
		// Halftone effect parameters
		ofParameterGroup halftoneGroup;
		ofParameter<float> noiseScale;
		ofParameter<float> randomSeed;
		
		// New dither parameters
		ofParameterGroup ditherGroup;
		ofParameter<float> dotRadius;
		ofParameter<int> samplesPerPixel;
		ofParameter<bool> enableDithering;
		
		ofxPanel gui;

		// Helpers
		void onPaletteIndexChanged(int& idx);
		void onAnyAllPaletteColorChanged(ofColor& c);
		void onHalftoneParamChanged(float& param);
		
		// Core data sync functions
		void syncParametersToCore();
		void initializeDefaultPalettes();
		
		// Get current palette colors from parameters
		std::array<ofColor,3> getCurrentPalette();
};

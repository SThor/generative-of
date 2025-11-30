// c:/Users/Paul/dev/open-frameworks/of_v0.12.1_msys2_mingw64_release/apps/generative-of/screenprint/src/circle.cpp
#include "circle.h"
#include <cmath>
#include <ofMain.h>

Circle::Circle(ofVec2f * position, float radius, int vertexCount, ofColor color, float wiggleFactor)
	: Shape(position, color, wiggleFactor)
	, mRadius(radius)
	, mVertexCount(vertexCount)
	, mCirclePath(new ofPath()) { // Default color white
	update(0.0f);
}

Circle::~Circle() {
	delete mCirclePath;
}

void Circle::draw() const {
	ofPushMatrix();
	ofTranslate(mPosition->x, mPosition->y);
    mCirclePath->setFillColor(mColor);
	mCirclePath->draw();
	ofPopMatrix();
}

void Circle::update(float timeElapsed) {
	mCirclePath->clear();
	float wiggle = 0.2f * timeElapsed;
	
	// Use position to create unique noise offset for each circle
	float noiseOffsetX = mPosition->x * 0.01f; // Scale down position for noise
	float noiseOffsetY = mPosition->y * 0.01f;
	
	for (int i = 0; i <= mVertexCount; ++i) {
		float angle = ofMap(i, 0, mVertexCount, 0, TWO_PI);
		
		// Incorporate position into noise calculation for unique variation per circle
		float noiseValue = ofNoise(cos(angle) + noiseOffsetX, sin(angle) + noiseOffsetY, wiggle);
		float radiusVariation = ofMap(noiseValue, 0, 1, -mRadius * mWiggleFactor, mRadius * mWiggleFactor);
		float currentRadius = mRadius + radiusVariation;

		float x = currentRadius * cos(angle);
		float y = currentRadius * sin(angle);
		if (i == 0) {
			mCirclePath->moveTo(x, y);
		} else {
			mCirclePath->curveTo(x, y);
		}
	}
}

void Circle::setRadius(float r) {
	mRadius = r;
}

float Circle::getRadius() const {
	return mRadius;
}
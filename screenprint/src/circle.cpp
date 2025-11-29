// c:/Users/Paul/dev/open-frameworks/of_v0.12.1_msys2_mingw64_release/apps/generative-of/screenprint/src/circle.cpp
#include "circle.h"
#include <cmath>
#include <ofMain.h>

Circle::Circle(ofVec2f * position, float radius, int vertexCount, float wiggleFactor)
	: Shape(position, wiggleFactor)
	, mRadius(radius)
	, mVertexCount(vertexCount)
	, mCirclePath(new ofPath()) {
	update(0.0f);
}

Circle::~Circle() = default;

void Circle::draw() const {
	ofPushMatrix();
	ofTranslate(mPosition->x, mPosition->y);
	mCirclePath->draw();
	ofPopMatrix();
}

void Circle::update(float timeElapsed) {
	mCirclePath->clear();
	float wiggle = 0.2f * timeElapsed;
	for (int i = 0; i <= mVertexCount; ++i) {
		float angle = ofMap(i, 0, mVertexCount, 0, TWO_PI);
		float noiseValue = ofNoise(cos(angle), sin(angle), wiggle);
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
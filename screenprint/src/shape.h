#pragma once

#include <ofMain.h>

class Shape {
public:
	Shape(ofVec2f * position, ofColor color, float wiggleFactor = 0.1f)
		: mPosition(position)
		, mColor(color)
		, mWiggleFactor(wiggleFactor) { }
	virtual ~Shape() { delete mPosition; }

	// lifecycle
	virtual void update(float timeElapsed) = 0;
	virtual void draw() const = 0;

	// color accessors
	virtual void setColor(const ofColor & color) { mColor = color; }
	virtual ofColor getColor() const { return mColor; }

	// wiggle factor accessors
	void setWiggleFactor(float wf) { mWiggleFactor = wf; }
	float getWiggleFactor() const { return mWiggleFactor; }

	// position accessors
	void setPosition(float x, float y) {
		mPosition->x = x;
		mPosition->y = y;
	}
	void setPosition(const ofVec2f & pos) {
		mPosition->x = pos.x;
		mPosition->y = pos.y;
	}
	ofVec2f * getPosition() const { return mPosition; }

protected:
	ofVec2f * mPosition;
	ofColor mColor;
	float mWiggleFactor;
};
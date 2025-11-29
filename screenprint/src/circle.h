#pragma once

#include "shape.h"
#include <ofMain.h>

class Circle : public Shape {
public:
	Circle(ofVec2f * position, float radius, int vertexCount, float wiggleFactor = 0.1f);
	~Circle() override;

	// Shape interface
	void draw() const override;
	void update(float timeElapsed) override;

	void setRadius(float r);
	float getRadius() const;

private:
	float mRadius;
	int mVertexCount;

	ofPath * mCirclePath;
};

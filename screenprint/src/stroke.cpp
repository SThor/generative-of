#include "stroke.h"

Stroke::Stroke(ofVec2f * position, ofVec2f * endPoint, float width, ofColor color, float wiggleFactor)
    : Shape(position, color, wiggleFactor)
    , mEndPoint(endPoint)
    , mWidth(width)
    , mControlPoints()
    , mStrokePath(new ofPath())
    , mDebug(false) {
    update(0.0f);
}

Stroke::~Stroke() {
    delete mStrokePath;
}

void Stroke::draw() const {	
	// Set color and draw the stroke as a filled shape
	ofPushStyle();
    mStrokePath->setFillColor(mColor);
    mStrokePath->setFilled(true);
	mStrokePath->draw();
	ofPopStyle();

    // Optionally, draw control points for debugging
    if (mDebug) {
        ofPushStyle();
        ofSetColor(ofColor::red);
        ofDrawLine(*mPosition, *mEndPoint);

        ofSetColor(ofColor::blue);
        for (int i = 0; i < mControlPoints.size(); ++i) {
            ofVec2f pt = mControlPoints[i];
            ofDrawCircle(pt, 2.0f);
        }
        ofPopStyle();
    }
}
void Stroke::update(float timeElapsed) {
    // Clear previous stroke data
    mStrokePath->clear();

    // Calculate direction vector from position to end point
    ofVec2f direction = *mEndPoint - *mPosition;
    float length = direction.length();
    direction.normalize();
    
    // Calculate perpendicular vector for creating width
    ofVec2f perpendicular(-direction.y, direction.x);
    perpendicular *= mWidth * 0.5f; // Half width for each side

    // Adaptive vertex count based on length (1 vertex per ~20 pixels)
    int adaptiveVertexCount = ofClamp(length / 20.0f, 5, 50);

    // Base wiggle amount for consistent variation
    float wiggle = 0.2f * timeElapsed;

    // Create centerline points with wiggle
    mControlPoints.clear();
    for (int i = 0; i <= adaptiveVertexCount; ++i) {
        float t = ofMap(i, 0, adaptiveVertexCount, 0, 1);
        
        // Interpolate along the line from position to end point
        ofVec2f basePoint = *mPosition + direction * (t * length);
        
        // Add bidirectional wiggle perpendicular to the direction
        // Wiggle increases along the stroke (more pronounced towards the end)
        float wiggleMultiplier = t * t * t; // Cubic increase: 0 at start, 1 at end
        float noiseValue = ofNoise(t * 8, wiggle);
        float wiggleAmount = ofMap(noiseValue, 0, 1, -1, 1) * mWiggleFactor * wiggleMultiplier;
        ofVec2f centerPoint = basePoint + direction.getPerpendicular() * wiggleAmount;
        
        mControlPoints.push_back(centerPoint);
    }

    // Create filled contour by going along one side, then back along the other
    if (!mControlPoints.empty()) {
        mStrokePath->moveTo(mControlPoints[0] + perpendicular);
        
        // Draw top edge
        for (int i = 1; i < mControlPoints.size(); ++i) {
            mStrokePath->curveTo(mControlPoints[i] + perpendicular);
        }
        
        // Draw bottom edge (in reverse)
        for (int i = mControlPoints.size() - 1; i >= 0; --i) {
            mStrokePath->curveTo(mControlPoints[i] - perpendicular);
        }
        
        // Close the path
        mStrokePath->close();
    }
}

void Stroke::setDebug(bool debug) {
    mDebug = debug;
}

bool Stroke::getDebug() const {
    return mDebug;
}
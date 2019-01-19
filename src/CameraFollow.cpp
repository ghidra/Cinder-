#include "CameraFollow.h"

#include "cinder/Log.h"

using namespace std;
using namespace ci;
using namespace ci::app;

CameraFollow::CameraFollow(vec3 offset, float distance)
	: mDistance(distance)
	, mUp(vec3(0,1,0))
	, mCenter(vec3())
	, mLocalDeltaTime(0.0f)
	, mOffset(offset)
{
	mEye = mOffset * mDistance;//putting it in negative z makes it look down positive z
	mPersp.setPerspective( 75.0f, getWindowAspectRatio(), 5.0f, 2000.0f );

	mPersp.lookAt( mEye, mCenter, mUp );
}

void CameraFollow::Update(const vec3 worldPosition, const float deltaTime)
{
	
	mLocalDeltaTime = deltaTime;
	vec3 pos = worldPosition + (mOffset*mDistance);
	mPersp.setEyePoint(pos);
	mPersp.lookAt(pos, worldPosition, mUp);
}

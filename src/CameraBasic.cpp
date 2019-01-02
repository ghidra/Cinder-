#include "CameraBasic.h"

#include "cinder/Log.h"

using namespace std;
using namespace ci;
using namespace ci::app;

CameraBasic::CameraBasic(float distance)
	: mDistance(distance)
	, mUp(vec3(0,1,0))
	, mCenter(vec3())
{
	mEye		= vec3( 0.0f, 0.0f, mDistance );
	mPersp.setPerspective( 75.0f, getWindowAspectRatio(), 5.0f, 2000.0f );
	//Setup(500.0f);
}
// void CameraBasic::Setup(float distance)
// {
// 	mDistance = distance;
// 	mEye = vec3( 0.0f, 0.0f, mDistance );
// 	mPersp.setPerspective( 75.0f, getWindowAspectRatio(), 5.0f, 2000.0f );
// }

void CameraBasic::Update(float deltaTime, float elapsedTime)
{
	mEye = vec3( 0.0f, 0.0f, mDistance );
	mPersp.lookAt( mEye, mCenter, mUp );
}

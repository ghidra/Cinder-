#include "CameraBasic.h"

#include "cinder/Log.h"

using namespace std;
using namespace ci;
using namespace ci::app;

CameraBasic::CameraBasic(float distance)
	: mDistance(distance)
	, mUp(vec3(0,1,0))
	, mCenter(vec3())
	, mMovementSpeed(10.0f)
	, mLocalDeltaTime(0.0f)
	, mMovingForward(false)
	, mMovingBackward(false)
	, mMovingLeft(false)
	, mMovingRight(false)
	, mMovingUpward(false)
	, mMovingDownward(false)
{
	mEye		= vec3( 0.0f, 0.0f, mDistance );
	mPersp.setPerspective( 75.0f, getWindowAspectRatio(), 5.0f, 2000.0f );
	//Setup(500.0f);
	mPersp.lookAt( mEye, mCenter, mUp );
}
// void CameraBasic::Setup(float distance)
// {
// 	mDistance = distance;
// 	mEye = vec3( 0.0f, 0.0f, mDistance );
// 	mPersp.setPerspective( 75.0f, getWindowAspectRatio(), 5.0f, 2000.0f );
// }

void CameraBasic::Update(float deltaTime)
{
	
	mLocalDeltaTime = deltaTime;

	vec3 direction = vec3();
	vec3 viewDirection = mPersp.getViewDirection();
	vec3 viewUp = mPersp.getWorldUp();
	vec3 viewSide = glm::normalize(glm::cross(viewUp,viewDirection));

	if( mMovingForward )
		direction += viewDirection;
	if( mMovingLeft )
		direction += viewSide;
	if( mMovingBackward )
		direction -= viewDirection;
	if( mMovingRight )
		direction -= viewSide;
	if( mMovingUpward )
		direction += viewUp;
	if( mMovingDownward )
		direction -= viewUp;

	if( glm::length(direction)>0.0)
	{
		direction = glm::normalize(direction)*(mLocalDeltaTime*mMovementSpeed);

		mPersp.setEyePoint( mPersp.getEyePoint()+direction );
	}
	//CI_LOG_I("WE SHOULD BE MOVING THE CAMERA");
}

void CameraBasic::keyDown(KeyEvent event)
{
	if( event.getChar() == 'w' )
		mMovingForward=true;
	if( event.getChar() == 'a' )
		mMovingLeft=true;
	if( event.getChar() == 's' )
		mMovingBackward=true;
	if( event.getChar() == 'd' )
		mMovingRight=true;
	if( event.getChar() == 'q' )
		mMovingUpward=true;
	if( event.getChar() == 'e' )
		mMovingDownward=true;
}

void CameraBasic::keyUp( KeyEvent event )
{
	if( event.getChar() == 'w' )
		mMovingForward=false;
	if( event.getChar() == 'a' )
		mMovingLeft=false;
	if( event.getChar() == 's' )
		mMovingBackward=false;
	if( event.getChar() == 'd' )
		mMovingRight=false;
	if( event.getChar() == 'q' )
		mMovingUpward=false;
	if( event.getChar() == 'e' )
		mMovingDownward=false;
}

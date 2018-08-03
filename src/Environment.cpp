/*
 RenderKit
 
 Copyright (c) 2016, Simon Geilfus, All rights reserved.
 This code is intended for use with the Cinder C++ library: http://libcinder.org
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_slides.pdf
// https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
// http://the-witness.net/news/2012/02/seamless-cube-map-filtering/
// https://seblagarde.wordpress.com/2012/06/10/amd-cubemapgen-for-physically-based-rendering/
// https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
// https://seblagarde.wordpress.com/2012/09/29/image-based-lighting-approaches-and-parallax-corrected-cubemap/
// http://graphics.stanford.edu/papers/envmap/
// https://graphics.cg.uni-saarland.de/fileadmin/cguds/courses/ss15/ris/slides/RIS18Green.pdf
// https://github.com/rlk/envtools
// https://github.com/derkreature/IBLBaker/
// https://github.com/dariomanesku/cmftStudio

#include "Environment.h"

#include <random>
#include <algorithm>

#include "cinder/gl/Context.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/scoped.h"
#include "cinder/gl/draw.h"
#include "cinder/Log.h"

using namespace ci;
using namespace std;

namespace renderkit {

EnvironmentRef Environment::create( int16_t width, int16_t height, const Format &format )
{
	return make_shared<Environment>( width, height, format );
}
EnvironmentRef Environment::create( const ci::gl::TextureCubeMapRef &skybox, const Format &format )
{
	return make_shared<Environment>( skybox, format );
}
EnvironmentRef Environment::create( const ci::gl::TextureCubeMapRef &skybox, const ci::gl::TextureCubeMapRef &radianceMap, const ci::gl::TextureCubeMapRef &irradianceMap, const Format &format )
{
	return make_shared<Environment>( skybox, radianceMap, irradianceMap, format );
}

Environment::Environment( int16_t width, int16_t height, const Format &format ) 
: Environment( 
	gl::TextureCubeMap::create( width, height, gl::TextureCubeMap::Format()
								.internalFormat( GL_RGBA32F )
								.mipmap().minFilter( GL_LINEAR_MIPMAP_LINEAR ).magFilter( GL_LINEAR )
								.wrap( GL_CLAMP_TO_EDGE ) ), 
	nullptr, nullptr, Format( format ).probe() ) 
{
}
Environment::Environment( const ci::gl::TextureCubeMapRef &envMap, const Format &format ) 
: Environment( envMap, nullptr, nullptr, format ) 
{
}
Environment::Environment( const ci::gl::TextureCubeMapRef &skybox, const ci::gl::TextureCubeMapRef &radianceMap, const ci::gl::TextureCubeMapRef &irradianceMap, const Format &format )
: mEnvironmentMap( skybox ), mRadianceMap( radianceMap ), mIrradianceMap( irradianceMap ),
mPosition( format.mPosition ), mSize( format.mSize ), mHasRadiance( format.mRadiance ), mHasIrradiance( format.mIrradiance )
{
	if( format.mIsProbe ) {
		auto fboFormat = gl::Fbo::Format().attachment( GL_COLOR_ATTACHMENT0, mEnvironmentMap );
		mFbo = gl::Fbo::create( mEnvironmentMap->getWidth(), mEnvironmentMap->getHeight(), fboFormat );

		//mTexture->setLabel( "EnvMap" );
		//mFbo->setLabel( "EnvironmentFbo" );
		//CI_LOG_W( "PROBE" );
	}

	if( ( ! mIrradianceMap && mHasIrradiance ) || ( ! mRadianceMap && mHasRadiance ) ) {
		if( mEnvironmentMap ) {
			mFilter = EnvironmentFilter::create( mEnvironmentMap );
		}
		else {
			mFilter = EnvironmentFilter::create();
		}
		
		mRadianceMap = mFilter->getPmRadianceEnvMap();
		mIrradianceMap = mRadianceMap;
	}
}

Environment::Format& Environment::Format::radiance( bool enabled )
{
	mRadiance = enabled;
	return *this;
}
Environment::Format& Environment::Format::irradiance( bool enabled )
{
	mIrradiance = enabled;
	return *this;
}
Environment::Format& Environment::Format::progressive( bool enabled )
{
	mIsProgressive = enabled;
	return *this;
}
Environment::Format& Environment::Format::size( const ci::vec3 &boxSize )
{
	mSize = boxSize;
	return *this;
}
Environment::Format& Environment::Format::position( const ci::vec3 &boxPosition )
{
	mPosition = boxPosition;
	return *this;
}
Environment::Format& Environment::Format::probe( bool enabled )
{
	mIsProbe = enabled;
	return *this;
}
		
ci::gl::TextureCubeMapRef Environment::getEnvironmentMap() const
{
	return mEnvironmentMap;
}
void Environment::setGlslUniforms( const ci::gl::GlslProg *glsl ) const
{
	glsl->uniform( "uEnvironmentMap", 0 );
	int numMipMaps = floor( std::log2( mRadianceMap->getWidth() ) ) - 1;
	glsl->uniform( "uEnvMapMaxMip", (float) ( numMipMaps - 1 ) );
}
void Environment::setGlslUniforms( const ci::gl::GlslProgRef &glsl ) const
{
	setGlslUniforms( glsl.get() );
}
ci::gl::TextureCubeMapRef Environment::getRadianceMap() const
{
	return mRadianceMap;
}
ci::gl::TextureCubeMapRef Environment::getIrradianceMap() const
{
	return mIrradianceMap;
}
void Environment::write()
{
}
void Environment::read()
{
}
void Environment::update()
{
	if( mHasIrradiance || mHasRadiance ) {
		mFilter->filter();
	}
}

ScopedEnvironmentWrite::ScopedEnvironmentWrite( const EnvironmentRef &envMap )
: mGlContext( gl::Context::getCurrent() ), mEnvironment( envMap )
{
	mGlContext->pushFramebuffer( mEnvironment->mFbo );
	//CI_LOG_W( mEnvironment->mFbo->getWidth() );
	gl::pushMatrices();
}
ScopedEnvironmentWrite::~ScopedEnvironmentWrite()
{
	gl::popMatrices();
	mGlContext->popFramebuffer();
	//mEnvironment->mFbo->resolveTextures();
	//mEnvironment->update();
}
void ScopedEnvironmentWrite::bindFace( uint8_t dir )
{
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + dir, mEnvironment->getEnvironmentMap()->getId(), 0 );
}

void ScopedEnvironmentWrite::setViewMatrix( uint8_t dir, const ci::vec3 & eye )
{
	static const vec3 target[6] = { vec3( 1, 0, 0 ), vec3( -1, 0, 0 ), vec3( 0, 1, 0 ), vec3( 0, -1, 0 ), vec3( 0, 0, 1 ), vec3( 0, 0, -1 ) };
	static const vec3 up[6] = { vec3( 0, 1, 0 ), vec3( 0, 1, 0 ), vec3( 0, 0, -1 ), vec3( 0, 0, 1 ), vec3( 0, 1, 0 ), vec3( 0, 1, 0 ) };
		
	gl::setViewMatrix( glm::lookAt( eye, eye + target[dir], up[dir] ) );
}

ScopedEnvironmentRead::ScopedEnvironmentRead( const EnvironmentRef &envMap, uint8_t textureUnit )
: mGlContext( gl::Context::getCurrent() )
, mEnvironment( envMap )
, mTextureUnit{ textureUnit }
{
	//mGlContext->pushBoolState( GL_TEXTURE_CUBE_MAP_SEAMLESS, true );
	mGlContext->pushTextureBinding( mEnvironment->getRadianceMap()->getTarget(), mEnvironment->getRadianceMap()->getId(), mTextureUnit );
	//if( auto glsl = mGlContext->getGlslProg() ) {
	//	mEnvironment->setGlslUniforms( glsl );
	//}
}
ScopedEnvironmentRead::~ScopedEnvironmentRead()
{
	//mGlContext->popBoolState( GL_TEXTURE_CUBE_MAP_SEAMLESS );
	mGlContext->popTextureBinding( mEnvironment->getRadianceMap()->getTarget(), mTextureUnit );
}

} // namespace renderkit
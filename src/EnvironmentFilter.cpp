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

#include "EnvironmentFilter.h"

#include "cinder/FileWatcher.h"

#include "cinder/Log.h"
#include "cinder/app/App.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/scoped.h"
#include "cinder/gl/draw.h"
#include <random>

using namespace ci;
using namespace std;

namespace renderkit {

EnvironmentFilterBase::EnvironmentFilterBase( const Format &format )
: mFaceSize( format.getFaceSize() ),
mNumMips( format.getNumMips() ),
mGammaInput( format.getGammaInput() ),
mGammaOutput( format.getGammaOutput() ),
mEdgeFixup( format.getEdgeFixup() )
{
}

EnvironmentFilterRef EnvironmentFilter::create( const Format &format )
{
	return make_shared<EnvironmentFilter>( format );
}

EnvironmentFilterRef EnvironmentFilter::create( const ci::gl::TextureCubeMapRef &envMap, const Format &format )
{
	return make_shared<EnvironmentFilter>( envMap, format );
}

EnvironmentFilter::EnvironmentFilter( const Format &format )
: EnvironmentFilterBase( format )
{
	initializeGlslProg( format );
}

EnvironmentFilter::EnvironmentFilter( const ci::gl::TextureCubeMapRef &envMap, const Format &format )
: EnvironmentFilterBase( format )
{
	mNumMips = format.getNumMips();
	mGammaInput = format.getGammaInput();
	mGammaOutput = format.getGammaOutput();
	mEnvMap = envMap;
	initializeGlslProg( format );
	initializeRenderTargets();
	filter();
}

ci::gl::TextureCubeMapRef EnvironmentFilter::getPmRadianceEnvMap() const
{
	if( mFilterFbo ) {
		return static_pointer_cast<gl::TextureCubeMap>( mFilterFbo->getTextureBase( GL_COLOR_ATTACHMENT0 ) );
	}
	else {
		return nullptr;
	}
}

void EnvironmentFilter::filter()
{
	// skip if no env map
	if( ! mEnvMap ) {
		CI_LOG_W( "EnvironmentFilter: No EnvMap Input texture" );
		return;
	}

	// create the radiance texture and framebuffer
	if( ! mFilterFbo ) {
		initializeRenderTargets();
	}

	gl::ScopedMatrices scopedMatrices;
	gl::ScopedGlslProg shaderScp( mGlslProg );
	gl::ScopedFramebuffer framebufferScp( mFilterFbo );
	gl::ScopedDepth scopedDepth( false );
	gl::ScopedBlend scopedBlend( false );
	
	auto filterTexture = mFilterFbo->getTextureBase( GL_COLOR_ATTACHMENT0 );
	mGlslProg->uniform( "uMaxMip", (float) mNumMips - 1 );
	//mGlslProg->uniform( "uGammaIn", vec3( mGammaInput ) );
	//mGlslProg->uniform( "uGammaOut", vec3( mGammaOutput ) );


	auto eyePos = vec3( 0 );
	CameraPersp cam;
	static const vec3 viewDirs[6] = { vec3( 1, 0, 0 ), vec3( -1, 0, 0 ), vec3( 0, 1, 0 ), vec3( 0, -1, 0 ), vec3( 0, 0, 1 ), vec3( 0, 0, -1 ) };
	for( int level = 0; level < mNumMips; level++ ){
		gl::ScopedTextureBind texScp( level > 0 ? filterTexture : mEnvMap, 0 );
		if( level > 0 ) {
			glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, level - 1 );
			glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, level - 1 );
		}
		vec2 size			= gl::Texture2d::calcMipLevelSize( level, mFilterFbo->getWidth(), mFilterFbo->getHeight() );
		auto proj = ci::CameraPersp( (int)size.x, (int)size.y, 90.0f, 0.1f, 100.0f ).getProjectionMatrix();
		mGlslProg->uniform( "uMip", (float) level );
		gl::ScopedViewport viewport( vec2( 0 ), vec2( size ) );
		for( GLenum dir = GL_TEXTURE_CUBE_MAP_POSITIVE_X; dir < GL_TEXTURE_CUBE_MAP_POSITIVE_X + 6; ++dir ) {
			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, dir, filterTexture->getId(), level );
			mat4 view = mat4();
			cam.lookAt( eyePos, eyePos + viewDirs[dir - GL_TEXTURE_CUBE_MAP_POSITIVE_X] );
			if( dir != GL_TEXTURE_CUBE_MAP_POSITIVE_Y && dir != GL_TEXTURE_CUBE_MAP_NEGATIVE_Y )
				view *= glm::rotate( (float)M_PI, vec3( 0, 0, 1 ) );
			view *= cam.getViewMatrix();
			gl::setProjectionMatrix( proj );
			gl::setViewMatrix( view );
			gl::drawCube( vec3( 0 ), vec3( 2 ) );
		}
	}

	gl::ScopedTextureBind scopedTex( filterTexture );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0 );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, mNumMips - 1 );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
}

void EnvironmentFilter::initializeGlslProg( const Format &format )
{
	try {
		mGlslProg = gl::GlslProg::create( app::loadAsset( "glsl/pbr/EnvFilter.vert" ), app::loadAsset( "glsl/pbr/EnvFilter.frag" ) );
		mGlslProg->uniform( "uCubeMapTex", 0 );
	}
	catch( const gl::GlslProgCompileExc &exc ) { CI_LOG_EXCEPTION( exc.what(), exc ); }
}

void EnvironmentFilter::initializeRenderTargets()
{
	auto textureResolution = min( (GLint) mFaceSize, mEnvMap->getWidth() );
	auto textureFormat = gl::TextureCubeMap::Format().internalFormat( mEnvMap->getInternalFormat() ).mipmap().minFilter( GL_LINEAR_MIPMAP_LINEAR ).magFilter( GL_LINEAR ).immutableStorage().wrap( GL_CLAMP_TO_EDGE );
	mNumMips = mNumMips == 0 ? (uint8_t)floor( std::log2( textureResolution ) ) : mNumMips;
	textureFormat.setMaxMipmapLevel( mNumMips - 1 );
	
	auto pmremMap = gl::TextureCubeMap::create( textureResolution, textureResolution, textureFormat );
	mFilterFbo = gl::Fbo::create( textureResolution, textureResolution, gl::Fbo::Format().attachment( GL_COLOR_ATTACHMENT0, pmremMap ) );
}

} // namespace renderkit

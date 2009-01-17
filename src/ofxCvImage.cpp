
#include "ofxCvImage.h"
#include "ofxCvGrayscaleImage.h"
#include "ofxCvColorImage.h"
#include "ofxCvFloatImage.h"



//--------------------------------------------------------------------------------
ofxCvImage::ofxCvImage() {
    width			= 0;
    height			= 0;
    bUseTexture		= true;
    bTextureDirty   = true;
	bAllocated		= false;
	pixels			= NULL;
}

//--------------------------------------------------------------------------------
ofxCvImage::~ofxCvImage() {
    clear();
}


//--------------------------------------------------------------------------------
void ofxCvImage::clear() {

    // ------------------------------ only delete if the
	// ------------------------------ image is really an image.
	// ------------------------------ ie, w > 0, h > 0

	if (bAllocated == true){
		if (width > 0 && height > 0){
			cvReleaseImage( &cvImage );
			cvReleaseImage( &cvImageTemp );
		}
		delete pixels;
        pixels = NULL;
		width = 0;
		height = 0;

		if( bUseTexture ) {
			tex.clear();
		}
		
		bAllocated = false;
	}
}


//--------------------------------------------------------------------------------
void ofxCvImage::setUseTexture( bool bUse ) {
	bUseTexture = bUse;
}


//--------------------------------------------------------------------------------
void ofxCvImage::swapTemp() {
	IplImage*  temp;
	temp = cvImage;
	cvImage	= cvImageTemp;
	cvImageTemp	= temp;
}





// Set Pixel Data

//--------------------------------------------------------------------------------
void ofxCvImage::operator -= ( float value ) {
	cvSubS( cvImage, cvScalar(value), cvImageTemp );
	swapTemp();
    bTextureDirty = true;
}

//--------------------------------------------------------------------------------
void ofxCvImage::operator += ( float value ) {
	cvAddS( cvImage, cvScalar(value), cvImageTemp );
	swapTemp();
    bTextureDirty = true;
}


//--------------------------------------------------------------------------------
void ofxCvImage::operator -= ( const ofxCvImage& mom ) {
	if( mom.width == width && mom.height == height &&
	    mom.getCvImage()->nChannels == cvImage->nChannels && 
        mom.getCvImage()->depth == cvImage->depth )
    {
		cvSub( cvImage, mom.getCvImage(), cvImageTemp );
		swapTemp();
        bTextureDirty = true;
	} else {
        cout << "error in -=, images need to match in size and type" << endl;
	}
}

//--------------------------------------------------------------------------------
void ofxCvImage::operator += ( const ofxCvImage& mom ) {
	if( mom.width == width && mom.height == height &&
	    mom.getCvImage()->nChannels == cvImage->nChannels && 
        mom.getCvImage()->depth == cvImage->depth )
    {
		cvAdd( cvImage, mom.getCvImage(), cvImageTemp );
		swapTemp();
        bTextureDirty = true;
	} else {
        cout << "error in +=, images need to match in size and type" << endl;
	}
}

//--------------------------------------------------------------------------------
void ofxCvImage::operator *= ( const ofxCvImage& mom ) {
    float scalef = 1.0f / 255.0f;
	if( mom.width == width && mom.height == height &&
	    mom.getCvImage()->nChannels == cvImage->nChannels && 
        mom.getCvImage()->depth == cvImage->depth )
    {
		cvMul( cvImage, mom.getCvImage(), cvImageTemp, scalef );
		swapTemp();
        bTextureDirty = true;
	} else {
        cout << "error in *=, images need to match in size and type" << endl;
	}
}

//--------------------------------------------------------------------------------
void ofxCvImage::operator &= ( const ofxCvImage& mom ) {
	if( mom.width == width && mom.height == height &&
	    mom.getCvImage()->nChannels == cvImage->nChannels && 
        mom.getCvImage()->depth == cvImage->depth )
    {
		cvAnd( cvImage, mom.getCvImage(), cvImageTemp );
		swapTemp();
        bTextureDirty = true;
	} else {
        cout << "error in &=, images need to match in size and type" << endl;
	}
}




// Image Filter Operations
//--------------------------------------------------------------------------------
void ofxCvImage::dilate() {
	cvDilate( cvImage, cvImageTemp, 0, 1 );
	swapTemp();
    bTextureDirty = true;
}

//--------------------------------------------------------------------------------
void ofxCvImage::erode() {
	cvErode( cvImage, cvImageTemp, 0, 1 );
	swapTemp();
    bTextureDirty = true;
}

//--------------------------------------------------------------------------------
void ofxCvImage::blur( int value ) {
    if( value % 2 == 0 ) {
        cout << "warning in blur: value not odd -> will add 1 to cover your back" << endl;
        value++;
    }
	cvSmooth( cvImage, cvImageTemp, CV_BLUR , value);
	swapTemp();
    bTextureDirty = true;
}

//--------------------------------------------------------------------------------
void ofxCvImage::blurGaussian( int value ) {
    if( value % 2 == 0 ) {
        cout << "warning in blurGaussian: value not odd -> will add 1 to cover your back" << endl;
        value++;
    }
	cvSmooth( cvImage, cvImageTemp, CV_GAUSSIAN ,value );
	swapTemp();
    bTextureDirty = true;
}

//--------------------------------------------------------------------------------
void ofxCvImage::invert(){
    cvNot(cvImage, cvImage);
    bTextureDirty = true;
}

//--------------------------------------------------------------------------------
void ofxCvImage::convertToRange(float scaleMin, float scaleMax ){
    float range = (scaleMax - scaleMin);
    float scale = 255/range;
    float offset = - (scaleMin * scale);  // ie, 0.5 - 1 = scale by (255*2), subtract 255, 128-255 = scale by 1/2, subtract 128
    cvConvertScale( cvImage, cvImageTemp, scale, offset );
    swapTemp();
    bTextureDirty = true;
}




// Image Transformation Operations

//--------------------------------------------------------------------------------
void ofxCvImage::mirror( bool bFlipVertically, bool bFlipHorizontally ) {
	int flipMode = 0;

	if( bFlipVertically && !bFlipHorizontally ) flipMode = 0;
	else if( !bFlipVertically && bFlipHorizontally ) flipMode = 1;
	else if( bFlipVertically && bFlipHorizontally ) flipMode = -1;
	else return;

	cvFlip( cvImage, cvImageTemp, flipMode );
	swapTemp();
    bTextureDirty = true;
}

//--------------------------------------------------------------------------------
void ofxCvImage::translate( float x, float y ) {
    transform( 0, 0,0, 1,1, x,y );
    bTextureDirty = true;
}

//--------------------------------------------------------------------------------
void ofxCvImage::rotate( float angle, float centerX, float centerY ) {
    transform( angle, centerX, centerY, 1,1, 0,0 );
    bTextureDirty = true;
}

//--------------------------------------------------------------------------------
void ofxCvImage::scale( float scaleX, float scaleY ) {
    transform( 0, 0,0, scaleX,scaleY, 0,0 );
    bTextureDirty = true;
}

//--------------------------------------------------------------------------------
void ofxCvImage::transform( float angle, float centerX, float centerY,
                            float scaleX, float scaleY,
                            float moveX, float moveY )
{
    float sina = sin(angle * DEG_TO_RAD);
    float cosa = cos(angle * DEG_TO_RAD);
    CvMat*  transmat = cvCreateMat( 2,3, CV_32F );
    cvmSet( transmat, 0,0, scaleX*cosa );
    cvmSet( transmat, 0,1, scaleY*sina );
    cvmSet( transmat, 0,2, -centerX*scaleX*cosa - centerY*scaleY*sina + moveX + centerX );
    cvmSet( transmat, 1,0, -1.0*scaleX*sina );
    cvmSet( transmat, 1,1, scaleY*cosa );
    cvmSet( transmat, 1,2, -centerY*scaleY*cosa + centerX*scaleX*sina + moveY + centerY);

    cvWarpAffine( cvImage, cvImageTemp, transmat );
	swapTemp();
    bTextureDirty = true;

    cvReleaseMat( &transmat );
}

//--------------------------------------------------------------------------------
void ofxCvImage::undistort( float radialDistX, float radialDistY,
                           float tangentDistX, float tangentDistY,
                           float focalX, float focalY,
                           float centerX, float centerY ){
    float camIntrinsics[] = { focalX, 0, centerX, 0, focalY, centerY, 0, 0, 1 };
    float distortionCoeffs[] = { radialDistX, radialDistY, tangentDistX, tangentDistY };
    cvUnDistortOnce( cvImage, cvImageTemp, camIntrinsics, distortionCoeffs, 1 );
	swapTemp();
    bTextureDirty = true;
}


//--------------------------------------------------------------------------------
void ofxCvImage::remap( const IplImage* mapX, const IplImage* mapY ) {
    cvRemap( cvImage, cvImageTemp, mapX, mapY );
	swapTemp();
    bTextureDirty = true;
}




/**
*    A  +-------------+  B
*      /               \
*     /                 \
*    /                   \
* D +-------------------- +  C
*/

//--------------------------------------------------------------------------------
void ofxCvImage::warpPerspective( const ofPoint& A, const ofPoint& B, 
                                  const ofPoint& C, const ofPoint& D ) 
{ 
    // compute matrix for perspectival warping (homography) 
    CvPoint2D32f cvsrc[4]; 
    CvPoint2D32f cvdst[4]; 
    CvMat* translate = cvCreateMat( 3,3, CV_32FC1 ); 
    cvSetZero( translate ); 

    cvdst[0].x = 0; 
    cvdst[0].y = 0; 
    cvdst[1].x = width; 
    cvdst[1].y = 0; 
    cvdst[2].x = width; 
    cvdst[2].y = height; 
    cvdst[3].x = 0; 
    cvdst[3].y = height; 

    cvsrc[0].x = A.x; 
    cvsrc[0].y = A.y; 
    cvsrc[1].x = B.x; 
    cvsrc[1].y = B.y; 
    cvsrc[2].x = C.x; 
    cvsrc[2].y = C.y; 
    cvsrc[3].x = D.x; 
    cvsrc[3].y = D.y; 

    cvWarpPerspectiveQMatrix( cvsrc, cvdst, translate );  // calculate homography 
    cvWarpPerspective( cvImage, cvImageTemp, translate ); 
    swapTemp();
    bTextureDirty = true;
    cvReleaseMat( &translate ); 
} 



//--------------------------------------------------------------------------------
void ofxCvImage::warpIntoMe( const ofxCvGrayscaleImage& mom,
                            ofPoint src[4], ofPoint dst[4] )
{
	// compute matrix for perspectival warping (homography)
	CvPoint2D32f cvsrc[4];
	CvPoint2D32f cvdst[4];
	CvMat* translate = cvCreateMat( 3, 3, CV_32FC1 );
	cvSetZero( translate );
	for (int i = 0; i < 4; i++ ) {
		cvsrc[i].x = src[i].x;
		cvsrc[i].y = src[i].y;
		cvdst[i].x = dst[i].x;
		cvdst[i].y = dst[i].y;
	}
	cvWarpPerspectiveQMatrix( cvsrc, cvdst, translate );  // calculate homography
	cvWarpPerspective( mom.getCvImage(), cvImage, translate);
    bTextureDirty = true;
	cvReleaseMat( &translate );
}




// Other Image Operations

//--------------------------------------------------------------------------------
int ofxCvImage::countNonZeroInRegion( int x, int y, int w, int h ) const {
	if (w == 0 || h == 0) return 0;
    int count = 0;
	cvSetImageROI( cvImage, cvRect(x,y,w,h) );
	count = cvCountNonZero( cvImage );
	cvResetImageROI( cvImage );
	return count;
}






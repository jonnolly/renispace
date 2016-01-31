/*
 * CBlockReader.h
 *
 *  Created on: 24 Jan 2016
 *      Author: Jack Buckingham
 */

#ifndef SRC_CBLOCKREADER_H_
#define SRC_CBLOCKREADER_H_

// ~~~ INCLUDES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include <string>
#include <opencv2/opencv.hpp>

/* ~~~ CLASS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * This class will encapsulate the algorithms required to analyse the spots on a single still image
 * of a block (stored as a member variable). It contains private methods to
 *  - detect spots,
 *  - verify that they are not too close to the edge of the photo to risk missing some spots,
 *  - verify that the arrangement of the spots is expected.
 * These are interfaced by a single public function CountSpots(...).
 *
 * Constructors and Destructors:
 * 	Default Constructor - The default constructor will create an instance of the class with an
 * 		empty cv::Mat as the image. This is designed to be used with the TakePhoto public method so
 * 		that the class instance can be initialised from a photo straight from the camera.
 *
 * 	CBlockReader(const string imagePath) - Accepts a file path to an image with which to
 * 		initialise the class instance.
 *
 * 	Destructor - The class contains an autogenerated virtual destructor stub.
 *
 * Public Member Functions:
 * 	CountSpots(...) - This will return an integer specifying the number of spots counted. A value
 * 		of -1 is returned if the spots are suspicious. That is, if either the spots found were to
 * 		close to the edge of the photo to be sure that there were no spots missed, or if the
 * 		arrangement of the spots found does not match the arrangement expected for that number of
 * 		spots.
 *
 * 	ComputeBlockLocation(...) - Outputs the approximate position of the block relative to the
 * 	robot. This allows the caller to reposition the robot for a better photo.
 *
 * 	TakePhoto(...) - This will use the Pi's camera to take a photo. It will save the photo in the
 *			specified location, update m_Image to use this photo and reset all member variables
 *			calculated using the old image.
 *
 */
class CBlockReader
{
public:
	// === Constructors and Destructors =============================================================
	CBlockReader();
	CBlockReader(const std::string imagePath);
	virtual ~CBlockReader();

	// === Public Functions =========================================================================
	int CountSpots();
	bool ComputeBlockLocation(double& blockRelPosn_x, double& blockRelPosn_y);
	bool TakePhoto(const std::string saveLocation);

	// === Exceptions ===============================================================================
	struct InputImagePath_BadFilePath
	{
		std::string mm_ImagePath;
		InputImagePath_BadFilePath(std::string imagePath)
				: mm_ImagePath { imagePath }
		{
		}
	};

private:
	// === Private Functions ========================================================================
	void DetectSpots();
	bool VerifySpotArrangement();
	bool VerifySpotNbhdVisible();

	// === Member Variables =========================================================================
	cv::Mat m_Image;
	std::vector<cv::KeyPoint> m_Spots;

};

#endif /* SRC_CBLOCKREADER_H_ */

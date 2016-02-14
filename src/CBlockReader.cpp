/*
 * CBlockReader.cpp
 *
 *  Created on: 24 Jan 2016
 *      Author: Jack Buckingham
 */

// ~~~ INCLUDES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "CBlockReader.h"
#include <cmath>

// ~~~ NAMESPACES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
using namespace std;
using namespace cv;

// -/-/-/-/-/-/-/ CONSTRUCTORS AND DESTRUCTORS /-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/
/* ~~~ FUNCTION (default constructor) ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Initialises m_Image and m_Spots as empty by implicit calls to their default constructors.
 */
CBlockReader::CBlockReader()
{
}

/* ~~~ FUNCTION (constructor) ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Initialises m_Image from a specified file.
 *
 * The image is resized immediately after loading.
 * The constructor throws an exception if the image could not be loaded.
 *
 * INPUTS:
 * 	imagePath - A string containing the path to an image to use to initialise the class instance.
 */
CBlockReader::CBlockReader(string imagePath)
{
	Mat image_fullsize = imread(imagePath, IMREAD_GRAYSCALE);
	if (image_fullsize.empty())
	{
		throw InputImagePath_BadFilePath { imagePath };
	}
	else
	{
		resize(image_fullsize, m_Image, Size(), 0.2, 0.2, INTER_AREA);
	}
}


// -/-/-/-/-/-/-/ INITIALISER FUNCTIONS /-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-
/* ~~~ FUNCTION (public) ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *	This function uses the Pi's camera to take a photo. It will save the photo, update m_Image to use
 *	this photo and reset all member variables calculated using any previous image.
 *
 *	INPUTS:
 *	saveLocation - This should contain the path at which to save the photo.
 */
bool CBlockReader::TakePhoto(const std::string saveLocation)
{
	// TODO: Find out how to take a photo with the PI camera and then implement CBlockReader::TakePhoto
}

// -/-/-/-/-/-/-/ SPOT COUNTING FUNCTIONS /-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-
/* ~~~ FUNCTION (public) ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * This function uses the opencv SimpleBlobDetector to count and return the number of spots in the
 * picture. It calls VerifySpotNbhdVisible() and VerifySpotArrangement() to check that the number
 * counted can be trusted. A value of -1 is returned instead if not.
 */
int CBlockReader::CountSpots()
{
	DetectSpots();

	if (VerifySpotNbhdVisible() & VerifySpotArrangement())
		return m_Spots.size();
	else
		return -1;
}

/* ~~~ FUNCTION (private) ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * This function uses the opencv SimpleBlobDetector to identify the spots in the picture. The spots
 * are filtered based on their area, circularity, convexity and inertia.
 */
void CBlockReader::DetectSpots()
{
	////////////////////////////////////////////////////////////
	// Setup SimpleBlobDetector Parameters.

	SimpleBlobDetector::Params params;

	// Change thresholds
	params.minThreshold = 10;
	params.maxThreshold = 200;

	// Filter by Area.
	params.filterByArea = true;
	params.minArea = 500;

	// Filter by Circularity
	params.filterByCircularity = true;
	params.minCircularity = 0.7;

	// Filter by Convexity
	params.filterByConvexity = true;
	params.minConvexity = 0.87;

	// Filter by Inertia
	params.filterByInertia = true;
	params.minInertiaRatio = 0.8;

	////////////////////////////////////////////////////////////
	// Detect Spots

	SimpleBlobDetector detector(params);
	detector.detect(m_Image, m_Spots);
}

/* ~~~ FUNCTION (private) ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * This function verifies that the spots are in the expected arrangement for that number of spots.
 * That is, it verifies that they are related to the expected arrangement by a non-reflecting
 * isometry. That is, by a rotation and a translation.
 *
 * It does this by
 *  - computing the mean of the spot locations and subtracting it from the spots
 *  - sorting the spots by angle and radius (anticlockwise angle from pos x-axis)
 *  - computing the distances between all pairs of spots
 *  - comparing these distances with the expected distances, for each 'circular permutation' of the
 *    spots (i.e. a permutation which is a power of (1,...,n) ).
 *
 * This is an O(n^2) algorithm where n is the number of spots.
 *
 * By allowing a circular permutation of the spots, we are allowing for mismatching of measured and
 * expected spots due to rotation about the mean of the spots.
 * Ideally we would just look at the circular permutations which matched up groups of measured spots
 * with the same angle with groups of expected spots with the same angle (i.e. not split up groups).
 * However, it is easier to simply include all permutations round the circle, and rely on the ones
 * with split groups to fail the test (which they should!).
 *
 */
bool CBlockReader::VerifySpotArrangement() const
{
	// Compute mean spot location
	double meanLoc_x {0};
	double meanLoc_y {0};
	for (unsigned int i = 0; i < m_Spots.size(); ++i)
	{
		meanLoc_x += m_Spots[i].pt.x;
		meanLoc_y += m_Spots[i].pt.y;
	}

	if (m_Spots.size() == 0)
		return false;
	else
	{
		meanLoc_x /= m_Spots.size();
		meanLoc_y /= m_Spots.size();
	}

	// Centre points by subtracting mean
	vector<Point2f> centredSpots {};
	centredSpots.reserve(m_Spots.size());
	for (unsigned int i = 0; i < m_Spots.size(); ++i)
	{
		// TODO: Check that I can update the coordinates of aSpot like this...
		Point2f aSpot = m_Spots[i].pt;
		aSpot.x -= meanLoc_x;
		aSpot.y -= meanLoc_y;
		centredSpots.push_back(aSpot);
	}

	// Sort centredSpots
	double angTol = 0.05; // radians
	double radTol = 5; // pixels
	sort(centredSpots.begin(), centredSpots.end(), CompareByAngleThenRadius(angTol, radTol));

	// Compute distances between spots
	vector<vector<double> > spotDistances(centredSpots.size(), vector<double>(centredSpots.size(), -1));
	for (unsigned int i = 0; i < spotDistances.size(); ++i)
	{
		for (unsigned int j = 0; j < spotDistances.size(); ++j)
		{
			double xDiff = centredSpots[i].x - centredSpots[j].x;
			double yDiff = centredSpots[i].y - centredSpots[j].y;

			spotDistances[i][j] = sqrt(xDiff * xDiff + yDiff * yDiff);
		}
	}

	// Load expected spot distances
	// TODO: Load expected spot distances
	vector<vector<double> > expectedSpotDistances(centredSpots.size(), vector<double>(centredSpots.size(), -1));

	// Compare with expected spot distances
	double spotDistTol = 1; // temporary value -- use proportion of spot size? // TODO: Set spotDistTol as a member variable?
	for (unsigned int n = 0; n < m_Spots.size(); ++n) // Permute round circle by n points
	{
		bool match = true;
		for (unsigned int i = 0; i < m_Spots.size(); ++i)
		{
			for (unsigned int j = 0; j < m_Spots.size(); ++j)
			{
				// TODO: Check the modular arithmetic is implemented to do what I want here!!
				match &= abs(expectedSpotDistances[(i+n) % centredSpots.size()][(j+n) % centredSpots.size()] - spotDistances[i][j]) < spotDistTol;
			}
		}

		if (match)
			return true;
	}

	return false;
}

/* ~~~ FUNCTION (private) ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * This function verifies that the spots are not too close to the edge of the image such that we
 * are at risk of cropping out extra spots.
 */
bool CBlockReader::VerifySpotNbhdVisible() const
{
	double minSpotPadding = 1; // A proportion of the diameter of the spot

	bool result = true;
	for (unsigned int i = 0; i < m_Spots.size(); ++i)
	{
		double distFromLeftEdge = m_Spots[i].pt.x;
		double distFromRightEdge = m_Image.cols - m_Spots[i].pt.x;
		double distFromTopEdge = m_Spots[i].pt.y;
		double distFromBottomEdge = m_Image.rows - m_Spots[i].pt.y;

		if (distFromLeftEdge < minSpotPadding * m_Spots[i].size)
		{
			result = false;
		}
		else if (distFromRightEdge < minSpotPadding * m_Spots[i].size)
		{
			result = false;
		}
		else if (distFromTopEdge < minSpotPadding * m_Spots[i].size)
		{
			result = false;
		}
		else if (distFromBottomEdge < minSpotPadding * m_Spots[i].size)
		{
			result = false;
		}
	}

	return result;
}


// -/-/-/-/-/-/-/ BLOCK LOCATION FUNCTIONS /-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/
/* ~~~ FUNCTION (public) ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * This function analyses the image to determine the position of the block relative to the robot.
 * This is useful for providing feedback to the caller when the picture supplied to the CBlockReader
 * is rejected when counting the spots.
 *
 * OUTPUTS:
 * blockRelPosn_x - The x-coordinate of the approximate centre of the block in the image frame.
 * blockRelPosn_y - The y-coordinate of the approximate centre of the block in the image frame.
 * return value - This returns true if the above quantities could be computed from the picture, and
 * 	false otherwise. For example, false is returned if the block could not be found within the
 * 	image.
 */
bool CBlockReader::ComputeBlockLocation(double& blockRelPosn_x, double& blockRelPosn_y) const
{
	blockRelPosn_x = 0;
	blockRelPosn_y = 0;

	for (unsigned int i = 0; i < m_Spots.size(); ++i)
	{
		blockRelPosn_x += m_Spots[i].pt.x;
		blockRelPosn_y += m_Spots[i].pt.y;
	}

	if (m_Spots.size() == 0)
		return false;
	else
	{
		blockRelPosn_x /= m_Spots.size();
		blockRelPosn_y /= m_Spots.size();
		return true;
	}

}

// -/-/-/-/-/-/-/ HELPER FUNCTIONS /-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/
/* ~~~ Struct (emulating private function) ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * This function will compare two vectors of cv::Point2f. If one point is further round the circle
 * than the other, then it will be deemed larger. If two points are the same angle round the circle,
 * up to tolerence, then the radius will be used to sort them.
 * Angles are measured anticlockwise from the positive x-axis.
 * The angle given to (0,0) (and points within angTol of this) is zero. Hence any such point will be
 * smaller than any non (0,0) point.
 *
 * INPUTS [constructor]:
 * angTol - If the angle computed (in RADIANS) for the two points is less than angTol then the
 * 	points will be deemed to have the same angle.
 * radTol - If a point has norm less than radTol then it will be deemed to be zero.
 *
 * OPERATOR():
 * 	If point1 < point2 by the algorithm defined above, then the function returns true. Else it
 * 	returns false.
 *
 */
CBlockReader::CompareByAngleThenRadius::CompareByAngleThenRadius(double angTol, double radTol)
		: m_AngTol { angTol }, m_RadTol { radTol }
{
	if (m_AngTol < 0) {/* TODO: Throw exception*/}
	if (m_RadTol < 0) {/* TODO: Throw exception*/}
}

bool CBlockReader::CompareByAngleThenRadius::operator ()(const Point2f point1, const Point2f point2) const
{
	// Compute radii squared
	double point1_rad2 = point1.x * point1.x + point1.y * point1.y;
	double point2_rad2 = point2.x * point2.x + point2.y * point2.y;

	// Check neither point is the centre point
	if (point2_rad2 < m_RadTol * m_RadTol)
		return false; // point2 <= point1

	if (point1_rad2 < m_RadTol * m_RadTol)
		return true; // point1 == (0,0) < point2 (would have returned false in previous statement if point2 == (0,0) as well)

	// Compute angles
	double point1_ang = atan2(point1.y, point1.x);
	double point2_ang = atan2(point2.y, point2.x);
	// TODO: Declare and check errorno - see page 918 of C++ book

	// Compare angles
	double pi = 3.14; // TODO: Find out how to get PI properly!
	if (abs(point1_ang - point2_ang) < m_AngTol
			|| abs(point1_ang - point2_ang > pi - m_AngTol))
	{
		// If the points have the same angle
		if (point1_rad2 < point2_rad2)
			return true;
		else
			return false;
	}
	else if (point1_ang < point2_ang)
		return true;
	else // If point 2 has the smaller angle
		return false;

}

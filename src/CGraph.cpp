/*
 * CGraph.cpp
 *
 *  Created on: 13 Nov 2015
 *      Author: Jack Buckingham
 */

// ~~~ INCLUDES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "CGraph.h"
#include<limits>

// ~~~ NAMESPACES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
using namespace std;

// -/-/-/-/-/-/-/ CONSTRUCTORS AND DESTRUCTORS /-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/
// TODO: Update comments
/* ~~~ FUNCTION (constructor) ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * This function is a constructor for the CGraph class. It takes inputs for the distance matrix.
 * distanceMatrix[i][j] should be: - the distance from vertex i to vertex j.
 *                                 - -1 if vertices i and j are not adjacent (so infinite distance)
 *
 * It checks that
 *   - The vector<vector<double> > is a matrix and is square.
 *   - The matrix is not too large. The order of the graph must fit into an unsigned integer.
 *   - The entries of the matrix are >= 0 or -1 (-1 is used to represent infinity).
 *
 * It also initialises the adjacency matrix using the distance matrix.
 *
 */
CGraph::CGraph(const vector<vector<double> > &distanceMatrix, const vector<unsigned int> vertexLabels)
		: m_Order { (unsigned int)distanceMatrix.size() }, m_InternalToExternal { vertexLabels }
{
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//// Check Inputs ////

	// Distance matrix
	CGraph_DistMatCheckResult distanceMatrixType = CheckInput_DistMat(distanceMatrix);

	switch (distanceMatrixType) {
	case CGraph_DistMatCheckResult::tooLarge:
		throw InputDistMat_MatrixTooLarge { distanceMatrix.size(), std::numeric_limits<unsigned int>::max() };
		break;
	case CGraph_DistMatCheckResult::badShape:
		//TODO Change this to a badShape exception
		throw InputDistMat_NotSquareMatrix { distanceMatrix };
		break;
	case CGraph_DistMatCheckResult::invalidElements:
		throw InputDistMat_InvalidElements { distanceMatrix };
		break;
	case CGraph_DistMatCheckResult::square:
		m_DistanceMatrix = distanceMatrix;
		break;
	case CGraph_DistMatCheckResult::lowerTriangular:
		// Define and fill out distance matrix as a square matrix
		m_DistanceMatrix = vector<vector<double> >(m_Order, vector<double>(m_Order, 0));
		for (unsigned int i = 0; i < m_Order; ++i)
		{
			for (unsigned int j = 0; j < m_Order; ++j)
			{
				if (i < j)
					m_DistanceMatrix[i][j] = distanceMatrix[j][i];
				else
					m_DistanceMatrix[i][j] = distanceMatrix[i][j];
			}
		}
		break;
	case CGraph_DistMatCheckResult::upperTriangular:
		m_DistanceMatrix = vector<vector<double> >(m_Order, vector<double>(m_Order, 0));
		for (unsigned int i = 0; i < m_Order; ++i)
		{
			for (unsigned int j = 0; j < m_Order; ++j)
			{
				if (i > j)
					m_DistanceMatrix[i][j] = distanceMatrix[j][i];
				else
					m_DistanceMatrix[i][j] = distanceMatrix[i][j];
			}
		}
		break;
	default:
		throw InternalException("Code broken internally. CGraph::CheckInput_DistMat returned undefined.");
	}


	// Check and store external-to-internal vertex labels
	if (m_Order != vertexLabels.size())
		throw InputVertexLabels_BadSize { vertexLabels.size() };
	for (unsigned int i = 0; i < vertexLabels.size(); ++i)
		m_ExternalToInternal[vertexLabels[i]] = i;
	if (m_Order != m_ExternalToInternal.size())
		throw InputVertexLabels_RepeatedLabel { vertexLabels };

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//// Create adjacency matrix ////

	// Allocate space in matrix
	m_AdjacencyMatrix = vector< vector<bool> >(m_Order, vector<bool>(m_Order, false));

	for (unsigned int i = 0; i < m_Order; ++i)
	{
		// Create the matrix one row at a time.
		for (unsigned int j = 0; j < m_Order; ++j)
		{
			if (distanceMatrix[i][j] >= 0 && i != j)
			{
				m_AdjacencyMatrix[i][j] =true;
			}
		}
	}

	//TODO: Do I also need to initialise the Dijkstra member variables or will their default constructors be called?
}

/* ~~~ FUNCTION (destructor) ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * This function is the destructor for the CGraph class.
 */
CGraph::~CGraph()
{
// TODO Auto-generated destructor stub
}

// -/-/-/-/-/-/-/ INTERNAL NUMBERING FUNCTIONS /-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/
/* ~~~ FUNCTION (private) ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * This function converts the internal numbering of the vertices to the external numbering.
 *
 * -- Overload 1 --
 * INPUT:
 * vertex = An unsigned int which is the internal label of some vertex of the graph
 *
 * OUTPUT:
 * An unsigned int which is the external label of vertex.
 *
 * -- Overload 2 --
 * INPUT/OUTPUT:
 * vertices = A vector of unsigned ints which are the internal labels of some vertices of the graph.
 *            This is passed as a reference and updated by the function to contain the corresponding
 *            external labels.
 */
unsigned int CGraph::InternalToExternal(const unsigned int vertex) const
{
	return m_InternalToExternal.at(vertex);
}

void CGraph::InternalToExternal(vector<unsigned int>& vertices) const
{
	for (long unsigned int i = 0; i < vertices.size(); ++i)
		vertices[i] = m_InternalToExternal.at(vertices[i]);
}

/* ~~~ FUNCTION (private) ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * This function converts the external numbering of the vertices to the internal numbering.
 *
 * -- Overload 1 --
 * INPUT:
 * vertex = An unsigned int which is the external label of some vertex of the graph
 *
 * OUTPUT:
 * An unsigned int which is the internal label of vertex.
 *
 * -- Overload 2 --
 * INPUT/OUTPUT:
 * vertices = A vector of unsigned ints which are the external labels of some vertices of the graph.
 *            This is passed as a reference and updated by the function to contain the corresponding
 *            internal labels.
 */
unsigned int CGraph::ExternalToInternal(const unsigned int vertex) const
{
	return m_ExternalToInternal.at(vertex);
}

void CGraph::ExternalToInternal(vector<unsigned int>& vertices) const
{
	for (long unsigned int i = 0; i < vertices.size(); ++i)
		vertices[i] = m_ExternalToInternal.at(vertices[i]);
}

// -/-/-/-/-/-/-/ DIJKSTRA FUNCTIONS /-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/
/* ~~~ FUNCTION (public) ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * This is a public wrapper for InternalShortestDistance.
 * It returns the shortest distance between two specified vertices, and an example shortest route
 * between them. This is done by searching for a previous appropriate set of Dijkstra outputs, and
 * calling Dijkstra if none exists.
 * Inputs and outputs use the external vertex numbering.
 *
 * INPUTS:
 * startVertex       = An unsigned integer representing the vertex at the start of the desired
 *                     route.
 * endVertex         = An unsigned integer representing the vertex at the end of the desired route.
 * preferStartVertex = A bool to determine whether to prefer running Dijkstra from the startVertex
 *                     (as opposed to from the end vertex).
 *                     The this argument allows the caller to minimise the calls to
 *                     InternalDijkstra required to return the desired shortest routes through the
 *                     graph.
 *                     If this argument is omitted then the
 *                     default of false shall be used (since this requires marginally less effort on
 *                     the part of the computer).
 *
 * INPUT OUTPUTS:
 * shortestDistance = The shortest distance between startVertex and endVertex.
 * outputRoute      = A vector containing an example shortest route from startVertex to endVertex.
 *                    outputRoute.front() == startVertex
 *                    outputRoute[i] == the ith element of the route
 *                    outputRoute.back() == endVertex
 *
 */
void CGraph::ShortestDistance(const unsigned int& startVertex, const unsigned int& endVertex, const bool& preferStartVertex, double& shortestDistance, vector<unsigned int>& outputRoute)
{
	// Convert to internal vertex numbering
	unsigned int iStartVertex = ExternalToInternal(startVertex);
	unsigned int iEndVertex = ExternalToInternal(endVertex);

	// Do the work
	InternalShortestDistance(iStartVertex, iEndVertex, preferStartVertex, shortestDistance, outputRoute);

	// Convert to external vertex numbering
	InternalToExternal(outputRoute);
}

void CGraph::ShortestDistance(const unsigned int& startVertex, const unsigned int& endVertex, double& shortestDistance, vector<unsigned int>& outputRoute)
{
	ShortestDistance(startVertex, endVertex, false, shortestDistance, outputRoute);
}

/* ~~~ FUNCTION (private) ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * This function is a wrapper for Dijkstra's algorithm.
 * It returns the shortest distance between two specified vertices, and an example shortest route
 * between them. This is done by searching for a previous appropriate set of Dijkstra outputs, and
 * calling Dijkstra if none exists.
 * Inputs and outputs use the internal vertex numbering.
 *
 * INPUTS:
 * startVertex       = An unsigned integer representing the vertex at the start of the desired
 *                     route.
 * endVertex         = An unsigned integer representing the vertex at the end of the desired route.
 * preferStartVertex = A bool to determine whether to prefer running Dijkstra from the startVertex
 *                     (as opposed to from the end vertex).
 *                     The this argument allows the caller to minimise the calls to
 *                     InternalDijkstra required to return the desired shortest routes through the
 *                     graph.
 *
 * INPUT OUTPUTS:
 * shortestDistance = The shortest distance between startVertex and endVertex.
 * outputRoute      = A vector containing an example shortest route from startVertex to endVertex.
 *                    outputRoute.front() == startVertex
 *                    outputRoute[i] == the ith element of the route
 *                    outputRoute.back() == endVertex
 *
 */
void CGraph::InternalShortestDistance(const unsigned int& startVertex, const unsigned int& endVertex, const bool& preferStartVertex, double& shortestDistance, vector<unsigned int>& outputRoute)
{
	/*  -- Decide on whether to use Dijkstra from the startVertex or the endVertex -- //
	 *   - Default is to use endVertex if Dijkstra as already been called for this, and startVertex
	 *     otherwise.
	 * Call internalDijkstra if necessary (from startVertex)
	 */
	bool fromStartVertex;
	if (m_DijkstraStartVertices.count(endVertex) > 0)
		fromStartVertex = false;
	else if (m_DijkstraStartVertices.count(startVertex) > 0)
		fromStartVertex = true;
	else if (preferStartVertex)
	{
		fromStartVertex = true;
		InternalDijkstra(startVertex);
	}
	else // !preferStartVertex
	{
		fromStartVertex = false;
		InternalDijkstra(endVertex);
	}

	// -- Unwind the shortest path and set shortestDistance -- //
	if (fromStartVertex)
	{
		// index is the index in m_DijkstraOutputRoutes and m_DijkstraShortestDistances
		// corresponding to startVertex
		unsigned int index = m_DijkstraStartVertices[startVertex];

		// Set first element in reverseOutputRoute as endVertex.
		// Then repeatedly push_back the parent vertex of the latest vertex in reverseOutputRoute until we reach startVertex
		vector<unsigned int> reverseOutputRoute = {endVertex};
		while (reverseOutputRoute.back() != startVertex)
			reverseOutputRoute.push_back(m_DijkstraOutputRoutes[index][reverseOutputRoute.back()]);

		// Set outputRoute as reverse of outputRoute
		outputRoute = vector<unsigned int> (reverseOutputRoute.rbegin(), reverseOutputRoute.rend());
//		outputRoute = vector<unsigned int> (reverseOutputRoute.size());
//		for (unsigned int i = 0; i < reverseOutputRoute.size(); ++i)
//			outputRoute[i] = reverseOutputRoute[reverseOutputRoute.size()-1 - i];

		// Set shortestDistance
		shortestDistance = m_DijkstraShortestDistances[index][endVertex];
	}
	else
	{
		// index is the index in m_DijkstraOutputRoutes and m_DijkstraShortestDistances
		// corresponding to endVertex
		unsigned int index = m_DijkstraStartVertices[endVertex];

		// Set first element in outputRoute as startVertex
		// Then repeatedly push_back the parent vertex of the latest vertex in outputRoute until we reach endVertex
		outputRoute = vector<unsigned int> {startVertex};
		while (outputRoute.back() != endVertex)
			outputRoute.push_back(m_DijkstraOutputRoutes[index][outputRoute.back()]);

		// Set shortestDistance
		shortestDistance = m_DijkstraShortestDistances[index][startVertex];
	}

}

/* ~~~ FUNCTION (private) ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * This function is an implementation of Dijkstra's algorithm -
 *                                                https://en.wikipedia.org/wiki/Dijkstra's_algorithm
 * It is called by the public function CGraph::Dijkstra and computes the tree of minimal routes in
 * the graph, from a specified start vertex to every other vertex in the graph. The output is stored
 * in the member variables of CGraph beginning with 'm_Dijkstra'.
 *
 * INPUTS:
 * 	startVertex - An integer to indicate the vertex of the graph from which to run the algorithm.
 * 	              Vertices of the graph will be represented by integers 0,...,n-1 where n is the
 * 	              order of the graph.
 *
 * OUTPUT:
 * 	The function returns an int which is m_DijkstraStartVertices[startVertex]. That is, it is the
 * 	index in m_DijkstraShortestDistances and m_DijkstraOutputRoutes corresponding to startVertex.
 *
 * MEMBER VARIABLES SET:
 * 	m_DijkstraShortestDistances - A vector of integers will be created which contains the shortest
 * 	              distances from the startVertex to each vertex of the graph. A value of -1 will
 * 	              be used to indicate that the vertex is not connected to startVertex.
 * 	              This vector is then appended to m_DijkstraShortestDistances.
 * 	m_DijkstraOutputRoutes - A vector of integers will be created to define examples of paths from
 * 	              the startVertex to each vertex of the graph which have the shortest total
 * 	              distance. The union of the example paths form a tree which is defined here by
 * 	              setting outputRoutes[i] to be the 'parent' of vertex i in the tree. By default,
 * 	              outputRoutes[startVertex] = startVertex. If a vertex i is not part of the same
 * 	              connected component as startVertex then outputRoutes[i] is -1.
 * 	              This vector is then appended to m_DijkstraOutputRoutes.
 * 	m_DijkstraStartVertices - A key-value pair is added in the map<int,int>
 * 	              m_DijkstraStartVertices to save the start vertex used to generate the Dijkstra
 * 	              results. The key is the startVertex and the value is the corresponding index in
 * 	              m_DijkstraShortestDistances and m_DijkstraOutputRoutes.
 *
 */
unsigned int CGraph::InternalDijkstra(const unsigned int& startVertex)
{
	// -- Initial Admin -- //
	// Check startVertex is a valid vertex
	if (startVertex < 0 || startVertex >= m_Order)
	{
		throw Dijkstra_InvalidStartVertex { startVertex };
	}

	// Check whether we have already computed Dijkstra for this startVertex
	auto mapIterator = m_DijkstraStartVertices.find(startVertex);
	if (mapIterator != m_DijkstraStartVertices.end())
		return mapIterator->second; // Todo: Check this syntax does what I want!!

	// Create and initialise vectors shortestDistances and outputRoutes
	vector<double> shortestDistances(m_Order, -1);
	vector<unsigned int> outputRoutes(m_Order, -1);
	for (unsigned int i = 0; i < m_Order; ++i)
	{
		if (i == startVertex || m_AdjacencyMatrix[startVertex][i])
		{
			outputRoutes[i] = startVertex;
			shortestDistances[i] = m_DistanceMatrix[startVertex][i];
		}
	}
	outputRoutes[startVertex] = startVertex;

	// Initialise knownDistances
	vector<bool> knownDistances(m_Order, false);
	knownDistances[startVertex] = true;

	// -- Main Algorithm Body -- //
	bool moreVertices = true; // If graph is empty then startVertex will not be a valid vertex and an exception will already have been thrown
	while (moreVertices)
	{
		// Find the vertex with the shortest unconfirmed shortestDistance to startVertex
		unsigned int nextClosest = -1;
		double nextShortestDistance = -1;
		for (unsigned int i = 0; i < m_Order; ++i)
		{
			if (!knownDistances[i] && shortestDistances[i] != -1
					&& (nextShortestDistance == -1 || shortestDistances[i] < nextShortestDistance))
			// For each vertex i for which we have not already confirmed the shortest distance to startVertex,
			// if we have a current estimate for its shortest distance to startVertex and if it is less than that of nextClosest,
			// replace nextClosest by i.
			{
				nextClosest = i;
				nextShortestDistance = shortestDistances[i];
			}
		}

		// Update shortest distances and shortest paths
		for (unsigned int i = 0; i < m_Order; ++i)
		{
			if (!knownDistances[i] && m_AdjacencyMatrix[nextClosest][i]
					&& (shortestDistances[i] == -1
							|| shortestDistances[i] > shortestDistances[nextClosest] + m_DistanceMatrix[nextClosest][i]))
			// For each neighbour i of nextClosest whose shortest distance to startVertex we do not yet know,
			// if it is faster to go via nextClosest (or if we have no current fastest path),
			// then update shortestDistances and outputRoutes.
			{
				shortestDistances[i] = shortestDistances[nextClosest] + m_DistanceMatrix[nextClosest][i];
				outputRoutes[i] = nextClosest;
			}
		}

		// Update knownDistances
		knownDistances[nextClosest] = true;

		// Check whether there are still more
		// (if remaining vertices are all infinity then don't bother checking any more)
		moreVertices = false;
		for (unsigned int i = 0; i < m_Order; ++i)
		{
			if (!knownDistances[i] && shortestDistances[i] != -1)
				moreVertices = true;
		}
	}

	// -- Save Results in Member Variables -- //
	m_DijkstraOutputRoutes.push_back(outputRoutes);
	m_DijkstraShortestDistances.push_back(shortestDistances);
	m_DijkstraStartVertices[startVertex] = m_DijkstraOutputRoutes.size() - 1;

	return m_DijkstraStartVertices[startVertex];
}

// -/-/-/-/-/-/-/ HELPER FUNCTIONS /-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/
CGraph_DistMatCheckResult CGraph::CheckInput_DistMat(const vector<vector<double> >& distanceMatrix) const
{
	// Check distanceMatrix is not too large for m_Order to fit in an unsigned int type
	// TODO: Can I make this a constexpr?
	const long unsigned max_unsignedInt = numeric_limits<unsigned int>::max();
	if (distanceMatrix.size() > max_unsignedInt)
		return CGraph_DistMatCheckResult::tooLarge;

	// Get dimensions and check the input is a square or triangular matrix
	// Determine shape if possible, and return bad shape if no shape is matched
	CGraph_DistMatCheckResult matrixShape { CGraph_DistMatCheckResult::undefined };
	if (distanceMatrix[0].size() == m_Order && distanceMatrix[1].size() == m_Order)
		matrixShape = CGraph_DistMatCheckResult::square;
	else if (distanceMatrix[0].size() == 1 && distanceMatrix[1].size() == 2 )
		matrixShape = CGraph_DistMatCheckResult::lowerTriangular;
	else if (distanceMatrix[0].size() == m_Order && distanceMatrix[1].size() == m_Order - 1)
		matrixShape = CGraph_DistMatCheckResult::upperTriangular;
	else
		return CGraph_DistMatCheckResult::badShape;

	// Check rest of matrix agrees with this shape
	for (unsigned int i = 0; i < m_Order; ++i)
	{
		if (distanceMatrix[i].size() > max_unsignedInt)
			return CGraph_DistMatCheckResult::tooLarge;

		unsigned int rowLength = distanceMatrix[i].size();
		if (matrixShape == CGraph_DistMatCheckResult::square && rowLength != m_Order)
			return CGraph_DistMatCheckResult::badShape;
		else if (matrixShape == CGraph_DistMatCheckResult::lowerTriangular && i > 0
				&& rowLength != distanceMatrix[i-1].size() + 1)
			return CGraph_DistMatCheckResult::badShape;
		else if (matrixShape == CGraph_DistMatCheckResult::upperTriangular && i > 0
				&& rowLength != distanceMatrix[i-1].size() - 1)
			return CGraph_DistMatCheckResult::badShape;
	}
	if (matrixShape == CGraph_DistMatCheckResult::undefined)
		throw InternalException("Code broken internally. Variable matrixShape was not changed from undefined.");

	// Check elements are valid
	for (unsigned int i = 0; i < m_Order; ++i)
	{
		for (unsigned int j = 0; j < distanceMatrix[i].size(); ++j)
		{
			if (distanceMatrix[i][j] < 0 && distanceMatrix[i][j] != -1)
				return CGraph_DistMatCheckResult::invalidElements;
		}
	}

	// Return positive result
	return matrixShape;
}



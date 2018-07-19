#include "PathFinding.h"

//AddNode function, takes a location Vector2D and pushes this onto the vector of Nodes 
void Graph::AddNode(Vector2D location)
{
	Node nu;
	nu.position = location;
	NodeVector.push_back(nu);
}

//CheckEdges function, uses nested iterators to check all possible nodes in the node vector, and which nodes can see other nodes,
//adds edges between nodes which can see other nodes to the list of edges, and the distance of that edge
void Graph::CheckEdges()
{
	float distance = 0;

	for (std::vector<Node>::iterator itX = NodeVector.begin(); itX != NodeVector.end(); itX++)
	{
		for (std::vector<Node>::iterator itY = NodeVector.begin(); itY != NodeVector.end(); itY++)
		{
			if ((itX != itY) && (StaticMap::GetInstance()->IsLineOfSight(itX->position, itY->position)))
			{
				distance = ((itX->position) - (itY->position)).magnitude();
				AddEdge(&*itX , &*itY , distance);
			}
		}
	}
}

//Adds the edge fo the list of edges, takes a from and to node and the cost of the edge
void Graph::AddEdge(Node* from, Node* to, float cost)
{
	Edge nu;
	nu.toNode = to;
	nu.fromNode = from;
	cost = GetDistance(from, to);
	to->edgeList.push_back(nu);
	from->edgeList.push_back(nu);
	
}

//gets the closest node and returns a pointer to that node, takes a position which the node will be closest too
Node* Graph::GetClosestNode(Vector2D position)
{
	//sets the closest node as nullptr
	Node* closest = nullptr;

	//initialises the distance to a large number
	float distance = 80000;
	//count integer, used to initialise the initial node as the closest
	int count = 0;

	//iterates through the vector of nodes, sets the distance to the distance between the first node
	//and the position given (if the node is visable), and sets that node as the closest and increments the count
	for (std::vector<Node>::iterator it = NodeVector.begin(); it != NodeVector.end(); it++)
	{
		if ((count == 0) && (StaticMap::GetInstance()->IsLineOfSight(it->position, position)))
		{
			distance = (position - it->position).magnitude();
			closest = &*it;
			count++;
		}
		//checks if the distance between the node and the position is smaller than the recorded distance if they are in sight of each other and sets
		//that node as the closest
		else if ((((position)-(it->position)).magnitude() < distance) && (StaticMap::GetInstance()->IsLineOfSight(it->position, position)))
		{
			distance = ((position)-(it->position)).magnitude();
			closest = &*it;
		}
	}
	//returns the closest nerd
	return closest;
}

//Returns the distance between two nodes
float Graph::GetDistance(Node* from, Node* to)
{
	return ((from->position) - (to->position)).magnitude();
}

//returns the distance between the two nodes
float Graph::Heuristic(Node* from, Node* to)
{
	return ((to->position - from->position).magnitude());
}

//if there are values on the nodelist, iterate through all nodes
//and find the lowest fScore and return the node it belongs to
Node* Graph::LowestFScore(std::list <Node*> &NodeList)
{
	if (!NodeList.empty())
	{
		std::list<Node*>::iterator it = NodeList.begin();
		Node* LowestScore(*it);
		it++;

		for (; it != NodeList.end(); it++)
		{
			if ((*it)->f < LowestScore->f)
			{
				LowestScore = (*it);
			}
		}
		return LowestScore;
	}
	else
	{
		return nullptr;
	}
	
}

//Takes a node pointer and returns the path from the pointer
//by popping previous positions till the path is empty
std::vector<Vector2D> Graph::GetPath(Node* Current)
{
	std::vector <Vector2D> path;
	while (Current->previous != nullptr)
	{
		path.push_back(Current->position);
		Current = Current->previous;
	}

	//std::reverse(path.begin(), path.end());

	return path;
}

//used to check if a list of nodes contains a specific node, using an iterator
bool Graph::Contains(Node* nodeToFind, std::list<Node*> &list)
{
	std::list<Node*>::iterator it = list.begin();

	for (; it != list.end(); it++)
	{
		if (*it == nodeToFind)
		{
			return true;
		}
	}
	return false;
}

//Takes two Vector2Ds and gets the closest nodes from those points and pathfinds
//between those nodes and returns that path
std::vector<Vector2D> Graph::Pathfind(Vector2D from, Vector2D to)
{
	Node* f = GetClosestNode(from);
	Node* t = GetClosestNode(to);

	std::vector<Vector2D> path = Pathfind(f, t);

	return path;

}

//essential pathfinding algorithm, takes two nodes, initialiseseach f and g value in the nodevector to 0,
//clears the open and closed list, pushes the from node onto the open list, adjusts the previous value
//and the g and f values (f using the heuristic function). Then uses a while loop till the open list is 
//not empty.
std::vector<Vector2D> Graph::Pathfind(Node* from, Node* to)
{
	for (std::vector<Node>::iterator it = NodeVector.begin(); it != NodeVector.end(); it++)
	{
		it->f = 0;
		it->g = 0;
	}

	closedList.clear();
	openList.clear();

	openList.push_back(from);
	from->previous = nullptr;
	from->g = 0;
	from->f = from->g + Heuristic(from, to);

	while (!(openList.empty()))
	{
		//Stores the lowerstF score in the open list
		Node* currentNode = LowestFScore(openList);
		//if currentNode is the destination node, return the path
		if (currentNode == to)
		{
			return GetPath(currentNode);
		}
		//remove the currentnode from the open list and push onto the closed list
		openList.remove(currentNode);
		closedList.push_back(currentNode);

		//iterator that iterates through all edges in the edgelist
		std::list<Edge>::iterator it = currentNode->edgeList.begin();
		for (; it != currentNode->edgeList.end(); it++)
		{
			//sets the neighbour node
			Node* neighbour = it->toNode;
			//if not on the closed list
			if (!Contains(neighbour, closedList))
			{
				//set newG
				float newG = (currentNode->g) + ((currentNode->position - neighbour->position).magnitude());
				//if not on open list or neighbour G > newG
				if (!(Contains(neighbour, openList)) || neighbour->g > newG)
				{
					//set new previous g and f
					neighbour->previous = currentNode;
					neighbour->g = newG;
					neighbour->f = newG + Heuristic(neighbour, to);
					//if not on openlist
					if (!(Contains(neighbour, openList)))
					{
						//push onto open list
						openList.push_back(neighbour);
					}
				}
			}
		}
	}
	//setup answer vector of V2Ds and return
	std::vector<Vector2D> ans;
	ans.push_back(from->position);
	return ans;
}

//draws all nodes on the map
void Graph::DrawNodes()
{
	for (std::vector<Node>::iterator it = NodeVector.begin(); it != NodeVector.end(); it++)
	{
		Renderer::GetInstance()->DrawDot(it->position, 4);
	}
}

//draws all edges on the map
void Graph::DrawEdges()
{
	for (std::vector<Node>::iterator itX = NodeVector.begin(); itX != NodeVector.end(); itX++)
	{
		for (std::list<Edge>::iterator itY = itX->edgeList.begin(); itY != itX->edgeList.end(); itY++)
		{
			Renderer::GetInstance()->DrawLine(itY->fromNode->position, itY->toNode->position, 5);
		}
	}
}

//Recursive function which calls itself and divides into rectangles, checks if the center is inside a block,
//if so, divide into 4 more rectangles. if no collision, add the center of the rectandle to the node list.
void Graph::AnalyseMap(Rectangle2D r)
{
	

	if (r.GetArea()< 15000)
	{
		return;
	}
	else if (StaticMap::GetInstance()->IsInsideBlock(r))
	{
		
		float top = r.GetTopLeft().YValue;
		float bottom = r.GetBottomLeft().YValue;
		float left = r.GetTopLeft().XValue;
		float right = r.GetBottomRight().XValue;
		float centreX = r.GetCentre().XValue;
		float centreY = r.GetCentre().YValue;

		Rectangle2D r1;
		r1.PlaceAt(top, left, centreY, centreX);

		Rectangle2D r2;
		r2.PlaceAt(top, centreX, centreY, right);

		Rectangle2D r3;
		r3.PlaceAt(centreY, left, bottom, centreX);

		Rectangle2D r4;
		r4.PlaceAt(centreY, centreX, bottom, right);


		AnalyseMap(r1);
		AnalyseMap(r2);
		AnalyseMap(r3);
		AnalyseMap(r4);
	}
	else
	{
		AddNode(r.GetCentre());
	}


}


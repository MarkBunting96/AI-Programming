#pragma once

#include "vector2D.h"
#include  "list"
#include "vector"
#include "renderer.h"
#include "staticmap.h"

//Reference to Node, enables Node to be used in Edge Class
class Node;

//Edge Class, contains two Nodes of the edge and the cost of the edge
class Edge
{
public:
	Node* fromNode;		
	Node* toNode;		
	float cost;
};

//Node Class, contains a list of edges that the node has,
//the position of the node, the Node previous to this, and
//two placeholder float variables for the A* Path finding algorithm
class Node
{
public:
	std::list<Edge> edgeList;
	Vector2D position;
	Node* previous;
	float g;			
	float f;			
private:

};

//Graph Class, private attributes/functions include
//a vector of nodes, an AddNode function,
//an AddEdge function, a GetClosestNode function, a GetDistance
//function, a Pathfind function, an openList and a closedList
//of nodes, and two Node pointers that point to the current Node
//and the nextNode.
//Public attributes/functions include an AnalyseMap functions
//a CheckEdges function, a DrawNodes function, a DrawEdges function
//a Heuristic function, a LowestFScore function, a GetPath function
//a Contains function, and another Pathfind function (one takes Nodes,
//the other takes Vector2Ds).
class Graph
{
private:
	std::vector<Node> NodeVector;										//Vector of Nodes in the map
	void AddNode(Vector2D location);									//AddNode function, takes a location
	void AddEdge(Node* from, Node* to, float cost);						//AddEdge function, takes the from and to Node, and the cost
	Node* GetClosestNode(Vector2D position);							//GetClosestNode function, takes a Vector2D and returns a pointer to a Node
	float GetDistance(Node* from, Node* to);							//GetDistance function, takes two Nodes and returns a float
	std::vector<Vector2D> Pathfind(Node* from, Node* to);				//Pathfind function, takes two Nodes and returns a vector of Vector2D's
	std::list<Node*> openList;											//list of Nodes, the openList
	std::list<Node*> closedList;										//list of Nodes, the closedList
	Node* currentPtr;													//currentPtr, points to the current Node
	Node* nextPtr;														//nextPtr, points to the next Node

public:
	void AnalyseMap(Rectangle2D r);										//AnalyseMap function, takes a Rectangle2D
	void CheckEdges();													//CheckEdges function
	void DrawNodes();													//DrawNodes function
	void DrawEdges();													//DrawEdges Function
	float Heuristic(Node* from, Node* to);								//Heuristic function, takes two pointers to Nodes (from and to), returns a float
	Node* LowestFScore(std::list <Node*> &NodeList);					//LowestFScore, takes a list of Node pointers and returns a node pointer
	std::vector<Vector2D> GetPath(Node* Current);						//GetPath function, takes a node pointer and returns a vector of Vector of Vector2Ds
	bool Contains(Node* nodeToFind, std::list<Node*> &list);			//contains function, takes a node pointer, and a list of node pointers, returns a boolean
	std::vector<Vector2D> Pathfind(Vector2D from, Vector2D to);			//Alternate Pathfind function, takes two Vector2Ds instead of nodes and returns a vector of Vector2Ds

};



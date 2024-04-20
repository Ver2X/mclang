// C++ program to concatenate two strings using
// rope data structure.
// #include <bits/stdc++.h>
#include <iostream>
using namespace std;

// Maximum no. of characters to be put in leaf nodes
const int LEAF_LEN = 2;

// Rope structure
class Rope {
public:
  Rope *left, *right, *parent;
  char *str;
  int lCount;
};

// Function that creates a Rope structure.
// node --> Reference to pointer of current root node
// l --> Left index of current substring (initially 0)
// r --> Right index of current substring (initially n-1)
// par --> Parent of current node (Initially nullptr)
void createRopeStructure(Rope *&node, Rope *par, char a[], int l, int r);

// Function that prints the string (leaf nodes)
void printstring(Rope *r);

// Function that efficiently concatenates two strings
// with roots root1 and root2 respectively. n1 is size of
// string represented by root1.
// root3 is going to store root of concatenated Rope.
void concatenate(Rope *&root3, Rope *root1, Rope *root2, int n1);
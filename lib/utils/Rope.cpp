#include "utils/Rope.h"
void createRopeStructure(Rope *&node, Rope *par, char a[], int l, int r) {
  Rope *tmp = new Rope();
  tmp->left = tmp->right = nullptr;

  // We put half nodes in left subtree
  tmp->parent = par;

  // If string length is more
  if ((r - l) > LEAF_LEN) {
    tmp->str = nullptr;
    tmp->lCount = (r - l) / 2;
    node = tmp;
    int m = (l + r) / 2;
    createRopeStructure(node->left, node, a, l, m);
    createRopeStructure(node->right, node, a, m + 1, r);
  } else {
    node = tmp;
    tmp->lCount = (r - l);
    int j = 0;
    tmp->str = new char[LEAF_LEN];
    for (int i = l; i <= r; i++)
      tmp->str[j++] = a[i];
  }
}

// Function that prints the string (leaf nodes)
void printstring(Rope *r) {
  if (r == nullptr)
    return;
  if (r->left == nullptr && r->right == nullptr)
    cout << r->str;
  printstring(r->left);
  printstring(r->right);
}

// Function that efficiently concatenates two strings
// with roots root1 and root2 respectively. n1 is size of
// string represented by root1.
// root3 is going to store root of concatenated Rope.
void concatenate(Rope *&root3, Rope *root1, Rope *root2, int n1) {
  // Create a new Rope node, and make root1
  // and root2 as children of tmp.
  Rope *tmp = new Rope();
  tmp->parent = nullptr;
  tmp->left = root1;
  tmp->right = root2;
  root1->parent = root2->parent = tmp;
  tmp->lCount = n1;

  // Make string of tmp empty and update
  // reference r
  tmp->str = nullptr;
  root3 = tmp;
}
/*
// Driver code
int main()
{
    // Create a Rope tree for first string
    Rope *root1 = nullptr;
    char a[] =  "Hi This is geeksforgeeks. ";
    int n1 = sizeof(a)/sizeof(a[0]);
    createRopeStructure(root1, nullptr, a, 0, n1-1);

    // Create a Rope tree for second string
    Rope *root2 = nullptr;
    char b[] =  "You are welcome here.";
    int n2 = sizeof(b)/sizeof(b[0]);
    createRopeStructure(root2, nullptr, b, 0, n2-1);

    // Concatenate the two strings in root3.
    Rope *root3 = nullptr;
    concatenate(root3, root1, root2, n1);

    // Print the new concatenated string
    printstring(root3);
    cout << endl;
    return 0;
}
*/
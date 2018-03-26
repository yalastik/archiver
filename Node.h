//block look like:  <offs,len,ch> example: <3,2,d>
class Node
{
public:
    //everything public
    short offs;
    short len;
    char ch;

    Node(short o, short l, char c) : offs(o), len(l), ch(c)
    {};

    Node()
    {};

};
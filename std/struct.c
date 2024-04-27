int main()
{
    struct t
    {
        int x;
    };
    int t = 1;
    struct t y;
    y.x = 2;
    return t + y.x;
}
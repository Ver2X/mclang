int main() {
    int a = 7;
    int d = 3;
    int e = 10;
    {
        int a = 2;
        int c = a + 2;
        {
            int h = d + c;
            e = h + 3;
        }
    }
    return a + e;
}
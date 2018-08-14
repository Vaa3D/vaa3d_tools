#ifndef NEW_STRUCT_H
#define NEW_STRUCT_H
struct Coordinate
{
    int x;
    int y;
    int z;
    bool operator == (const Coordinate &b) const
    {
        return (x==b.x && y==b.y && z == b.z);
    }
};
struct Max_level
{
    int level1;
    int level2;
};
struct Soma
{
  double x_b;
  double x_e;
  double y_b;
  double y_e;
  double z_b;
  double z_e;
};
uint qHash(const Coordinate key)
{
    return key.x + key.y + key.z;
}
uint qHash(const vector<Coordinate> key)
{
    return key[0].x + key[0].y + key[0].z;
}
#endif // NEW_STRUCT_H

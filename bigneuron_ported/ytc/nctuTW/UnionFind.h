#ifndef UNION_FIND_H
#define UNION_FIND_H

class MyUnionFind3D
{

public:

	//MyUninonFind3D(unsigned long int L){ UnionFindInitialize(L); };
	unsigned long int Length;
	unsigned long int	*Parent;
	unsigned long int	*Size;
//unsigned char	*Image;

	~MyUnionFind3D(){ delete [] Parent; delete[] Size; }
	void UnionFindInitialize(unsigned long int length)
	{
		Parent = new unsigned long int [length];
		Size = new unsigned long int [length];
		for(unsigned long int i=0; i< length; i++)
		{
			Parent[i] = i;
			Size[i] = 1;
		}

		Length = length;
	}

	void Union (unsigned long i, unsigned long j)
	{
		if (Size[i]>Size[j]) 
		{
			Parent[j] = i;
			Size[i] = Size[j]+Size[i];
		}
		else {
			Parent[i] = j;
			Size[j] = Size[j]+Size[i];		
		}
	}

	unsigned long Find (unsigned long i)
	{
		unsigned long S, next;
		S = i;
		while (Parent[S] != S) S = Parent[S];
		while (Parent[i] != S) {
			next = Parent[i];
			Parent[i] = S;
			i = next;
		};
		return S;
	}
};

#endif
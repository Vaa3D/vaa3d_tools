using System;

// ref: http://hos.ac/blog/#blog0001
class SkewHeap<T> where T : IComparable<T>
{
    private HeapNode root;

    public SkewHeap()
    {
        root = null;
    }

    private class HeapNode
    {
        public T value;
        public HeapNode l, r;

        public HeapNode(T value)
        {
            this.value = value;
            l = null; r = null;
        }

        public HeapNode(T value, HeapNode l, HeapNode r)
        {
            this.value = value;
            this.l = l; this.r = r;
        }
    }

    public bool Empty()
    {
        return root == null;
    }

    public void Push(T value)
    {
        if (root == null)
        {
            root = new HeapNode(value);
        }
        else
        {
            var node = new HeapNode(value);
            root = Meld(root, node);
        }
    }

    public T Pop()
    {
        if (root == null)
        {
            throw new InvalidOperationException("SkewHeap is empty");
        }

        T value = root.value;
        root = Meld(root.l, root.r);
        return value;
    }

    public void Merge(SkewHeap<T> heap)
    {
        root = Meld(root, heap.root);
    }

    private static HeapNode Meld(HeapNode a, HeapNode b)
    {
        if (a == null) return b;
        if (b == null) return a;

        if (a.value.CompareTo(b.value) > 0) Swap(ref a, ref b);

        var heap = new HeapNode(a.value, a.l, Meld(a.r, b));
        Swap(ref heap.l, ref heap.r);

        return heap;
    }

    private static void Swap<U>(ref U a, ref U b)
    {
        U tmp = a; a = b; b = tmp;
    }
}

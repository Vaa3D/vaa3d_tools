
nspointer select_neighbor_id( nspointer *neighbors )
{
    nsint i,j,used,hipos;
    nspointer ids[26];
    nsint counts[26];

    for(i=0,used=0;i<26;i++)
    {
        if(neighbors[i]!=0)
        {
            for(j=0;j<used;j++)
            {
                if(neighbors[i]==ids[j])
                {
                    counts[j]++;
                    break;
                }
            }
            if(j==used)
            {
                ids[used] = neighbors[i];
                counts[used]=1;
                used++;
            }
        }
    }


    /* debug *//*
    printf("\nFound %d ids:",used);
    for(i=0;i<used;i++)
    {
        printf("\nid=%d , count=%d", ids[i],counts[i]);
    }*/

    
    /* determine majority */
    if(used!=0)
    {
        hipos = 0;
        for(i=1;i<used;i++)
        {
            if(counts[i]>counts[hipos]) hipos=i;
        }
        return ids[hipos];
    }

    return NULL;
}
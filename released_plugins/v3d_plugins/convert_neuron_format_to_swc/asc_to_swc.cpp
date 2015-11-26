#include "asc_to_swc.h"
#include <iostream>
#include <basic_surf_objs.h>
#include <sstream>


bool lookFor(char * string, ifstream* in) {
    char c[500];
    *in >> c;
    while((!in->eof() || !in->fail()) && strncmp(c,string,strlen(string))!=0){
        *in >> c;
    }
    if (memcmp(string, c, strlen(string)) == 0) {
        return true;
    } else
        return false;
}

void asc_to_swc::readASC_file(NeuronTree &nt, char* fname_asc)
{
    fname_asc;

    nt.name = fname_asc;
    nt.comment = "From .ASC file: ";
    nt.comment += fname_asc;
    OpenNeuroL(nt, fname_asc);
}

void asc_to_swc::add(NeuronTree &nt, int id, int type, double x, double y, double z, double radius, int pid)
{
    NeuronSWC tmpswc;
    tmpswc.x = x; tmpswc.y = y; tmpswc.z = z;
    tmpswc.parent = pid;
    tmpswc.n = id;
    tmpswc.radius = radius;
    tmpswc.type = type;
    nt.listNeuron.push_back(tmpswc);
}

int asc_to_swc::NeurolAdd(NeuronTree &nt, ifstream * in, int id, int type) {

    int biforc[1000];
    biforc[0] = -1;
    biforc[1] = -1;
    int index = 0;
    int lastindex = 0;
    int lookForCloseParenthesis = 0;
    int pid;
    int markctr = 0;
    pid = -1;
    int endTree = 0;
    int level=1;
    while (in->peek() != -1 && endTree == 0) {
        int save = 0;
        int foundBar = 0;
        index = 0;
        lookForCloseParenthesis = 1;
        save = 0;
        char c = 'y';
        double x, y, z, d;

        while (c != '(' && c != '|' && in->peek() != -1) {
            in->peek();
            c = in->get();
            if (lookForCloseParenthesis == 1 && c == ')'){
                lookForCloseParenthesis = 2;
                level--;
                //for test
                printf("%d; %d\n",level,index);
            }

            if (c == 10 || c == 13) {
                index = 0;
                lookForCloseParenthesis = 1;
            }
            if (c == ' ')
                index++;
            if (lookForCloseParenthesis == 2) {
                char k[100];
                if(in->peek()!=10 && in->peek()!=13 && in->peek()!=-1)
                    *in >> k;

                if (strcmp(k, "tree") == 0 || level==0) {
                    index = 0;
                    lastindex = 0;
                    biforc[2] = 1;
                    pid = 1;
                    endTree = 1;
                    break;
                }// else if (strcmp(k, "split") == 0) {
                else{
                    lookForCloseParenthesis = 1;
                }
            }
        }

        if (c == '|')
            foundBar = 1;

        //remove spaces after '('
        while (in->peek() == ' ' && in->peek() != -1)
            in->get();

        //if the line does not contains anything then save the previous id
        if (in->peek() == 10 || in->peek() == 13){
            save = 1;
        }
        // when found a '|' correct for pid
        if (lastindex == index && foundBar == 1) {
            int jj = 0;
            foundBar = 0;
            pid = biforc[index];
        }
        if (lastindex > index && save == 1) {
            //after a termination
            pid = biforc[index];
            lastindex = index;
        }
        if (lastindex < index && save == 1) {
            //new biforcation
            biforc[index] = id;
            lastindex = index;
        }
        //probable location for error!!
        if (in->peek() == -1) {
            int stop = 1;
        }
        // to skip marker blocks. when R is not present the line is skipped.
        int pos;
        pos = in->tellg();
        char tmp[200];
        in->getline(tmp, 200);
        if (strstr(tmp, "Marker") != NULL || strstr(tmp, "Spines") != NULL
                || strstr(tmp, "spines") != NULL || strstr(tmp, "marker")
                != NULL) {
            markctr++;

            while (strstr(tmp, "End of markers") == NULL) {
                pos = in->tellg();
                in->getline(tmp, 200);

            }

        }
        //adding second parameter here or eclipse is showing it as invalid argument error.
        in->seekg(pos,ios::beg);

        //if a number follow get the segment cooordinates
        if ((in->peek() >= '0' && in->peek() <= '9') || in->peek() == '-') {
            id++;
            //get segment
            c = in->peek();
            x = -1;
            y = -1;
            z = -1;
            *in >> x;
            *in >> y;
            *in >> z;
            *in >> d;
            add(nt, id, type, x, y, z, d, pid);
            pid = id;
        }else if(c == '('){
            //hb: increase lvl
            level++;
        }
        //go to end line
        while (c != 13 && c != -1 && c != 10) {
            in->peek();
            c = in->get();
        }

    }
    return id;
}

double * asc_to_swc::getValues(ifstream * in, double * ret) {
    char c = 'y';
    double xSum = 0, ySum = 0, zSum = 0, rSum = 0, xSq = 0, ySq = 0, zSq = 0,
            x, y, z, r;
    int count = 0;

    {
        x = -1;
        y = -1;
        z = -1;
        while (c != -1) {

            while (c != '(' && c != -1 && c != ')') {
                in->peek();
                c = in->get();
            }
            if (c == ')')
                break;
            //remove spaces after '('. added on 09/10/2010 by Sridevi. Otherwise the soma points are skipped
            while (in->peek() == ' ' && in->peek() != -1)
                in->get();
            if ((in->peek() >= '0' && in->peek() <= '9') || in->peek() == '-') {
                *in >> x;
                *in >> y;
                *in >> z;
                *in >> r;
                xSum += x;
                ySum += y;
                zSum += z;
                rSum += r;
                xSq += x * x;
                ySq += y * y;
                zSq += z * z;
                count++;
            }
            //go to end line
            while (c != 13 && c != 10) {
                in->peek();
                c = in->get();

            }

            if (in->peek() == ')')
                break;
        }

        ret[0] = xSum / count;
        ret[1] = ySum / count;
        ret[2] = zSum / count;

        double tmp = 0;
        double xvar = 0, yvar = 0, zvar = 0;
        tmp = xSq / count - ret[0] * ret[0];
        if (tmp > 0)
            xvar = sqrt(tmp);
        tmp = ySq / count - ret[1] * ret[1];
        if (tmp > 0)
            yvar = sqrt(tmp);
        tmp = zSq / count - ret[2] * ret[2];
        if (tmp > 0)
            zvar = sqrt(tmp);
        double soma = (xvar + yvar + zvar) / 3 * 4;
        ret[3] = soma;

        c = in->peek();
    }//else
    return ret;

}
#include <limits>
void asc_to_swc::OpenNeuroL(NeuronTree &nt, char* name) {
    ifstream * in = new ifstream(name, ios::in | ios::binary);

    int cellBody_cnt = 0;
    char buffer [33];

    //store biforc
    double x = 0, y = 0, z = 0;

    double ret[5];
    //initialize the ret array to a big integer
    ret[0] = ret[1] = ret[2] = ret[3] = ret[4] = -999999999;

    //modified the code such that all cellbody tags are searched at once sri 07/22/2010
    if (in->fail())
        in->clear();
    in->seekg(0, ios::beg);
    int id = 1; int pid = -1;
    while (!in->fail()) {
        if (lookFor("(CellBody)", in)) {
            getValues(in, ret);
        }

        if((ret[0]!=-999999999) && (ret[1]!=-999999999)){
            add(nt, id,1,ret[0],ret[1],ret[2],ret[3],pid);
            pid = id;id = id+1;
            cellBody_cnt++;
        }

        ret[0]=ret[1]=ret[2]=ret[3]=ret[4]=-999999999;
    }
    if (in->fail())
        in->clear();
    in->seekg(0, ios::beg);

    ret[0]=ret[1]=ret[2]=ret[3]=ret[4]=-999999999;

    while (!in->fail()) {
        if (lookFor("(Closed)", in)) {
            getValues(in, ret);
        }

        //adding the centroid point of each contour to the coverted file
        //insert soma. modified the code because soma insertion is failing hence checking on the ret[] array to add soma sri 07/22/2010
        if((ret[0]!=-999999999) && (ret[1]!=-999999999)){
            add(nt, id,1,ret[0],ret[1],ret[2],ret[3],pid);
            pid = id;id = id+1;
            cellBody_cnt++;
        }

        ret[0]=ret[1]=ret[2]=ret[3]=ret[4]=-999999999;

    }

    stringstream ss;
    ss << cellBody_cnt;

//    if(cellBody_cnt > 1){
//        strcpy(d[2], "# The original file has ");
//        //itoa(cellBody_cnt,buffer,10);
//        strcat(d[2],ss.str().c_str());
//        strcat(d[2], " soma contours that are averaged into ");
//        strcat(d[2], ss.str().c_str());
//        strcat(d[2], " soma points");
//    }else if(cellBody_cnt == 1){
//        strcpy(d[2], "#The original file has a single soma contour that is averaged into 3 soma points");

//    }else{
//        strcpy(d[2], "#The original file has no soma");

//    }

    if (in->fail())
        in->clear();
    in->seekg(0, ios::beg);
    //int id = 1;
    int dendctr = 0;
    while (in->peek() != -1) {
        dendctr++;
        lookFor("(Dendrite)", in);
        id = NeurolAdd(nt, in, id, 3);
    }

    if (in->fail())
        in->clear();
    in->seekg(0, ios::beg);
    int axonctr = 0;
    while (!in->eof()) {
        axonctr++;
        if (in->peek() == -1) {
            break;
        }
        if (lookFor("(Axon)", in)) {
            id = NeurolAdd(nt, in, id, 2);
        }
    }

    if (in->fail())
        in->clear();
    in->seekg(0, ios::beg);
    int apictr = 0;
    while (in->peek() != -1) {
        apictr++;

        lookFor("(Apical)", in);
        id = NeurolAdd(nt, in, id, 4);
    }

    in->close();
}

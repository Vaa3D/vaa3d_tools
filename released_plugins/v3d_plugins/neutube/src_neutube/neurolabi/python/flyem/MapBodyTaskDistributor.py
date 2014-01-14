import os;

class SkeletonizeTaskDistributor:
    '''
    classdocs
    '''

    def __init__(self):
        '''
        Constructor
        '''
        self.commandPath = '';
        self.bodyList = list();
        self.bodyDir = '';
        self.jobNumber = 5;
        self.usingCluster = False;
        self.args = '--sobj --minsize 0'
        
    def setCommandPath(self, path):
        self.commandPath = path;
        
    def setBodyList(self, bodyList):
        self.bodyList = bodyList;
        
    def setBodyDir(self, path):
        self.bodyDir = path;
        
    def setSwcDir(self, path):
        self.swcDir = path;
    
    def setJobNumber(self, n):
        self.jobNumber = n;

    def setArgs(self, args):
        self.args = args;
        
    def useCluster(self, using):
        self.usingCluster = using;
        
    def getFullCommand(self, index):
        bodyFile = self.bodyDir + '/' + str(self.bodyList[index]) + '.sobj';
        swcFile = self.swcDir + '/' + str(self.bodyList[index]) + '.swc';
        return self.commandPath + ' ' + bodyFile + " " + self.args + ' -o ' + swcFile;
        
    def generateScript(self, outputDir):
        #split bodies
        subscripts = list();
        subscriptFile = list();
        jobNumber = min(len(self.bodyList), self.jobNumber);
        for i in range(0, jobNumber):
            subscripts.append(outputDir + '/skeletonize_' + str(i + 1) + '.sh');
            subscriptFile.append(open(subscripts[i], "w"));
        
        index = 0;
        while index < len(self.bodyList):
            for i in range(0, jobNumber):
                swcFile = self.swcDir + '/' + str(self.bodyList[index]) + '.swc';
                subscriptFile[i].write('touch ' + swcFile + '.process\n');
                subscriptFile[i].write(self.getFullCommand(index) + '\n');
                index = index + 1;
                if index >= len(self.bodyList):
                    break;
        
        for i in range(0, jobNumber):
            subscriptFile[i].close();
            
        masterScript = outputDir + '/run.sh';
        f = open(masterScript, "w");
        for i in range(0, jobNumber):
            f.write('sh ' + os.path.abspath(subscripts[i]) + ' > ' + os.path.abspath(subscripts[i]) + '.out &\n');
        f.close();

if __name__ == '__main__':
    from os.path import expanduser
    home = expanduser("~")

    distr = SkeletonizeTaskDistributor();
    distr.setBodyList([1, 2]);
    distr.setCommandPath(home + '/Work/neutube/neurolabi/cpp/skeletonize-build-Qt_4_8_1_gcc-Debug/skeletonize');
    print distr.getFullCommand(0);
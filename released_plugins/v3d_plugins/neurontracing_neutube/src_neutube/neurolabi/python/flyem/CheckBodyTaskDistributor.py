import os
import glob

class CheckBodyTaskDistributor:
    def __init__(self):
        self.commandPath = '';
        self.bodyList = list();
        self.bodyDir = '.';
        self.resultDir = '.';
        self.resultSuffix = '';
        self.resultFormat = 'swc';
        self.scriptPrefix = 'process_';
        self.jobNumber = 5;
        self.usingCluster = False;
        self.args = '';

    def setCommandPath(self, path):
        self.commandPath = path;
        
    def setBodyList(self, bodyList):
        self.bodyList = bodyList;
        
    def loadBodyListFromDir(self, dirPath):
        self.bodyList = list();
        bodyFileList = glob.glob(dirPath + '/*.sobj');
        for bodyFile in bodyFileList:
            bodyId = int(os.path.splitext(os.path.basename(bodyFile))[0]);
            self.bodyList.append(bodyId);
        
    def setBodyDir(self, path):
        self.bodyDir = path;
        
    def setResultDir(self, path):
        self.resultDir = path;
    
    def setJobNumber(self, n):
        self.jobNumber = n;

    def setArgs(self, args):
        self.args = args;
        
    def useCluster(self, using):
        self.usingCluster = using;
        
    def getResultFile(self, bodyId):
        outputFile = self.resultDir + '/' + str(bodyId) + self.resultSuffix;
        if self.resultFormat:
            outputFile = outputFile + '.' + self.resultFormat;
            
        return outputFile;
        
    def getFullCommand(self, index):
        bodyFile = self.bodyDir + '/' + str(self.bodyList[index]) + '.sobj';
        outputFile = self.getResultFile(self.bodyList[index]);
        command = self.commandPath + ' ' + bodyFile + " " + self.args + ' -o ' + outputFile;
        
        return command;
        
    def generateScript(self, outputDir):
        #split bodies
        subscripts = list();
        subscriptFile = list();
        jobNumber = min(len(self.bodyList), self.jobNumber);
        for i in range(0, jobNumber):
            subscripts.append(outputDir + '/' + self.scriptPrefix + str(i + 1) + '.sh');
            subscriptFile.append(open(subscripts[i], "w"));
        
        index = 0;
        while index < len(self.bodyList):
            for i in range(0, jobNumber):
                outputFile = self.getResultFile(self.bodyList[index]);
                subscriptFile[i].write('touch ' + outputFile + '.process\n');
                subscriptFile[i].write('if [ ! -f ' + outputFile + ' ]; then\n');
                subscriptFile[i].write('  ' + self.getFullCommand(index) + '\n');
                subscriptFile[i].write('fi\n');
                index = index + 1;
                if index >= len(self.bodyList):
                    break;
        
        for i in range(0, jobNumber):
            subscriptFile[i].close();
            
        masterScript = outputDir + '/run.sh';
        f = open(masterScript, "w");
        for i in range(0, jobNumber):
            shcommand = 'sh ' + os.path.abspath(subscripts[i]) + ' > ' + os.path.abspath(subscripts[i]) + '.out';
            if self.usingCluster:
                shcommand = 'qsub -P flyemproj -N em_skeleton -j y -o /dev/null -b y -cwd -V ' + "'" + shcommand + "'";
            else:
                shcommand = shcommand + " &";
            f.write(shcommand + '\n');
        f.close();

if __name__ == '__main__':
    from os.path import expanduser
    home = expanduser("~")

    distr = CheckBodyTaskDistributor();
    distr.setBodyList([1, 2]);
    distr.setBodyDir('.');
    distr.setCommandPath(home + '/Users/zhaot/Work/neutube/neurolabi/cpp/CheckBody_Qt5_Debug/checkbody');
    print distr.getFullCommand(0);
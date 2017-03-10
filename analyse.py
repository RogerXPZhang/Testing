

inputFile = "inner_sproket.txt"
outputFile = inputFile[:-4] + "_output.csv"
baselineFile = inputFile[:-4] + "_baseline.csv"

class TestData():
    def __int__(self):
        return

    def fileConvertor(self,filename):
        f1 = open(filename,'r')
        outFile = open(outputFile,'w')
        text = f1.read()
        startpos = text.find("FE4A04",0)
        while(startpos != -1):
            startpos = text.find("FE4A04",startpos+1)
            outFile.write(str(int(text[startpos+6:startpos+10],16))+','+str(int(text[startpos+10:startpos+14],16))+'\n')
        f1.close()
        outFile.close()

    def setBaseline(self,baseline):
        f1 = open(outputFile,'r')
        text = f1.read()



app = TestData()
app.fileConvertor(inputFile)
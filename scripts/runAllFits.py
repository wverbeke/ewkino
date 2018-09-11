import os
import sys 
import time

def readSignificance( fileName ):
    with open( fileName ) as output :
        for line in output:
            for subline in line.split('\n'):
                if 'Significance:' in subline:
                    subline = subline.replace( 'Significance: ', '' )
                    return subline 
    return 'Invalid output file'


def readSignalStrength( fileName ):
    with open( fileName ) as output : 
        for line in output:
            for subline in line.split('\n'):
                if 'Best fit r: ' in subline:
                    subline = subline.replace('Best fit r: ', '')
                    subline = subline.replace('  (68% CL)', '')
                    return subline
    return 'Invalid output file'

def signalStrengthToFloats( signalStrength_string ):
    central = float(signalStrength_string.split()[0])
    uncDown = abs( float(signalStrength_string.split()[-1].split('/')[0]) )
    uncUp = abs( float(signalStrength_string.split()[-1].split('/')[1]) )
    return [central, uncDown, uncUp]


def systUnc( totalUnc, statUnc ):
    syst_unc = totalUnc*totalUnc - statUnc*statUnc
    syst_unc = syst_unc**0.5
    return syst_unc


def makeScript( fileName ):
    script = open( fileName, 'w' )
    script.write('export SCRAM_ARCH=slc6_amd64_gcc530\n')
    script.write('cd /user/wverbeke/CMSSW_8_1_0/src\n')
    script.write('source /cvmfs/cms.cern.ch/cmsset_default.sh\n')
    script.write('eval `scram runtime -sh`\n')
    currentDir = os.path.dirname( os.path.abspath( __file__ ) )
    script.write('cd ' + currentDir + '\n')
    return script


def submitScript( scriptName, wallTime ):
    os.system( 'qsub ' + scriptName + ' -l walltime=' + wallTime)


def evaluateResults( card, script): 

    #make workspace for card
    command_workspace = 'text2workspace.py '
    script.write( command_workspace + card + '\n')

    #name of resulting workspace 
    workspace = card.split('.')[0] + '.root'

    #compute observed significance
    command_significance = 'combine -M ProfileLikelihood --signif --cminDefaultMinimizerStrategy 0 '
    outputFile = 'output_' + card + 'obsSig'
    script.write( command_significance + workspace + ' > ' + outputFile + ' 2> ' + outputFile + '\n')

    #compute expected (a priori) significance 
    outputFile = 'output_' + card + 'expSig'
    expectedOption = ' -t -1 --expectSignal=1'
    script.write(command_significance + workspace + expectedOption + ' > ' + outputFile + ' 2> ' + outputFile + '\n')

    #compute signal strength
    outputFile = 'output_' + card + 'sigStrength'
    command_signal_strength = 'combine -M FitDiagnostics --saveNormalizations --cminDefaultMinimizerStrategy 0 '
    script.write( command_signal_strength + workspace + ' > ' + outputFile + ' 2> ' + outputFile + '\n')

    #compute stat only signal strength
    outputFile = 'output_' + card + 'sigStrength_statOnly'
    #command_signal_strength_statOnly = 'combine -M FitDiagnostics --saveNormalizations --cminDefaultMinimizerStrategy 0 --profilingMode=none '
    command_signal_strength_statOnly = 'combine -M FitDiagnostics --saveNormalizations --profilingMode=none '
    script.write( command_signal_strength_statOnly + workspace + ' > ' + outputFile + ' 2> ' + outputFile + '\n')
    


def combineCardsCommand(cardList, combinationName):
    combineCommand = 'combineCards.py'
    for card in cardList :
        combineCommand += ' ' + card
    combineCommand += (' > ' +combinationName) 
    return combineCommand


def combineCards(cardList, name):
    combineCommand = combineCardsCommand(cardList, name)
    os.system(combineCommand)


def evaluateCombinedCard( cardList, name, script):
    combineCommand = combineCardsCommand(cardList, name) 
    script.write(combineCommand + '\n')
    evaluateResults(name, script )


def newFileWithoutLines(oldFileName, newFileName, stringToRemove):
    lines = []
    with open(oldFileName) as f:
        for line in f.readlines():
            if stringToRemove in line:
                continue
            lines.append(line)
    with open(newFileName, 'w') as f:
        f.writelines(lines)


def countOccurrences(fileName, stringToCount):
    count = 0 
    with open(fileName) as f:
        for line in f.readlines():
            if stringToCount in line:
                count += 1
    return count


def countStatNuisances(fileName):
    return countOccurrences(fileName, '_stat_')


def replaceNumberOfNuisances(fileName):
    content = []
    with open(fileName) as f:
        content = f.readlines()
    new_content = []
    for line in content:   
        if 'kmax' in line:
            oldNumNuisances = int( line.split(' ')[1] )
            old_file = fileName.split('_noStat')[0] + '.txt'
            numStatNuisances = countStatNuisances( old_file )
            newNumNuisances = oldNumNuisances - numStatNuisances
            line = line.replace( str(oldNumNuisances), str(newNumNuisances) )
        new_content.append(line)
    with open(fileName, 'w') as f:
        f.writelines( new_content )


def addAutoMCStats(fileName):
    with open(fileName, 'a') as file:
        file.write('* autoMCStats 0\n')


def makeAutoMCDataCard(datacardName):
    newCardName = datacardName.split('.')[0] + '_noStat.txt'
    newFileWithoutLines(datacardName, newCardName, '_stat_')
    replaceNumberOfNuisances(newCardName)
    addAutoMCStats(newCardName)
    return newCardName


def jobsAreRunning():
    #first make sure qstat worked
    qstatWorked = False
    while not qstatWorked:
        os.system('qstat -u$USER > check_running_jobs.txt')
        with open('check_running_jobs.txt') as f :
            qstatWorked = True
            for line in f:
                if 'qstat:' in line:
                    qstatWorked = False

    with open('check_running_jobs.txt') as f :
        for line in f:
            if 'combine' in line :
                return True
    return False


def runAllFits(): 
    if len( sys.argv ) == 1 : # no extra command line options given 
        signal_regions = [ "1bJet23Jets", "1bJet4Jets", "2bJets" ]
        years = [ "2016", "2017" ]
        control_regions = [ "WZ", "ZZ" ]
        
        
        cards_SR_2016 = [ "datacard_onZ_" + SR + "_2016.txt" for SR in signal_regions ]
        cards_SR_2017 = [ "datacard_onZ_" + SR + "_2017.txt" for SR in signal_regions]
        cards_SR_total = cards_SR_2016 + cards_SR_2017
        
        cards_CR_2016 = [ "datacard_" + CR + "_2016.txt" for CR in control_regions ]
        cards_CR_2017 = [ "datacard_" + CR + "_2017.txt" for CR in control_regions ]
        cards_CR_total = cards_CR_2016 + cards_CR_2017
        
        cards_SR_CR_2016 = cards_SR_2016 + cards_CR_2016
        cards_SR_CR_2017 = cards_SR_2017 + cards_CR_2017
        cards_SR_CR_total = cards_SR_total + cards_CR_total
        
        for card in cards_SR_total :
            scriptName = 'combine_' + card
            script = makeScript( scriptName )
            autoMC_card = makeAutoMCDataCard( card )
            evaluateResults( autoMC_card, script)
            script.close()
            submitScript( scriptName , '00:05:00')
    
        combinations = [cards_SR_2016, cards_SR_2017, cards_SR_CR_2016, cards_SR_CR_2017, cards_SR_total, cards_SR_CR_total]
        combination_names = ['2016totalSRCard.txt', '2017totalSRCard.txt', '2016totalCard.txt', '2017totalCard.txt', 'totalSRCard.txt', 'totalCard.txt']
        for combination, name in zip(combinations, combination_names) :
            scriptName = 'combine_' + name
            combineCards(combination, name)
            script = makeScript( scriptName )
            autoMC_card = makeAutoMCDataCard( name )
            evaluateResults( autoMC_card, script )
            script.close()
            submitScript( scriptName , '00:05:00')
    
        while( jobsAreRunning() ):
            time.sleep(5)
   
        #mapping of datacard names to category names 
        #naming_map = {'datacard_onZ_1bJet23Jets_2016.txt' : '1 b jet, 2-3 jets',
        #    'datacard_onZ_1bJet4Jets_2016.txt' : '1 b jet, 4 jets',
        #    'datacard_onZ_2bJets_2016.txt' : '2 b jets',
        #    'datacard_onZ_1bJet23Jets_2017.txt' : '1 b jet, 2-3 jets',
        #    'datacard_onZ_1bJet4Jets_2017.txt' : '1 b jet, 4 jets',
        #    'datacard_onZ_2bJets_2017.txt' : '2 b jets'
        #    }


        
        #print results
        for card in cards_SR_total + combination_names: 
            card = card.split('.')[0] + '_noStat.txt'
            observed_significance = readSignificance( 'output_' + card + 'obsSig')
            observed_significance = float( observed_significance )
            expected_significance = readSignificance( 'output_' + card + 'expSig' )
            expected_significance = float( expected_significance )

            signal_strength = readSignalStrength( 'output_' + card + 'sigStrength' )
            signal_strength_withUnc = signalStrengthToFloats( signal_strength )
            signal_strength = signal_strength_withUnc[0]
            signal_strength_uncDown = signal_strength_withUnc[1]            
            signal_strength_uncUp = signal_strength_withUnc[2]

            signal_strength_statOnly = readSignalStrength( 'output_' + card + 'sigStrength_statOnly' )
            signal_strength_statOnly_withUnc = signalStrengthToFloats( signal_strength_statOnly )
            signal_strength_statOnly = signal_strength_statOnly_withUnc[0]
            signal_strength_statOnly_uncDown = signal_strength_statOnly_withUnc[1]
            signal_strength_statOnly_uncUp = signal_strength_statOnly_withUnc[2]
            signal_strength_systOnly_uncUp = systUnc( signal_strength_uncUp, signal_strength_statOnly_uncUp)
            signal_strength_systOnly_uncDown = systUnc( signal_strength_uncDown, signal_strength_statOnly_uncDown)
    
            print( 'Observed (expected) significance for {0} is {1:1.3f} ({2:1.3f})'.format(card, observed_significance, expected_significance)  )
            print( 'Observed signal strength for {0} is {1:1.3f} -{2:1.2f} +{3:1.2f}'.format(card, signal_strength, signal_strength_uncDown, signal_strength_uncUp)  )
            print( 'Observed signal strength for {0} is {1:1.3f} -{2:1.2f} +{3:1.2f}(stat) -{4:1.2f} +{5:1.2f}(syst)'.format(card, signal_strength_statOnly, 
                signal_strength_statOnly_uncDown, signal_strength_statOnly_uncUp, signal_strength_systOnly_uncDown, signal_strength_systOnly_uncUp) )
            print( '\n' )
    
        os.system('rm ' + 'combine_' + '*')
        os.system('rm ' + 'output_' + '*')


if __name__ == '__main__' :
    #card = 'datacard_onZ_1bJet23Jets_2016.txt'
    #autoMC_card = makeAutoMCDataCard( card )
    #os.system('cat ' + autoMC_card) 
    runAllFits()

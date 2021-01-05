########################################
# some small functions for general use #
########################################

def year_and_lumi_from_samplelist(samplelist_name):
    # return year (string) and luminosity (float) associated to a sample list
    # WARNING: depends on naming convention of sample lists.
    slname = samplelist_name
    if '/' in slname: slname = slname[slname.rfind('/'):]
    year = ''
    lumi = -1.
    if '2016' in slname: year = '2016'; lumi = 35900
    elif '2017' in slname: year = '2017'; lumi = 41500
    elif '2018' in slname: year = '2018'; lumi = 59700
    else:
	print('### WARNING ###: year not recognized from samplelist '+str(slname))
    return (year,lumi)

def data_type_from_samplelist(samplelist_name):
    # determine whether samplelist contains data or MC files
    # WARNING: depends on naming convention of sample lists.
    slname = samplelist_name
    if '/' in slname: slname = slname[slname.rfind('/'):]
    dtype = ''
    if 'MC' in slname: dtype = 'MC'
    elif 'data' in slname: dtype = 'data'
    else:
	print('### WARNING ###: data type not recognized from samplelist '+str(slname))
    return dtype

def isdata_from_filepath(filepath):
    # determine whether a file represents data
    if('2016data' in filepath or '2017data' in filepath or '2018data' in filepath): return True
    return False
    
def year_from_filepath(filepath):
    # determine the year based on the path to the file
    if('Summer16' in filepath): return '2016'
    elif('Fall17' in filepath): return '2017'
    elif('Autumn' in filepath): return '2018'
    else: print('### WARNING ###: cannot determine year from path to file: '+filepath)
    return None

def subselect_inputfiles(inputfiles,selection_type):
    # inputfiles is a list of dicts, e.g. given as output by extendsamplelist
    # inputfiles is assumed to consist of only data or only simulation, no mixture.
    isdata = inputfiles[0]['process_name']=='data'
    if selection_type=='3tight':
	# remove ZG as it is fully included in DY
        #return [f for f in inputfiles if not 'ZGToLLG' in f['file']]
	# alternative: use all files (make sure explicit overlap removal is in event selection!)
	return inputfiles
    if selection_type=='3prompt':
	# only prompt simulation needed here
	if isdata:
	    print('### WARNING ###: trying to run 3prompt selection on data, which does not make sense.')
	    print('                 run 3tight selection instead, which is equivalent for data.')
	    return None
	return [f for f in inputfiles if f['process_name']!='nonprompt']
    if selection_type=='fakerate':
	# data and prompt simulation needed in this case
	return [f for f in inputfiles if f['process_name']!='nonprompt']
    if selection_type=='2tight':
	# only nonprompt simulation needed here
	if isdata:
	    print('### WARNING ###: trying to run 2tight selection on data...')
	    print('                 run fakerate selection instead.')
	    return None
	return [f for f in inputfiles if f['process_name']=='nonprompt']
    print('### ERROR ###: selection_type not recognized: '+selection_type)
    return None

def subselect(stringlist,tagstodiscard=[],tagstokeep=[]):
    ### generic function to subselect strings from a list of strings
    sellist = []
    for s in stringlist:
        # if tags to keep are specified, check if any is there, else discard
        if(len(tagstokeep)>0):
            keep = False
            for t in tagstokeep:
                if t in s: keep = True; break
            if not keep: continue
        # for all tags to discard, check if none is there, else discard
        keep = True
        for t in tagstodiscard:
            if t in s: keep = False; break
        if not keep: continue
        sellist.append(s)
    return sellist

### test section ###
if __name__ == "__main__":
    testname = 'samplelist_tzq_2016_MC'
    print(year_and_lumi_from_samplelist(testname))
    print(data_type_from_samplelist(testname))

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
    

### test section ###
if __name__ == "__main__":
    testname = 'samplelist_tzq_2016_MC'
    print(year_and_lumi_from_samplelist(testname))
    print(data_type_from_samplelist(testname))

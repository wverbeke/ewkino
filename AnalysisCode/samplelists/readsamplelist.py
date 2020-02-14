#######################################################
# collection of python functions to read sample lists #
#######################################################
# grouping these functions here will avoid copy pasting and allow easy redefinition of format conventions
import sys

def readsamplelist(sample_list):
    # return a list of dicts containing the process names, sample names and cross sections
    samples = []
    with open(sample_list) as f:
        for line in f:
            if(line[0] != '#' and len(line)>1):
                # first extract the tag (short name) of the process
                line = line.split('"')
                process_name = line[1]
                line = line[2][1:]
                # now extract sample name (and version name if present)
                line = line.split(' ')
                sample_name = line[0].rstrip('\n')
                if '/' in sample_name:
                    sample_name = sample_name.split('/')
                    version_name = sample_name[1]
                    sample_name = sample_name[0]
                else:
                    version_name = ''
                # finally extract cross-section
                cross_section = 0.
                if len(line)>1:
                    try:
                        cross_section = float(line[1].rstrip('\n'))
                    except:
                        print('### WARNING ###: found incompatible cross-section "'+line[1].rstrip('\n')+'".')
                        print('                 Using zero as default.')
                # fill the list with a dict for this sample
                samples.append({'process_name':process_name,'sample_name':sample_name,
                                    'version_name':version_name,'cross_section':cross_section})
    return samples

if __name__ == "__main__":
    if len(sys.argv)!=2:
        print('### ERROR ###: wrong number of command line arguments.')
        print('               usage: python readsamplelist.py <name_of_samplelist>.txt')
        sys.exit()
    test = readsamplelist(sys.argv[1])
    for el in test:
        print(el)

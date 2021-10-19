#########################################################################################
# very simple calculator script to convert signal strengths to cross sections for ratio #
#########################################################################################

import math

def ss_to_statsyst( signalstrength, uperror, downerror, uperrorstat, downerrorstat ):
    uperrorsyst = math.sqrt(uperror**2-uperrorstat**2)
    downerrorsyst = math.sqrt(downerror**2-downerrorstat**2)
    return (signalstrength, uperrorstat, downerrorstat, uperrorsyst, downerrorsyst)

def ss_to_xs( prediction, signalstrength, uperror, downerror, uperrorstat=0, downerrorstat=0 ):
    xsec = signalstrength*prediction
    xsecuperror = uperror*prediction
    xsecdownerror = downerror*prediction
    if( uperrorstat==0 or downerrorstat==0 ): return (xsec,xsecuperror,xsecdownerror)
    xsecuperrorstat = uperrorstat*prediction
    xsecdownerrorstat = downerrorstat*prediction
    xsecuperrorsyst = math.sqrt(uperror**2-uperrorstat**2)*prediction
    xsecdownerrorsyst = math.sqrt(downerror**2-downerrorstat**2)*prediction
    return (xsec,xsecuperrorstat,xsecdownerrorstat,xsecuperrorsyst,xsecdownerrorsyst)

def convert_numbers_ratio():
    
    # define theory cross-sections
    centralt = 59.3
    centraltbar = 31.8
    centraltotal = 94.2
    centralsum = centralt+centraltbar
    centralt = centralt*centraltotal/centralsum
    centraltbar = centraltbar*centraltotal/centralsum

    # define signal strengths (copied from measurement output txt file)
    ss_t = (1.01528059687, 0.0967728266057, 0.0932363139926, 0.120326016859, 0.110853996763)
    ss_tbar = (0.793469357988, 0.146057645495, 0.138847717499, 0.172694518291, 0.162394317467)
    ss_total = (0.933191, 0.0797515, 0.0773858, 0.111522, 0.100152)
    ss_ratio = (1.273, 0.301, 0.225, 0.333, 0.236)

    # calculate cross-section with uncertainties for total, top and antitop
    print('--- cross-sections and uncertainties ---')
    print('    (format: central, upstat, downstat, upsyst, downsyst)')
    
    print('  signal strength for tZq inclusive:')
    print('  '+str(ss_to_statsyst( ss_total[0], ss_total[3], ss_total[4], 
                    ss_total[1], ss_total[2])) )
    print('  cross-section for tZq inclusive:')
    print('  '+str(ss_to_xs(centraltotal, ss_total[0], ss_total[3], ss_total[4], 
		    ss_total[1], ss_total[2])) )

    print('  signal strength for tZq top:')
    print('  '+str(ss_to_statsyst( ss_t[0], ss_t[3], ss_t[4], 
                    ss_t[1], ss_t[2])) )
    print('  cross-section for tZq top:')
    print('  '+str(ss_to_xs(centralt, ss_t[0], ss_t[3], ss_t[4], 
                    ss_t[1], ss_t[2])) )

    print('  cross-section for tZq antitop:')
    print('  '+str(ss_to_statsyst( ss_tbar[0], ss_tbar[3], ss_tbar[4],                
                    ss_tbar[1], ss_tbar[2])) )
    print('  cross-section for tZq antitop:')
    print('  '+str(ss_to_xs(centraltbar, ss_tbar[0], ss_tbar[3], ss_tbar[4],                
                    ss_tbar[1], ss_tbar[2])) )

    # calculate ratio
    central_ratio = centralt/centraltbar
    print('--- ratio and uncertainties ---')
    print('    (format: central, upstat, downstat, upsyst, downsyst)')
    print('  signal strength for tZq ratio:')
    print('  '+str(ss_to_statsyst( ss_ratio[0], ss_ratio[3], ss_ratio[4],
                    ss_ratio[1], ss_ratio[2])) )
    print('  cross-section for tZq ratio:')
    print('  '+str(ss_to_xs(central_ratio, ss_ratio[0], ss_ratio[3], ss_ratio[4],
                    ss_ratio[1], ss_ratio[2])) )

    # write output file
    with open('topratio_calculator_meas_output.txt','w') as f:
	f.write('tZq tZq')
	for num in ss_to_xs( centraltotal, ss_total[0],
                    ss_total[3], ss_total[4],
                    ss_total[1], ss_total[2]): f.write(' {}'.format(num))
	f.write('\n')
	f.write('tZq_top tZq_top')
	for num in ss_to_xs( centralt, ss_t[0], 
		    ss_t[3], ss_t[4],
                    ss_t[1], ss_t[2]): f.write(' {}'.format(num))
	f.write('\n')
	f.write('tZq_antitop tZq_antitop')
	for num in ss_to_xs( centraltbar, ss_tbar[0], 
		    ss_tbar[3], ss_tbar[4],
                    ss_tbar[1], ss_tbar[2]): f.write(' {}'.format(num))
	f.write('\n')
	f.write('ratio ratio')
	for num in ss_to_xs( central_ratio, ss_ratio[0], 
		    ss_ratio[3], ss_ratio[4],
                    ss_ratio[1], ss_ratio[2]): f.write(' {}'.format(num))


if __name__=='__main__':

    convert_numbers_ratio()
